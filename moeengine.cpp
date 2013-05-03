#include "moeengine.h"
#include "renderrecorder.h"
#include "moeresourcerequest.h"
#include "moescriptregisters.h"
#include "moegraphicscontainer.h"
#include "moegraphicstext.h"

#include <QTimerEvent>
#include <QScriptEngine>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QPointF>
#include <QDebug>
#include <QFile>

QThreadStorage<MoeEnginePointer> MoeEngine::_engine;

MoeEngine::MoeEngine() {
    makeCurrent();
    _scriptEngine = 0;
    _eventLoop = 0;

    moveToThread(this);
    setTicksPerSecond(24);
    setState(Stopped);
}

MoeEngine::~MoeEngine()
{
    qDebug() << "Destroying MoeEngine";
    if(isRunning())
    {
        qWarning() << "Destroying Engine while Running" << this;
        quit();
        wait();
    }
}

void MoeEngine::timerEvent(QTimerEvent* ev) {
    if(_scriptEngine) {
        if(_timers.contains(ev->timerId())) {
            _timers.take(ev->timerId()).call();
            killTimer(ev->timerId());
        }
    }
}

void MoeEngine::setState(State state)
{
    if(_state == state)
        return;

    if(state == Running)
        emit started();
    if(state == Stopped)
        emit stopped();
    _state = state;
    emit stateChanged(_state);
}

QScriptValue MoeEngine::eval(QString script)
{
    scriptEngine()->pushContext();
    QScriptValue val = scriptEngine()->evaluate(script);
    scriptEngine()->popContext();
    return val;
}

void MoeEngine::quit()
{
    _state = Stopped;
    QThread::quit();
}

void MoeEngine::debug(QVariant data)
{
    qDebug() << data;
}

void MoeEngine::exceptionThrown(QScriptValue exception)
{
    abort(QString("%1:%2\n%3").arg(exception.property("fileName").toString()).arg(exception.property("lineNumber").toNumber()).arg(exception.toString()));
}

void MoeEngine::includeFile(QString filePath){
    qDebug() << "Reading File" << filePath;

    QFile file(filePath);
    if(file.open(QFile::ReadOnly)) {
        _scriptEngine->pushContext();
        _scriptEngine->currentContext()->pushScope(_scriptEngine->globalObject());
        _scriptEngine->currentContext()->setActivationObject(_scriptEngine->globalObject());
        _scriptEngine->evaluate(QString(file.readAll()), filePath);
        _scriptEngine->currentContext()->popScope();
        _scriptEngine->popContext();
        qDebug() << filePath;
    } else
        _scriptEngine->currentContext()->throwError(QScriptContext::UnknownError, QString("File Not Found: %1").arg(filePath));
}

void MoeEngine::run()
{
    setState(Starting);
    makeCurrent();
    _error = QString();

    qDebug() << "Creating script engine";

    QScriptEngine scriptEngine;
    _scriptEngine = &scriptEngine;

    __moe_registerScriptConverters(_scriptEngine);
    QScriptValue globalObject = scriptEngine.globalObject();

    QMapIterator<QString, QMetaObject*> classIterator(_classes);
    while(classIterator.hasNext())
    {
        classIterator.next();

        qDebug() << "Injecting" << classIterator.key();
        globalObject.setProperty(classIterator.value()->className(),
                                 scriptEngine.newQMetaObject(classIterator.value()));
    }

    QMapIterator<QString, QObject*> iterator(_environment);
    while(iterator.hasNext())
    {
        iterator.next();

        qDebug() << "Injecting" << iterator.key();
        globalObject.setProperty(iterator.key(), scriptEngine.newQObject(iterator.value()));
    }

    globalObject.setProperty("engine", scriptEngine.newQObject(this));
    globalObject.setProperty("ResourceRequest", scriptEngine.newQMetaObject(&MoeResourceRequest::staticMetaObject));
    globalObject.setProperty("GraphicsObject", scriptEngine.newQMetaObject(&MoeGraphicsObject::staticMetaObject));
    globalObject.setProperty("GraphicsContainer", scriptEngine.newQMetaObject(&MoeGraphicsContainer::staticMetaObject));
    globalObject.setProperty("GraphicsText", scriptEngine.newQMetaObject(&MoeGraphicsText::staticMetaObject));
    globalObject.setProperty("RenderRecorder", scriptEngine.newQMetaObject((QMetaObject*)&RenderRecorder::staticMetaObject));

    includeFile(":/data/prototype.js");
    if(scriptEngine.hasUncaughtException()) {
        exceptionThrown(scriptEngine.uncaughtException());
        return;
    }

    includeFile(":/data/shared.js");
    if(scriptEngine.hasUncaughtException()) {
        exceptionThrown(scriptEngine.uncaughtException());
        return;
    }

    includeFile(":/data/content-select/init.js");
    if(scriptEngine.hasUncaughtException()) {
        exceptionThrown(scriptEngine.uncaughtException());
        return;
    }

    connect(&scriptEngine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exceptionThrown(QScriptValue)));
    qDebug() << "Entering Main Loop";
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

    _eventLoop = 0;
    qDebug() << "Engine Exited Main Loop";

    _scriptEngine = 0;

    qDebug() << "Engine Thread Exited";

    setState(Stopped);
}

void MoeEngine::abort(QString reason)
{
    qCritical() << "Execution Aborted" << reason;
    _error = QString("Uncaught Exception:\n%1").arg(reason);
    if(!_eventLoop)
        return;

    _scriptEngine->abortEvaluation();
    _eventLoop->exit(1);
    setState(Crashed);
}

void MoeEngine::registerClass(QMetaObject* metaObject)
{
    _classes.insert(metaObject->className(), metaObject);
}

void MoeEngine::inject(QString key, QObject* val)
{
    val->moveToThread(this);
    _environment.insert(key, val);
    registerClass((QMetaObject*)val->metaObject());
}
