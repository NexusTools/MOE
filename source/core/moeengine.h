#ifndef MOEENGINE_H
#define MOEENGINE_H

#include <QThreadStorage>
#include <QScriptValue>
#include <QVariant>
#include <QPointer>
#include <QThread>
#include <QMap>
#include <QUrl>

class MoeEngine;
class QEventLoop;
class QScriptEngine;

typedef QPointer<MoeEngine> MoeEnginePointer;

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
        Crashed,

        Stopping,
        Deleted,
        Starting,

        Running,
        Changing // Content is being changed
    };

    MoeEngine();
    virtual ~MoeEngine();
    bool event(QEvent *event);

    inline bool isActive() const{
        return _state >= Running;
    }

    inline bool isProcessing() const{
        return _state >= Starting;
    }

    static void registerQDebugHandler();
    inline QScriptEngine* scriptEngine() const{return _scriptEngine;}
    static inline MoeEnginePointer current() {return _engine.localData();}
    inline MoeEnginePointer engine() const{return MoeEnginePointer((MoeEngine*)this);}
    inline void makeCurrent() const{_engine.setLocalData(MoeEnginePointer(engine()));}

    Q_INVOKABLE void changeFileContext(QString context);
    void startWithArguments(QVariantMap args =QVariantMap());
    Q_INVOKABLE void startContent(QString content, QUrl loaderPath =QUrl("./standard.js"));

    inline QString error() const{return _error;}
    inline QString version() const{return "0.1 dev";}
    inline QVariantMap arguments() const{return _arguments;}

    void registerClass(const QMetaObject *metaObject);
    void inject(QString key, QVariant obj);

public slots:
    inline bool isNative(QScriptValue val) {
        return val.isQObject() || val.isQMetaObject();
    }

    void quit(QString message =QString());

    inline int setTimeout(QScriptValue callback, int mdelay =0) {
        int timerId = startTimer(mdelay);
        _timers.insert(timerId, callback);
        return timerId;
    }

    inline void clearTimeout(int handle) {
        _timers.remove(handle);
        killTimer(handle);
    }

    void debug(QVariant string);
    inline qreal random() {return (qreal)qrand()/(qreal)INT_MAX;}
    inline void setTicksPerSecond(uchar ticks) {_tickWait=1000/ticks;}

signals:
    void tick();
    void preciseTick(qreal);

    void stopped();
    void started();
    void cleanup();
    void changingContent();
    void crashed(QString reason);
    void uncaughtException(QScriptValue);
    void stateChanged(MoeEngine::State state);

protected:
    void stopExecution(QString reason, bool crashed, State newState);

    inline void abort(QString reason, bool crashed) {
        stopExecution(reason, crashed, crashed ? Crashed : Stopping);
    }

    Q_INVOKABLE void emitTick();
    void timerEvent(QTimerEvent *);
    void setState(State);

    void setupGlobalObject();
    void processEvents(qint32 until);
    void mainLoop();
    void run();

protected slots:
    void eval(QString);
    void exitEventLoop();
    inline void abort(QString reason) {stopExecution(reason, true, Crashed);}
    void exceptionThrown(QScriptValue);

private:
    static QThreadStorage<MoeEnginePointer> _engine;

    QString _error;
    State _state;
    int _tickWait;

    QVariantMap _arguments;
    QVariantMap _environment;

    QScriptEngine* _scriptEngine;
    QMap<int, QScriptValue> _timers;
    QList<const QMetaObject*> _classes;

    QUrl loader;
    QString initContentPath;
};

#endif // MOEENGINE_H
