#include "transferdelegate.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QMetaMethod>
#include <QFileInfo>
#include <QThread>
#include <QDir>


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

    _progress = 0;
    moveToThread(requestThread);
    derefTimer.setInterval(5000);
    derefTimer.setSingleShot(true);
    derefTimer.moveToThread(requestThread);
    connect(&derefTimer, SIGNAL(timeout()), this, SLOT(dereference()));
    startMethod.invoke(this, Qt::QueuedConnection);
}

void TransferDelegate::startRequest(){
    qDebug() << "Starting" << this << _url;

//    if(_url.isLocalFile() || _url.scheme() == "qrc") {
//        QFileInfo info(_url.scheme() == "qrc" ? QString(":%1").arg(_url.path()) : _url.toLocalFile());
//        qDebug() << this << "is Local";

//        QString _error;
//        QByteArray _data;
//        if(!info.exists())
//            _error = "File doesn't exist";
//        else {
//            if(info.isDir())
//               _error = "Index of emulation not implemented...";
//            else {
//                QFile file(info.absoluteFilePath());
//                if(file.open(QFile::ReadOnly)) {
//                    _data = file.readAll();
//                    if(_data.length() < info.size())
//                        _error = "Unexpected end of stream";
//                } else
//                    _error = "Unable to open for reading";
//            }
//        }

//        QMutexLocker locker(&mutex);
//        _progress = _error.isEmpty() ? 1 : -1;
//        emit progress(_progress);
//        if(_error.isEmpty()) {
//            qDebug() << this << "Finished" << _data.size();
//            emit receivedData(_data);
//        } else {
//            _error = QString("Error reading `%1`: %2").arg(info.filePath()).arg(_error);
//            qDebug() << this << _error;
//            emit error(_error);
//        }
//        return;
//    }

    static QNetworkAccessManager* netMan = 0;
    if(!netMan) //TODO: Add cache support
        netMan = new QNetworkAccessManager();

    reply = netMan->get(QNetworkRequest(QUrl(_url)));
    connect(reply, SIGNAL(finished()), this, SLOT(finishedCallback()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progressCallback(qint64,qint64)));
}
