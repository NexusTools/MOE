#include "transferdelegate.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QMetaMethod>
#include <QThread>


TransferDelegateReference TransferDelegate::getInstance(QUrl res) {
    static QHash<QUrl, WeakTransferDelegate> activeDelegates;

    TransferDelegateReference delegate;
    WeakTransferDelegate existing;
    if(activeDelegates.contains(res) && !(existing = activeDelegates.value(res)).isNull())
        return existing.toStrongRef();

    delegate = TransferDelegateReference(new TransferDelegate(res));
    activeDelegates.insert(res, delegate.toWeakRef());
    delegate.data()->self = delegate;
    return delegate;
}

TransferDelegate::TransferDelegate(QUrl url) : _url(url)
{
    static QMetaMethod startMethod = metaObject()->method(metaObject()->indexOfMethod("startRequest()"));
    static QThread* requestThread = 0;
    if(!requestThread) {
        requestThread = new QThread();
        requestThread->start();
    }
    moveToThread(requestThread);
    startMethod.invoke(this, Qt::QueuedConnection);
}

void TransferDelegate::startRequest(){
    qDebug() << "Starting" << this << _url;

    static QNetworkAccessManager* netMan = 0;
    if(!netMan) //TODO: Add cache support
        netMan = new QNetworkAccessManager();

    reply = netMan->get(QNetworkRequest(QUrl(_url)));
    connect(reply, SIGNAL(finished()), this, SLOT(finishedCallback()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progressCallback(qint64,qint64)));
    self.clear();
}
