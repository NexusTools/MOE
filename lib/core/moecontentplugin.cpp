#include "moeengine.h"
#include "moemodule.h"
#include "moescriptcontent.h"
#include "network/moeresourcerequest.h"

QString MoeModule::name() const{
    return metaObject()->className();
}

QString MoePlugin::path() const{
    return QString(":/%1/").arg(name());
}

QString MoeContentPlugin::name() const{
    return moduleName();
}

void MoeContentPlugin::startImpl(QString _loader) {
    QUrl loader = _loader.isEmpty() ? QUrl("loaders://native.js") : MoeUrl::locate(_loader, "loaders://");

    engine()->setApplicationName(metaObject()->className());
    MoeUrl::setDefaultContext(path());
    {
        MoeResourceRequest::reset();
        MoeResourceRequest* initRequest = new MoeResourceRequest(loader);
        connect(initRequest, SIGNAL(receivedString(QString)), engine(), SLOT(eval(QString)));
        connect(initRequest, SIGNAL(error(QString)), engine(), SLOT(abort(QString)));
        connect(initRequest, SIGNAL(completed(bool)), engine(), SLOT(deleteLater()), Qt::QueuedConnection);
    }
}

void MoeScriptContent::startImpl(QString _loader) {
    MoeContentPlugin::startImpl(_loader.isEmpty() ? "loaders://standard.js" : _loader);
}

QString MoeScriptContent::name() const{
    return "ScriptContent";
}

QString MoeScriptContent::path() const{
    return _path;
}
