#include "moeurl.h"
#include "moeengine.h"
#include "moegraphicsimage.h"
#include "moeresourcerequest.h"
#include "moescriptregisters.h"
#include "moegraphicscontainer.h"
#include "moegraphicstext.h"
#include "moegraphicssurface.h"

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

MoeEngine::~MoeEngine()
{
    //qDebug() << "Destroying MoeEngine";
    if(isRunning())
    {
        qWarning() << "Destroying Engine while Running" << this;
        quit();
        wait();
    }
}

void MoeEngine::changeFileContext(QString context) {
    if(_scriptEngine)
        MoeUrl::setDefaultContext(context);
    else
        initContentPath = context;
}

void MoeEngine::startContent(QString content, QUrl _loader) {
    qDebug() << "Starting Content" << content;
    initContentPath = content;
    loader = _loader;

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
    //static QMetaEnum stateEnum = MoeEngine::staticMetaObject.enumerator(MoeEngine::staticMetaObject.indexOfEnumerator("State"));
    //qDebug() << "Engine State Changed" << stateEnum.key(state) << "from" << stateEnum.key(_state);

    if(_state == state)
        return;

    emit stateChanged(state);
    if(state == Running)
        emit started();
    if(state == Stopped)
        emit stopped();
    if(state == Crashed)
        emit crashed(_error);
    _state = state;
}

void MoeEngine::play() {
    if(_state == Paused) {
        qDebug() << "Unpausing Engine";
        setState(Running);
    }
}

void MoeEngine::pause() {
    if(_state == Running) {
        qDebug() << "Pausing Engine";
        setState(Paused);
        if(QThread::currentThread() == this) {
            while(_state == Paused)
                msleep(50);
        }
    }
}

void MoeEngine::quit()
{
    abort("Engine Quit", false);
}

void MoeEngine::debug(QString data)
{
    QDebug debug(QtDebugMsg);
    debug << QString("[%1]").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)).toLocal8Bit().data();
    debug << "[DEBUG]";
    if(_scriptEngine) {
        QString trace = _scriptEngine->currentContext()->backtrace().at(1);
        int at = trace.indexOf(" at ");
        if(at > -1) {
            trace = trace.mid(at+4);
            if(trace.indexOf(":") == -1)
                trace = "anonymous:" + trace;
        }
        debug << trace.toLocal8Bit().data();
    }
    debug << '\n' << data.toLocal8Bit().data();
}

void MoeEngine::eval(QString script) {
    _scriptEngine->evaluate(script, "loader");
}

void MoeEngine::exceptionThrown(QScriptValue exception)
{
    if(_state == Starting || _state == Running) {
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
    setState(Starting);
    _error = QString();
    while(!initContentPath.isEmpty() && _error.isEmpty()) {
        QScriptEngine scriptEngine;
        _scriptEngine = &scriptEngine;
        MoeUrl::setDefaultContext(":/loaders/");

        __moe_registerScriptConverters(_scriptEngine);
        QScriptValue globalObject = scriptEngine.globalObject();
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
                globalObject.setProperty(iterator.key(), scriptEngine.newQObject(obj));
                continue;
            }

            globalObject.setProperty(iterator.key(), scriptEngine.newVariant(iterator.value()));
        }

        MoeUrl::setDefaultContext(initContentPath);
        MoeResourceRequest* initRequest = new MoeResourceRequest(loader);
        connect(initRequest, SIGNAL(receivedString(QString)), this, SLOT(eval(QString)), Qt::QueuedConnection);
        connect(initRequest, SIGNAL(error(QString)), this, SLOT(abort(QString)), Qt::QueuedConnection);
        connect(&scriptEngine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exceptionThrown(QScriptValue)));
        QElapsedTimer timer;

        QEventLoop eventLoop;
        _eventLoop = &eventLoop;
        timer.start();
        int sleepTime;
        int nextWait = _tickWait;
        setState(Running);
        while(_state != Stopped && _state != Crashed)
        {
            if(_state == Running) {
                emit tick();
                if((sleepTime = nextWait - timer.elapsed()) > 0)
                    eventLoop.processEvents(QEventLoop::WaitForMoreEvents, sleepTime);
            }

            while((sleepTime = nextWait - timer.elapsed()) > 0)
                msleep(sleepTime);

            nextWait += _tickWait - timer.restart();
        }

        foreach(int timerId, _timers.keys())
            killTimer(timerId);
        _timers.clear();

        _scriptEngine = 0;
        _eventLoop = 0;
    }

    initContentPath.clear();
    setState(Stopped);
    exit(0);
}

void MoeEngine::abort(QString reason, bool crash)
{
    _error = reason;
    setState(crash ? Crashed : Stopped);
    qCritical() << "Execution Aborted" << reason;

    if(!_eventLoop)
        return;

    _scriptEngine->abortEvaluation();
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
    static QRegExp absolutePath("^(([\\d\\w\\-_]*:)?[/\\\\]+|[\\d\\w\\-_]+:).*");
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
