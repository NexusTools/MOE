#ifndef URL_H
#define URL_H

#include "moeobject.h"

#include <QThreadStorage>
#include <QUrl>

class MoeUrl : public MoeObject
{
    Q_OBJECT

    friend class MoeEngine;
public:
    Q_INVOKABLE inline explicit MoeUrl(QString path, QString context =QString()) {setUrl(path, context);}
    Q_INVOKABLE void setUrl(QString path, QString context =QString()) {_url=locate(path, context);}

    inline QString schema() const{return _url.scheme();}
    Q_INVOKABLE inline void setScheme(QString schema) {return _url.setScheme(schema);}

    inline QString host() const{return _url.host();}
    Q_INVOKABLE void setHost(QString host) {return _url.setHost(host);}

    inline QString path() const{return _url.path();}
    Q_INVOKABLE inline void setPath(QString path) {return _url.setPath(path);}

    Q_INVOKABLE bool contains(QUrl url) {
        return url.toString(QUrl::FullyDecoded).startsWith(_url.toString(QUrl::FullyDecoded));
    }

    Q_INVOKABLE inline QString toString() const{
        return _url.toString();
    }

    inline QUrl url() const{
        return _url;
    }

    static QUrl locate(QString path, QString context =QString());

private:
    static QThreadStorage<QUrl> context;
    QUrl _url;

    static QUrl urlFromString(QString path);
    inline static void setDefaultContext(QString _context) {context.setLocalData(locate(_context));}
    inline static QUrl defaultContext() {return context.localData().isEmpty() || context.localData().isRelative() ? QUrl("qrc:/loaders/") : context.localData();}

};

#endif // URL_H
