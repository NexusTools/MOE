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
    qDebug() << "Downloading" << _url;

    if(_url.isLocalFile() || _url.scheme() == "qrc") {
        QFileInfo info(_url.scheme() == "qrc" ? QString(":%1").arg(_url.path()) : _url.toLocalFile());

        QString _error;
        QByteArray data;
        if(!info.exists())
            _error = "File doesn't exist";
        else {
            if(info.isDir()) {
                data = "<html><head></head><body><h1>Directory listing: ";
                data += info.absoluteFilePath();
                data += "</h1><hr /><ul>";
                foreach(QFileInfo childInfo, QDir(info.absoluteFilePath()).entryInfoList()) {
                    data += "<a href='";
                    data += childInfo.absoluteFilePath();
                    if(childInfo.isDir())
                        data += '/';
                    data += "'>";
                    data += childInfo.fileName();
                    data += "</a>";
                }
                data += "</ul></body></html>";
            } else {
                QFile file(info.absoluteFilePath());
                if(file.open(QFile::ReadOnly)) {
                    data = file.readAll();
                    if(data.length() < info.size())
                        _error = "Unexpected end of stream";
                } else
                    _error = "Unable to open for reading";
            }
        }

        QMutexLocker locker(&mutex);
        if(_error.isEmpty()) {
            _data = data;
            _progress = 1;
            emit progress(_progress);
            emit receivedData(_data);
        } else {
            _error = QString("Error reading `%1`: %2").arg(info.filePath()).arg(_error);
            _progress = -1;
            _data = _error.toLocal8Bit();
            emit progress(_progress);
            emit error(_error);
        }
        return;
    }

    static QNetworkAccessManager* netMan = 0;
    if(!netMan) //TODO: Add cache support
        netMan = new QNetworkAccessManager();

    reply = netMan->get(QNetworkRequest(QUrl(_url)));
    connect(reply, SIGNAL(finished()), this, SLOT(finishedCallback()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progressCallback(qint64,qint64)));
}
