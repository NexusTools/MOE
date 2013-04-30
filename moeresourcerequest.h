#ifndef MOERESOURCEREQUEST_H
#define MOERESOURCEREQUEST_H

#include "transferdelegate.h"

#include "moeobject.h"

class MoeResourceRequest : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeResourceRequest(QString resource){
        transferDelegate = TransferDelegate::getInstance(resource);
        connect(transferDelegate.data(), SIGNAL(progress(float)), this, SLOT(progressCallback(float)));
        connect(transferDelegate.data(), SIGNAL(complete(QByteArray)), this, SLOT(completeCallback(QByteArray)));
        connect(transferDelegate.data(), SIGNAL(error(QString)), this, SLOT(errorCallback(QString)));
    }

protected slots:
    inline void progressCallback(float prog) {
        emit progress(prog);
    }

    inline void completeCallback(QByteArray dat) {
        emit complete(dat);
        disconnectAll();
    }

    inline void errorCallback(QString err) {
        emit error(err);
        disconnectAll();
    }

signals:
    void progress(float);
    void complete(QByteArray);
    void error(QString err);

private:
    inline void disconnectAll() {
        disconnect(transferDelegate.data(), SIGNAL(progress(float)), this, SLOT(progressCallback(float)));
        disconnect(transferDelegate.data(), SIGNAL(complete(QByteArray)), this, SLOT(completeCallback(QByteArray)));
        disconnect(transferDelegate.data(), SIGNAL(error(QString)), this, SLOT(errorCallback(QString)));
    }

    TransferDelegateReference transferDelegate;
};

#endif // MOERESOURCEREQUEST_H
