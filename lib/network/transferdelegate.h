#ifndef TRANSFERDELEGATE_H
#define TRANSFERDELEGATE_H

#include <QSharedPointer>
#include <QNetworkReply>
#include <QMutexLocker>
#include <QMetaMethod>
#include <QDebug>
#include <QTimer>

#include "global.h"

class TransferDelegate;

typedef QWeakPointer<TransferDelegate> WeakTransferDelegate;
typedef QSharedPointer<TransferDelegate> TransferDelegateReference;

class MOEGAMEENGINE_EXPORT TransferDelegate : public QObject
{
    Q_OBJECT
public:
    static TransferDelegateReference getInstance(QUrl res);

    inline float currentProgress() {
        QMutexLocker locker(&mutex);
        return _progress > 0 ? _progress : 1;
    }

    inline QByteArray currentData() {
        QMutexLocker locker(&mutex);
        return _progress == 1 ? _data : QByteArray();
    }

    inline QString currentError() {
        QMutexLocker locker(&mutex);
        return _progress == -1 ? QString(_data) : QString("No Error");
    }

signals:
    void progress(float);
    void receivedData(QByteArray);
    void error(QString);

protected slots:
    void startRequest();
    inline void dereference() {self.clear();}

    inline void progressCallback(qint64 cur, qint64 max) {
        QMutexLocker locker(&mutex);
        _progress = (qreal)cur / (qreal)max;
    }

    void finishedCallback() {
        QMutexLocker locker(&mutex);
        _progress = reply->error() ? -1 : 1;
        reply->deleteLater();
        emit progress(_progress);
        if(reply->error()) {
            _data = reply->errorString().toLocal8Bit();
            emit error(QString(_data));
        } else {
            _data = reply->readAll();
            emit receivedData(_data);
        }
        derefTimer.start();
    }

private:
    TransferDelegate(QUrl);
    TransferDelegateReference self;

    inline void connectNotify(const QMetaMethod &signal) {
        QMutexLocker locker(&mutex);
        QString name(signal.name());
        if(name.startsWith("progress") && _progress > 0)
            emit progress(_progress);
        else if(name.startsWith("error") && _progress == -1)
            emit error(QString(_data));
        else if(name.startsWith("receivedData") && _progress >= 1)
            emit receivedData(_data);
    }

    QMutex mutex;
    const QUrl _url;
    float _progress;
    QByteArray _data; // error if -1
    QNetworkReply* reply;
    QTimer derefTimer;

};

#endif // TRANSFERDELEGATE_H
