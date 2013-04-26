#include "moeengine.h"

#include <QScriptEngine>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QPointF>
#include <QDebug>
#include <QFile>

QThreadStorage<MoeEngine*> MoeEngine::_engine;

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

void MoeEngine::setState(State state)
{
    if(_state == state)
        return;

    _state = state;
    emit stateChanged(_state);
}

void MoeEngine::quit()
{
    _state = Stopped;
    QThread::quit();
}

void MoeEngine::debug(QString string)
{
    qDebug() << string;
}

void MoeEngine::exceptionThrown(QScriptValue exception)
{
    abort(exception.toString());
}

void MoeEngine::run()
{
    setState(Starting);
    makeCurrent();

    qDebug() << "Reading Init File";
    QFile initFile(":/data/content-select/init.js");
    if(initFile.open(QFile::ReadOnly))
    {
        qDebug() << "Creating script engine";

        QScriptEngine scriptEngine;
        _scriptEngine = &scriptEngine;
        QScriptValue globalObject = scriptEngine.newObject();
        QMapIterator<QString, QObject*> iterator(_environment);
        while(iterator.hasNext())
        {
            iterator.next();

            qDebug() << "Injecting" << iterator.value()->metaObject()->className();
            globalObject.setProperty(iterator.value()->metaObject()->className(),
                                     scriptEngine.newQMetaObject(iterator.value()->metaObject()));
            globalObject.setProperty(iterator.key(), scriptEngine.newQObject(iterator.value()));
        }
        globalObject.setProperty("engine", scriptEngine.newQObject(this));
        scriptEngine.setGlobalObject(globalObject);
        scriptEngine.evaluate(QString(initFile.readAll()));

        if(scriptEngine.hasUncaughtException())
        {
            exceptionThrown(scriptEngine.uncaughtException());
        } else {
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
                if(_state != Running)
                {
                    while((sleepTime = nextWait - timer.elapsed()) > 0)
                        msleep(sleepTime);
                } else {
                    while((sleepTime = nextWait - timer.elapsed()) > 0 && _state == Running)
                        eventLoop.processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents, sleepTime);

                    emit tick();
                }

                nextWait += _tickWait - timer.restart();
            }
            _eventLoop = 0;
            qDebug() << "Engine Exited Main Loop";
        }
        _scriptEngine = 0;
    } else
        qWarning() << "Failed to open";
    qDebug() << "Engine Thread Exited";

    setState(Stopped);
}

void MoeEngine::abort(QString reason)
{
    if(!_eventLoop)
        return;

    qCritical() << "Execution Aborted" << reason;
    _scriptEngine->abortEvaluation();
    _eventLoop->exit(1);
    setState(Crashed);
}

void MoeEngine::inject(QString key, QObject* val)
{
    _environment.insert(key, val);
}
