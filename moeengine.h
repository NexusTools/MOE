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
    Q_PROPERTY(QString version READ version)
    Q_PROPERTY(QVariantMap arguments READ arguments)
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

    MoeEngine(QVariantMap args =QVariantMap());
    virtual ~MoeEngine();

    static inline MoeEnginePointer threadEngine() {return _engine.localData();}
    inline QScriptEngine* scriptEngine() const{return _scriptEngine;}
    inline MoeEnginePointer engine() const{return MoeEnginePointer((MoeEngine*)this);}
    inline void makeCurrent() const{_engine.setLocalData(MoeEnginePointer(engine()));}

    inline void setFileContext(QString context) {_fileContext=context;}
    inline void startContent(QString content) {
        setFileContext(content);
        start();
    }

    inline QString error() const{return _error;}
    inline QString version() const{return "0.1 dev";}
    inline QVariantMap arguments() const{return _arguments;}

    void registerClass(const QMetaObject *metaObject);
    void inject(QString key, QVariant obj);

public slots:
    void quit();
    void abort(QString reason, bool crash =true);

    inline int setTimeout(QScriptValue callback, int mdelay) {
        int timerId = startTimer(mdelay);
        _timers.insert(timerId, callback);
        return timerId;
    }

    inline void clearTimeout(int handle) {
        _timers.remove(handle);
        killTimer(handle);
    }

    inline qreal random() {return (qreal)qrand()/(qreal)INT_MAX;}
    QScriptValue eval(QString script);

    void debug(QVariant);
    inline void setTicksPerSecond(uchar ticks) {_tickWait=1000/ticks;}

protected slots:
    void exceptionThrown(QScriptValue exception);

signals:
    void tick();
    void stopped();
    void started();
    void crashed(QString reason);
    void stateChanged(MoeEngine::State state);

protected:
    void timerEvent(QTimerEvent *);
    void setState(State);
    void run();

    Q_INVOKABLE void includeFile(QString filePath);

private:
    State _state;
    int _tickWait;
    QString _error;
    QString _fileContext;
    QEventLoop* _eventLoop;
    QVariantMap _arguments;
    QVariantMap _environment;
    QScriptEngine* _scriptEngine;
    QList<const QMetaObject*> _classes;
    QMap<int, QScriptValue> _timers;
    static QThreadStorage<MoeEnginePointer> _engine;
};

#endif // MOEENGINE_H
