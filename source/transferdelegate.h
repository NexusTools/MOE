#ifndef TRANSFERDELEGATE_H
#define TRANSFERDELEGATE_H

#include <QSharedPointer>
#include <QNetworkReply>
#include <QMutexLocker>
#include <QMetaMethod>
#include <QDebug>

#include <QTimer>

class TransferDelegate;

typedef QWeakPointer<TransferDelegate> WeakTransferDelegate;
typedef QSharedPointer<TransferDelegate> TransferDelegateReference;

class TransferDelegate : public QObject
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

    inline void progressCallback(qint64 cur, qint64 max) {
        QMutexLocker locker(&mutex);
        _progress = (qreal)cur / (qreal)max;
    }

    void finishedCallback() {
        QMutexLocker locker(&mutex);
        _progress = reply->error() ? -1 : 1;
        reply->deleteLater();
        emit progress(_progress);
        qDebug() << this << "Finished" << reply->errorString();
        if(reply->error()) {
            _data = reply->errorString().toLocal8Bit();
            emit error(reply->errorString());
        } else {
            _data = reply->readAll();
            emit receivedData(_data);
         }
    }

private:
    TransferDelegate(QUrl);
    TransferDelegateReference self;

    inline void connectNotify(const QMetaMethod &signal) {
        qDebug() << "Connected" << this << signal.name();
    }

    QMutex mutex;
    const QUrl _url;
    float _progress;
    QByteArray _data; // error if -1
    QNetworkReply* reply;

};

#endif // TRANSFERDELEGATE_H
