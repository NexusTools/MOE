#include "../network/moeurl.h"
#include "moeengine.h"
#include "../gui/moegraphicsimage.h"
#include "../network/moeresourcerequest.h"
#include "moescriptregisters.h"
#include "../gui/moegraphicscontainer.h"
#include "../gui/moegraphicstext.h"
#include "../gui/moegraphicssurface.h"

#include <QScriptEngine>
#include <QElapsedTimer>
#include <QMetaMethod>
#include <QTimerEvent>
#include <QEventLoop>
#include <QDateTime>
#include <QFileInfo>
#include <QPointF>
#include <QDebug>
#include <QFile>

QThreadStorage<MoeEnginePointer> MoeEngine::_engine;

MoeEngine::MoeEngine() {
    makeCurrent();
    _scriptEngine = 0;
    _state = Stopped;
    _eventLoop = 0;

    moveToThread(this);
    setTicksPerSecond(24);
}

void MoeEngine::startWithArguments(QVariantMap args) {
    _arguments = args;

    QUrl loader(MoeUrl::locate(args.value("loader", "standard.js").toString(), ":/loaders/"));
    if(args.contains("example"))
        startContent(QString(":/examples/%1/").arg(args.value("example").toString()), loader);
    else if(args.contains("content") || args.contains(""))
        startContent(args.value("content", args.value("")).toString(), loader);
    else
        startContent(":/content-select/", loader);
}

bool MoeEngine::event(QEvent *event){
    if(event->type() == QEvent::DeferredDelete) {
        if(QThread::currentThread() == this) {
            abort("Engine marked for deletion", Deleted);
            return false;
        }
    }

    return QObject::event(event);
}

MoeEngine::~MoeEngine()
{
    //qDebug() << "Destroying MoeEngine";
    if(QThread::isRunning() && this != QThread::currentThread())
    {
        qWarning() << "Destroying engine while thread running" << this;
        quit();
        wait();
    }
}

inline QString pointerToString(void* ptr) {
    return QString("0x%0").arg((qulonglong)ptr,8,16,QChar('0'));
}

void customMessageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    QByteArray streamData;
    {
        QTextStream textStream(&streamData, QFile::WriteOnly);
        textStream << '[';
        textStream << QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        textStream << "] [";
        {
            QObject* thread = QThread::currentThread();
            QByteArray name = thread->objectName().toLocal8Bit();
            if(name.isEmpty())
                name = thread->metaObject()->className();
            textStream << name;
            textStream << ' ';
            textStream << thread;
        }
        textStream << "] ";

        QString func(ctx.function);
        if(func.isEmpty())
            func = "anonymous";
        else {
            QRegExp methodName("[\\s^]([\\w]+[\\w\\d\\-_]*(::[\\w]+[\\w\\d\\-_]*)?)\\(");
            if(methodName.indexIn(ctx.function) > -1)
                func = methodName.cap(1);
        }
        textStream << '[';
        textStream << func;
        textStream << "] ";

        QString file(ctx.file);
        if(file.isEmpty())
            file = "Unknown";
#ifndef QT_NO_DEBUG
        else {
            QRegExp nativeSourcePath("^((\\.\\.|\\w:|\\\\)?[/\\\\].*[/\\\\])?source[/\\\\](.+\\.(cpp|c|h|hpp))$", Qt::CaseInsensitive, QRegExp::RegExp2);
            if(nativeSourcePath.exactMatch(file))
                file = "https://raw.github.com/NexusTools/MOE/master/source/" + nativeSourcePath.cap(3);
        }
#endif
        textStream << file << ':' << ctx.line << '\n';

        switch(type) {
            case QtDebugMsg:
                textStream << "[DEBUG]";
            break;

            case QtWarningMsg:
                textStream << "[WARNING]";
            break;

            case QtCriticalMsg:
                textStream << "[CRITICAL]";
            break;

            case QtFatalMsg:
                textStream << "[FATAL]";
            break;

            default:
                textStream << "[UNKNOWN]";
            break;
        }

        textStream << ' ' << msg << "\n\n";
    }
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    FILE* out = type != QtDebugMsg ? stderr : stdout;
    fwrite(streamData.data(), 1, streamData.length(), out);
    fflush(out);

    if(type == QtFatalMsg)
        abort();
}

void MoeEngine::registerQDebugHandler() {
    static bool registered = false;
    if(registered)
        return;
    registered = true;

    qInstallMessageHandler(customMessageHandler);
}

void MoeEngine::changeFileContext(QString context) {
    if(_scriptEngine)
        MoeUrl::setDefaultContext(context);
    else
        initContentPath = context;
}

void MoeEngine::startContent(QString content, QUrl _loader) {
    qDebug() << "Starting content" << content << "using" << _loader;
    initContentPath = content;
    loader = _loader.isRelative() ? MoeUrl::locate(_loader.toString(), "qrc:/loaders/") : _loader;

    if(isActive())
        metaObject()->invokeMethod(this, "quit()", Qt::QueuedConnection);
    else
        start();
}

void MoeEngine::timerEvent(QTimerEvent* ev) {
    if(_scriptEngine) {
        if(_timers.contains(ev->timerId())) {
            _timers.take(ev->timerId()).call();
            killTimer(ev->timerId());

            if(_scriptEngine->hasUncaughtException())
                exceptionThrown(_scriptEngine->uncaughtException());
        }
    }
}

void MoeEngine::setState(State state)
{
    if(_state == state
            || _state == Deleted)
        return;

    static QMetaEnum stateEnum = MoeEngine::staticMetaObject.enumerator(MoeEngine::staticMetaObject.indexOfEnumerator("State"));
    qDebug() << "Engine state changed" << stateEnum.key(state) << "from" << stateEnum.key(_state);

    emit stateChanged(state);
    if(state == Running)
        emit started();
    if(state == Stopped)
        emit stopped();
    if(state == Crashed)
        emit crashed(_error);
    _state = state;
}

void MoeEngine::quit()
{
    abort("Engine Quit", false);
}

void MoeEngine::debug(QVariant value)
{
    if(_scriptEngine) {
        int line = 0;
        QByteArray function = _scriptEngine->currentContext()->backtrace().at(1).toLocal8Bit();
        int at = function.indexOf(" at ");
        QByteArray file;
        if(at > -1) {
            file = function.mid(at+4);
            function = function.mid(0, at);
            int lineSep = file.lastIndexOf(":");
            if(lineSep > -1) {
                line = file.mid(lineSep+1).toInt();
                file = file.left(lineSep);
            } else {
                file = "anonymous";
                line = file.toInt();
            }
        }
        if(function.startsWith('<'))
            function = function.mid(1, function.indexOf('>')-1);
        else {
            int pos = function.indexOf("(");
            function = function.mid(0, pos);
        }

        QDebug debug = QMessageLogger((const char*)file.data(), line, (const char*)function.data()).debug();
        switch(value.type()) {
            case QVariant::String:
                debug << value.toString();
            break;

            case QVariant::Double:
                debug << value.toDouble();
            break;

            case QVariant::Int:
                debug << value.toInt();
            break;

            case QVariant::Map:
                debug << value.toMap();
            break;

            case QVariant::List:
                debug << value.toList();
            break;

            case QVariant::StringList:
                debug << value.toStringList();
            break;

            default:
                debug << value;
        }
    } else
        qWarning() << "Engine not running.";
}

void MoeEngine::eval(QString script) {
    if(loader.isEmpty())
        _scriptEngine->evaluate(script);
    else {
        _scriptEngine->evaluate(script, loader.toString());
        loader.clear();
    }
}

void MoeEngine::exceptionThrown(QScriptValue exception)
{
    if(_state == Starting || _state == Running) {
        if(!_error.isEmpty()) {
            abort(_error);
            return;
        }
        QString message("An unhandled exception occured.\n");
        message += exception.toString();
        if(!_scriptEngine->uncaughtExceptionBacktrace().isEmpty()) {
            message += "\n\nStack Trace\n--------------------------";
            foreach(QString stack, _scriptEngine->uncaughtExceptionBacktrace())
                message += '\n' + stack;
        } else if(exception.property("lineNumber").isNumber() &&
                  exception.property("fileName").isString()) {
            message += "\non ";
            message += exception.property("fileName").toString();
            message += ':';
            message += QString("%1").arg(exception.property("lineNumber").toInt32());
        }


        static QMetaMethod exceptionSignal = metaObject()->method(metaObject()->indexOfSignal("uncaughtException(QScriptValue)"));
        if(isSignalConnected(exceptionSignal)) {
            qDebug() << "Exception deferred to script";
            _error = message;
            _scriptEngine->clearExceptions();
            emit uncaughtException(exception);
            if(!_scriptEngine->hasUncaughtException())
                _error.clear();
        } else
            abort(message);
    }
}

QScriptValue __eval_func__(QScriptContext* ctx, QScriptEngine* eng) {
    if(ctx->argument(0).toString().isEmpty()) {
        ctx->throwError("Evaluating nothing");
        return eng->nullValue();
    }

    ctx->pushScope(ctx->thisObject());
    QScriptValue result = eng->evaluate(ctx->argument(0).toString(), ctx->argument(1).toString());
    ctx->popScope();
    return result;
}

void MoeEngine::run()
{
    makeCurrent();
    _error.clear();
    setState(Starting);
    QEventLoop eventLoop;
    _eventLoop = &eventLoop;
    while(_state == Starting || _state == Changing) {
        {
            _scriptEngine = new QScriptEngine();
            MoeUrl::setDefaultContext(":/loaders/");

            __moe_registerScriptConverters(_scriptEngine);
            QScriptValue globalObject = _scriptEngine->globalObject();
            globalObject.setProperty("global", globalObject, QScriptValue::SkipInEnumeration);
            globalObject.setProperty("eval", _scriptEngine->newFunction(__eval_func__));

            QList<const QMetaObject*> classesToLoad = _classes;
            classesToLoad.append(&MoeUrl::staticMetaObject);
            classesToLoad.append(&MoeGraphicsImage::staticMetaObject);
            classesToLoad.append(&MoeGraphicsObject::staticMetaObject);
            classesToLoad.append(&MoeResourceRequest::staticMetaObject);
            classesToLoad.append(&MoeGraphicsContainer::staticMetaObject);
            classesToLoad.append(&MoeGraphicsSurface::staticMetaObject);
            classesToLoad.append(&MoeGraphicsText::staticMetaObject);

            foreach(const QMetaObject* metaObject, classesToLoad)  {
                QString key(metaObject->className());
                if(key.startsWith("Moe"))
                    key = key.mid(3);
                globalObject.setProperty(key, _scriptEngine->newQMetaObject(metaObject));
            }

            QVariant engine;
            QVariantMap environmentToLoad = _environment;
            engine.setValue<QObject*>((QObject*)this);
            environmentToLoad.insert("engine", engine);
            QMapIterator<QString, QVariant> iterator(environmentToLoad);
            while(iterator.hasNext())
            {
                iterator.next();

                //qDebug() << "Injecting" << iterator.key() << iterator.value();
                QObject* obj = iterator.value().value<QObject*>();
                if(obj) {
                    globalObject.setProperty(iterator.key(), _scriptEngine->newQObject(obj));
                    continue;
                }

                globalObject.setProperty(iterator.key(), _scriptEngine->newVariant(iterator.value()));
            }

            int nextWait = _tickWait;
            QElapsedTimer timer;
            int sleepTime;
            timer.start();

            connect(_scriptEngine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exceptionThrown(QScriptValue)));
            if(initContentPath.isEmpty())
                MoeUrl::setDefaultContext(":/content-select/");
            else {
                MoeUrl::setDefaultContext(initContentPath);
                initContentPath.clear();
            }

            {
                MoeResourceRequest* initRequest = new MoeResourceRequest(loader);
                connect(initRequest, SIGNAL(receivedString(QString)), this, SLOT(eval(QString)));
                connect(initRequest, SIGNAL(error(QString)), this, SLOT(abort(QString)));
                connect(initRequest, SIGNAL(completed(bool)), this, SLOT(deleteLater()), Qt::QueuedConnection);
            }

            if(_state == Starting) {
                setState(Running);
                while(_state == Running)
                {
                    emit tick();

                    if((sleepTime = nextWait - timer.elapsed()) > 0)
                        eventLoop.processEvents(QEventLoop::WaitForMoreEvents, sleepTime);
                    if(_scriptEngine->hasUncaughtException())
                        exceptionThrown(_scriptEngine->uncaughtException());

                    while((sleepTime = nextWait - timer.elapsed()) > 0)
                        msleep(sleepTime);

                    nextWait += _tickWait - timer.restart();
                }
            }

            qDebug() << "Content finished, cleaning up";
            foreach(int timerId, _timers.keys())
                killTimer(timerId);
            _timers.clear();

            _scriptEngine->deleteLater();
            _scriptEngine = 0;
        }

        qDebug() << "Processing remaining events";
        eventLoop.processEvents();
    }

    _eventLoop = 0;
    loader.clear();
    initContentPath.clear();
    if(_state == Deleted) {
        deleteLater();
        qWarning() << "Engine was deleted from within its own thread.";
        return;
    }
    setState(Stopped);
    qDebug() << "Engine thread finished";
    exit(0);
}

void MoeEngine::abort(QString reason, bool crash, State newState)
{
    if(_state == Deleted)
        return;

    _error = reason;
    setState(newState == Auto ? (crash ? Crashed : Stopping) : newState);
    if(crash)
        qCritical() << "Execution Aborted" << reason;
    else
        qDebug() << reason;

    if(_scriptEngine)
        _scriptEngine->abortEvaluation();

    if(!_eventLoop)
        return;
    _eventLoop->exit(1);
}

void MoeEngine::registerClass(const QMetaObject* metaObject)
{
    if(!_classes.contains(metaObject))
        _classes.append(metaObject);
}

void MoeEngine::inject(QString key, QVariant val)
{
    if(val.canConvert(QMetaType::QObjectStar)) {
        QObject *obj = ((QObject*)val.value<QObject*>());
        registerClass(obj->metaObject());
        obj->moveToThread(this);
    }

    _environment.insert(key, val);
}

QUrl MoeUrl::urlFromString(QString path) {
    QRegExp absolutePath("^(([\\d\\w\\-_]*:)?[/\\\\]+|[\\d\\w\\-_]+:).*");
    if(path.startsWith('.'))
        return QUrl(path);
    else if(absolutePath.exactMatch(path)) {
        if(absolutePath.cap(1) == ":/")
            return QUrl(QString("qrc%1").arg(path));

        QFileInfo fileInfo(absolutePath.cap(0));
        if(fileInfo.exists())
            return QUrl::fromLocalFile(path);

        return QUrl(path);
    } else
        return QUrl(QString("./%1").arg(path));
}

QUrl MoeUrl::locate(QString path, QString context) {
    QUrl url(urlFromString(path));
    if(url.isRelative()) {
        QUrl _context(urlFromString(context));
        if(_context.isEmpty() || _context.isRelative())
            _context = defaultContext();

        url = _context.resolved(url);
    }

    return url;
}

QThreadStorage<QUrl> MoeUrl::context;
