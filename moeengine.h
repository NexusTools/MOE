#ifndef MOEENGINE_H
#define MOEENGINE_H

#include <QThreadStorage>
#include <QScriptValue>
#include <QVariant>
#include <QPointer>
#include <QThread>
#include <QMap>

class MoeEngine;

typedef QPointer<MoeEngine> MoeEnginePointer;

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

    static inline MoeEnginePointer threadEngine() {return _engine.localData();}
    inline QScriptEngine* scriptEngine() const{return _scriptEngine;}
    inline MoeEnginePointer engine() const{return MoeEnginePointer((MoeEngine*)this);}
    inline void makeCurrent() const{_engine.setLocalData(MoeEnginePointer(engine()));}

    void registerClass(QMetaObject* metaObject);
    void inject(QString key, QObject* obj);

public slots:
    void quit();
    void abort(QString reason);

    QScriptValue eval(QString script);

    void debug(QVariant);
    inline void setTicksPerSecond(uchar ticks) {_tickWait=1000/ticks;}

protected slots:
    void exceptionThrown(QScriptValue exception);

signals:
    void tick();
    void stateChanged(State state);

protected:
    void setState(State);
    void run();

    Q_INVOKABLE void includeFile(QString filePath);

private:
    State _state;
    int _tickWait;
    QEventLoop* _eventLoop;
    QScriptEngine* _scriptEngine;
    QMap<QString, QMetaObject*> _classes;
    QMap<QString, QObject*> _environment;
    static QThreadStorage<MoeEnginePointer> _engine;
};

#endif // MOEENGINE_H
