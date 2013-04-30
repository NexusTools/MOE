#include "transferdelegate.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QMetaMethod>
#include <QThread>
#include <QDebug>

TransferDelegateReference TransferDelegate::getInstance(QString res) {
    static QHash<QString, WeakTransferDelegate> activeDelegates;

    TransferDelegateReference delegate;
    WeakTransferDelegate existing;
    if(activeDelegates.contains(res) && !(existing = activeDelegates.value(res)).isNull()) {
        delegate = existing.toStrongRef();
        QTimer::singleShot(50, delegate.data(), SLOT(emitState()));
        return delegate;
    }

    delegate = TransferDelegateReference(new TransferDelegate(res));
    activeDelegates.insert(res, delegate.toWeakRef());
    return delegate;
}

TransferDelegate::TransferDelegate(QString url) : _url(url)
{
    qDebug() << "Created new TransferDelegate for" << url;

    static QMetaMethod startMethod = metaObject()->method(metaObject()->indexOfMethod("startRequest()"));
    static QThread* requestThread = 0;
    if(!requestThread) {
        requestThread = new QThread();
        requestThread->start();
    }
    moveToThread(requestThread);
    qDebug() << startMethod.isValid() << startMethod.name();
    startMethod.invoke(this, Qt::QueuedConnection);
}

void TransferDelegate::startRequest(){
    qDebug() << "Starting to Download" << _url;
    static QNetworkAccessManager* netMan = 0;
    if(!netMan) {
        netMan = new QNetworkAccessManager();
        //TODO: Add cache support
    }
    reply = netMan->get(QNetworkRequest(QUrl(_url)));
    connect(reply, SIGNAL(finished()), this, SLOT(finishedCallback()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progressCallback(qint64,qint64)));
}
