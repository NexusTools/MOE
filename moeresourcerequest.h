#ifndef MOERESOURCEREQUEST_H
#define MOERESOURCEREQUEST_H

#include "transferdelegate.h"
#include <QThreadStorage>
#include <QScriptEngine>
#include <QMetaMethod>
#include <QJsonValue>

#include "moeobject.h"
#include "moeengine.h"

class MoeResourceRequest : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE inline explicit MoeResourceRequest(QString resource){
        transferDelegate = TransferDelegate::getInstance(resource);
        connect(transferDelegate.data(), SIGNAL(progress(float)), this, SLOT(progressCallback(float)), Qt::QueuedConnection);
        connect(transferDelegate.data(), SIGNAL(receivedData(QByteArray)), this, SLOT(completeCallback(QByteArray)), Qt::QueuedConnection);
        connect(transferDelegate.data(), SIGNAL(error(QString)), this, SLOT(errorCallback(QString)), Qt::QueuedConnection);
    }

protected slots:
    inline void progressCallback(float prog) {
        emit progress(prog);
    }

    inline void completeCallback(QByteArray dat) {
        static QMetaMethod jsonSignal = metaObject()->method(metaObject()->indexOfSignal("receivedJSON(QScriptValue)"));
        emit receivedData(dat);
        if(isSignalConnected(jsonSignal)) {
            engine()->scriptEngine()->pushContext();
            QScriptValue value = engine()->scriptEngine()->evaluate(QString("(%1)").arg(QString(dat)), "evalJSON");
            engine()->scriptEngine()->popContext();
            emit receivedJSON(value);

        }
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
    void error(QString err);

private:
    inline void disconnectAll() {
        disconnect(transferDelegate.data(), SIGNAL(progress(float)), this, SLOT(progressCallback(float)));
        disconnect(transferDelegate.data(), SIGNAL(receivedData(QByteArray)), this, SLOT(completeCallback(QByteArray)));
        disconnect(transferDelegate.data(), SIGNAL(error(QString)), this, SLOT(errorCallback(QString)));
    }

    static QThreadStorage<QString> context;
    TransferDelegateReference transferDelegate;
};

#endif // MOERESOURCEREQUEST_H
