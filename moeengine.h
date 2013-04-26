#ifndef MOEENGINE_H
#define MOEENGINE_H

#include <QThreadStorage>
#include <QScriptValue>
#include <QThread>
#include <QMap>

class QScriptEngine;
class QEventLoop;

class MoeEngine : public QThread
{
    Q_OBJECT
    Q_ENUMS(State)
public:
    enum State
    {
        Stopped,
        Paused,
        Starting,
        Crashed,
        Running
    };

    MoeEngine();
    virtual ~MoeEngine();

    static inline MoeEngine* threadEngine() {return _engine.localData();}
    inline QScriptEngine* scriptEngine() const{return _scriptEngine;}
    inline MoeEngine* engine() const{return (MoeEngine*)this;}
    inline void makeCurrent() const{_engine.setLocalData(engine());}

    void inject(QString key, QObject* obj);

public slots:
    void quit();
    void abort(QString reason);

    void debug(QString);
    inline void setTicksPerSecond(uchar ticks) {_tickWait=1000/ticks;}

protected slots:
    void exceptionThrown(QScriptValue exception);

signals:
    void tick();
    void stateChanged(State state);

protected:
    void setState(State);
    void run();

private:
    State _state;
    int _tickWait;
    QEventLoop* _eventLoop;
    QScriptEngine* _scriptEngine;
    QMap<QString, QObject*> _environment;
    static QThreadStorage<MoeEngine*> _engine;
};

#endif // MOEENGINE_H
