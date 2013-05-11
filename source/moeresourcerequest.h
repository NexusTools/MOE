#ifndef MOERESOURCEREQUEST_H
#define MOERESOURCEREQUEST_H

#include "transferdelegate.h"
#include <QThreadStorage>
#include <QScriptEngine>
#include <QMetaMethod>
#include <QJsonValue>

#include "moeobject.h"
#include "moeengine.h"
#include "moeurl.h"

class MoeResourceRequest : public MoeObject
{
    Q_OBJECT
public:
    inline MoeResourceRequest(const char* resource){
        init(MoeUrl::locate(resource));
    }

    inline MoeResourceRequest(QString resource){
        init(MoeUrl::locate(resource));
    }

    Q_INVOKABLE inline explicit MoeResourceRequest(QUrl resource){
        init(resource);
    }

protected slots:
    inline void progressCallback(float prog) {
        emit progress(prog);
    }

    inline void completeCallback(QByteArray dat) {
        static QMetaMethod jsonSignal = metaObject()->method(metaObject()->indexOfSignal("receivedJSON(QScriptValue)"));
        static QMetaMethod stringSignal = metaObject()->method(metaObject()->indexOfSignal("receivedString(QString)"));
        emit receivedData(dat);
        if(isSignalConnected(jsonSignal)) {
            engine()->scriptEngine()->pushContext();
            QScriptValue value = engine()->scriptEngine()->evaluate(QString("(%1)").arg(QString(dat)), "evalJSON");
            engine()->scriptEngine()->popContext();
            emit receivedJSON(value);
        }
        if(isSignalConnected(stringSignal))
            emit receivedString(QString::fromUtf8(dat));

        disconnectAll();
    }

    inline void errorCallback(QString err) {
        emit error(err);
        disconnectAll();
    }

signals:
    void progress(float);
    void receivedData(QByteArray);
    void receivedJSON(QScriptValue);
    void receivedString(QString);
    void error(QString err);
    void completed(bool);

private:
    inline void init(QUrl resource){
        qDebug() << "Downloading" << resource;
        transferDelegate = TransferDelegate::getInstance(resource);
    }

    void connectNotify(const QMetaMethod &signal) {
        QString signalName = signal.name();
        bool complete = signalName.startsWith("complete");
        if((complete || signalName.startsWith("received")) && !receivedConnection)
            receivedConnection = connect(transferDelegate.data(), SIGNAL(receivedData(QByteArray)), this, SLOT(completeCallback(QByteArray)), Qt::QueuedConnection);
        else if((complete || signalName.startsWith("error")) && !errorConnection)
            errorConnection = connect(transferDelegate.data(), SIGNAL(error(QString)), this, SLOT(errorCallback(QString)), Qt::QueuedConnection);
        else if(signalName.startsWith("progress") && !progressConnection)
            progressConnection = connect(transferDelegate.data(), SIGNAL(progress(float)), this, SLOT(progressCallback(float)), Qt::QueuedConnection);
    }

    inline void disconnectAll() {
        if(progressConnection)
            disconnect(progressConnection);
        if(receivedConnection)
            disconnect(receivedConnection);
        if(errorConnection)
            disconnect(errorConnection);
    }

    QMetaObject::Connection progressConnection;
    QMetaObject::Connection receivedConnection;
    QMetaObject::Connection errorConnection;

    static QThreadStorage<QString> context;
    TransferDelegateReference transferDelegate;
};

#endif // MOERESOURCEREQUEST_H
