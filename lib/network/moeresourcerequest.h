#ifndef MOERESOURCEREQUEST_H
#define MOERESOURCEREQUEST_H

#include "transferdelegate.h"
#include <QThreadStorage>
#include <QScriptEngine>
#include <QStringList>
#include <QMetaMethod>
#include <QHash>

#include "core/moeobject.h"
#include "global.h"
#include "moeurl.h"

typedef QHash<QString, QString> QHashString;

class MOEGAMEENGINE_EXPORT MoeResourceRequest : public MoeObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url)
public:
    inline MoeResourceRequest(const char* resource){
        init(MoeUrl::locate(resource));
    }
    inline MoeResourceRequest(QString resource){
        init(MoeUrl::locate(resource));
    }
    Q_INVOKABLE inline explicit MoeResourceRequest(QUrl resource){
        init(resource.isRelative() ? MoeUrl::locate(resource.toString()) : resource);
    }
    virtual ~MoeResourceRequest() {
        disconnect(receivedConnection);
    }

    inline QUrl url() const{
        return _url;
    }

    static inline void reset() {
        bindPath("res://", "qrc:/");
        bindPath("data://", "qrc:/resources/");
        bindPath("libs://", "qrc:/libraries/");
        bindPath("examples://", "qrc:/examples/");
        bindPath("settings://", "qrc:/settings/");
        bindPath("loaders://", "qrc:/loaders/");
    }

    static inline void bindPath(QString from, QString to) {
        if(!to.endsWith('/'))
            to += '/';
        if(!from.endsWith('/'))
            from += '/';
        qDebug() << "Binding" << from << to;
        boundPaths.localData().insert(from, to);
    }

protected slots:
    inline void progressCallback(float prog) {
        if(!progressConnection)
            return;
        emit progress(prog);
    }

    void completeCallback(QByteArray dat);

    inline void errorCallback(QString err) {
        if(!errorConnection)
            return;
        emit error(err);
        disconnectAll();
    }

signals:
    void progress(float);
    void receivedData(QByteArray);
    void receivedJSON(QScriptValue);
    void receivedString(QString);
    void receivedChildList(QStringList);
    void receivedXML(QVariantMap);
    void error(QString err);
    void completed(bool);

private:
    inline void init(QUrl resource){
        _url = resource;

        QString fetch = resource.toString();
        qDebug() << "Looking up" << fetch;
        QHashIterator<QString, QString> i(boundPaths.localData());
        while(i.hasNext()) {
            i.next();
            if(fetch.startsWith(i.key())) {
                fetch = i.value() + fetch.mid(i.key().length());
                qDebug() << i.key() << i.value() << fetch;
                resource = QUrl(fetch);
                break;
            }
        }
        _realBase = fetch;
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
        disconnect(progressConnection);
        disconnect(receivedConnection);
        disconnect(errorConnection);
    }

    QMetaObject::Connection progressConnection;
    QMetaObject::Connection receivedConnection;
    QMetaObject::Connection errorConnection;

    QUrl _url;
    QString _realBase;
    TransferDelegateReference transferDelegate;

    static QThreadStorage<QString> context;
    static QThreadStorage<QHashString> boundPaths;
};

#endif // MOERESOURCEREQUEST_H
