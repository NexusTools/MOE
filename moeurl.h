#ifndef URL_H
#define URL_H

#include "moeobject.h"

#include <QThreadStorage>
#include <QUrl>

class MoeUrl : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE inline explicit MoeUrl(QString path, QUrl context =QUrl()) {setUrl(path, context);}
    Q_INVOKABLE void setUrl(QString path, QUrl context =QUrl()) {url=locate(path, context);}

    Q_INVOKABLE inline static void setDefaultContext(QUrl _context) {context.setLocalData(_context);}
    Q_INVOKABLE inline static QUrl defaultContext() {return context.localData().isEmpty() || context.localData().isRelative() ? QUrl(":/data/content-select") : context.localData();}

    inline QString schema() const{return url.scheme();}
    Q_INVOKABLE inline void setScheme(QString schema) {return url.setScheme(schema);}

    inline QString host() const{return url.host();}
    Q_INVOKABLE void setHost(QString host) {return url.setHost(host);}

    inline QString path() const{return url.path();}
    Q_INVOKABLE inline void setPath(QString path) {return url.setPath(path);}

    Q_INVOKABLE inline QString toString() const{
        return url.toString();
    }

    static QUrl locate(QString path, QUrl context =QString());

private:
    static QThreadStorage<QUrl> context;
    QUrl url;
    
};

#endif // URL_H
