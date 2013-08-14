#include <network/moeresourcerequest.h>
#include <network/moeurl.h>

#include "moeobject.h"
#include "moeengine.h"
#include "moescriptregisters.h"

#include <QAbstractEventDispatcher>
#include <QScriptEngine>
#include <QElapsedTimer>
#include <QMetaMethod>
#include <QTimerEvent>
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

    moveToThread(this);
    setTicksPerSecond(24);
}

inline void _exit(int i) {
    exit(i);
}

void MoeEngine::startWithArguments(QVariantMap args) {
    _arguments = args;

    QUrl loader(MoeUrl::locate(args.value("loader", "standard.js").toString(), "loaders://"));
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
    if(_loader.isRelative())
        _loader = MoeUrl::locate(_loader.toString(), "loaders://");
    if(QThread::currentThread() != this) {
        if(isRunning()) {
            QMetaMethod startContent = metaObject()->method(metaObject()->indexOfMethod("startContent(QString,QUrl)"));
            Q_ASSERT(startContent.isValid());
            startContent.invoke(this, Qt::QueuedConnection, Q_ARG(QString, content), Q_ARG(QUrl, _loader));
        } else {
            initContentPath = content;
            loader = _loader;
            start();
        }
    } else {
        if(_state == Running) {
            qDebug() << "Changing content" << content << _loader;
            initContentPath = content;
            loader = _loader;
            stopExecution("Changing content", false, Changing);
        } else
            qCritical() << "Cannot change content from this state" << _state;
    }

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
    if(_state == state || _state == Deleted)
        return;

    static QMetaEnum stateEnum = MoeEngine::staticMetaObject.enumerator(MoeEngine::staticMetaObject.indexOfEnumerator("State"));
    qDebug() << "Engine state changed" << stateEnum.key(state) << "from" << stateEnum.key(_state);

    switch(state){
        case Running:
            emit started();
        break;
        case Stopped:
            emit stopped();
        break;
        case Changing:
            emit changingContent();
        break;
        case Crashed:
            emit crashed(_error);
        break;
        default:
        break;
    }
    emit stateChanged(state);
    _state = state;
}

void MoeEngine::quit(QString message)
{
    stopExecution(message.isEmpty() ? "Engine Quit" : message, false, Stopping);
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

void MoeEngine::exitEventLoop() {
    exit(0);
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

void MoeEngine::processEvents(qint32 until) {
    QTimer::singleShot(until, Qt::PreciseTimer, this, SLOT(exitEventLoop()));
    exec();
}

void MoeEngine::setupGlobalObject() {
    QScriptValue globalObject = _scriptEngine->globalObject();
    globalObject.setProperty("global", globalObject, QScriptValue::SkipInEnumeration);
    globalObject.setProperty("eval", _scriptEngine->newFunction(__eval_func__));

    QList<const QMetaObject*> classesToLoad = _classes;
    classesToLoad.append(&MoeUrl::staticMetaObject);
    classesToLoad.append(&MoeResourceRequest::staticMetaObject);

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

        QObject* obj = iterator.value().value<QObject*>();
        if(obj) {
            globalObject.setProperty(iterator.key(), _scriptEngine->newQObject(obj));
            continue;
        }

        globalObject.setProperty(iterator.key(), _scriptEngine->newVariant(iterator.value()));
    }

    initializeContentEnvironment(_scriptEngine, globalObject);
}

static QMetaMethod tickSignal = QMetaMethod::fromSignal(&MoeEngine::tick);
static QMetaMethod preciseTickSignal = QMetaMethod::fromSignal(&MoeEngine::preciseTick);
void MoeEngine::mainLoop() {
    qDebug() << "Preparing main loop";
    QMetaMethod emitTick = metaObject()->method(metaObject()->indexOfMethod("emitTick()"));
    Q_ASSERT(emitTick.isValid());

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
        MoeResourceRequest::reset();
        MoeResourceRequest* initRequest = new MoeResourceRequest(loader);
        connect(initRequest, SIGNAL(receivedString(QString)), this, SLOT(eval(QString)));
        connect(initRequest, SIGNAL(error(QString)), this, SLOT(abort(QString)));
        connect(initRequest, SIGNAL(completed(bool)), this, SLOT(deleteLater()), Qt::QueuedConnection);
    }

    if(_state == Starting) {
        setState(Running);
        qDebug() << "Entering main loop" << MoeUrl::defaultContext().toString();
        while(_state == Running)
        {
            if(isSignalConnected(tickSignal))
                emitTick.invoke(this, Qt::QueuedConnection);

            while((sleepTime = nextWait - timer.elapsed()) > 0) {
                processEvents(sleepTime);
                if(_scriptEngine->hasUncaughtException()) {
                    exceptionThrown(_scriptEngine->uncaughtException());
                    break;
                }
                if((sleepTime = nextWait - timer.elapsed()) > 0)
                    msleep(qMin(sleepTime, 50));
            }

            if(_state == Running){
                qint32 elapsed = _tickWait - timer.restart();
                if(isSignalConnected(preciseTickSignal))
                    emit preciseTick((qreal)elapsed/(qreal)_tickWait);
                nextWait += elapsed;
            }
        }

        qDebug() << "Content finished, cleaning up";
    }
}

void MoeEngine::run()
{
    makeCurrent();
    _error.clear();
    setState(Starting);
    Q_ASSERT(eventDispatcher());

initializeEngine:
    _scriptEngine = new QScriptEngine();
    __moe_registerScriptConverters(_scriptEngine);
    initializeScriptEngine(_scriptEngine);

    qDebug() << "Initializing new engine context";
    MoeUrl::setDefaultContext("loaders://");

    setupGlobalObject();
    mainLoop();

    foreach(int timerId, _timers.keys())
        killTimer(timerId);
    _timers.clear();

    emit cleanup();
    MoeObject::instances.localData().clear();
    qDebug() << "Processing remaining events.";
    _scriptEngine->collectGarbage();
    _scriptEngine->deleteLater();
    _scriptEngine = 0;
    processEvents(0);

    if(_state == Changing) {
        _error.clear();
        setState(Starting);
        goto initializeEngine;
    }

    loader.clear();
    initContentPath.clear();
    if(_state == Deleted) {
        deleteLater();
        qWarning() << "Engine was deleted from within its own thread.";
        return;
    }
    if(_state != Crashed)
        setState(Stopped);
    qDebug() << "Engine thread finished";
}

void MoeEngine::emitTick() {
    emit tick();
}

void MoeEngine::stopExecution(QString reason, bool crash, State newState)
{
    if(_state == Deleted)
        return;

    _error = reason;
    setState(newState);
    if(crash)
        qCritical() << "Execution Aborted" << reason;
    else
        qDebug() << reason;

    if(_scriptEngine)
        _scriptEngine->abortEvaluation();

    exitEventLoop();
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

        if(!_context.path().endsWith('/'))
            _context.setPath(_context.path()+'/');

        qDebug() << url << _context;
        url = _context.resolved(url);
        qDebug() << url;
    }

    return url;
}

QThreadStorage<QUrl> MoeUrl::context;
