#ifndef TRANSFERDELEGATE_H
#define TRANSFERDELEGATE_H

#include <QSharedPointer>
#include <QNetworkReply>
#include <QMutexLocker>
#include <QDebug>
#include <QTimer>

class TransferDelegate;

typedef QWeakPointer<TransferDelegate> WeakTransferDelegate;
typedef QSharedPointer<TransferDelegate> TransferDelegateReference;

class TransferDelegate : public QObject
{
    Q_OBJECT
public:
    static TransferDelegateReference getInstance(QString res);

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

protected slots:
    inline void emitState(){
        if(_progress == 1)
            emit receivedData(currentData());
        else if(_progress >= 0)
            emit progress(currentProgress());
        else
            emit error(currentError());
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
        if(reply->error()) {
            _data = reply->errorString().toLocal8Bit();
            emit error(reply->errorString());
        } else {
            _data = reply->readAll();
            emit receivedData(_data);
         }
    }

private:
    TransferDelegate(QString);
    TransferDelegateReference self;

    QMutex mutex;
    const QString _url;
    float _progress;
    QByteArray _data; // error if -1
    QNetworkReply* reply;

};

#endif // TRANSFERDELEGATE_H
