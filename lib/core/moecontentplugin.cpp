#include "moeengine.h"
#include "moescriptcontent.h"
#include "network/moeresourcerequest.h"

QString MoePlugin::path() const{
    return QString(":/%1/").arg(name());
}

QString MoeContentPlugin::name() const{
    return moduleName();
}

void MoeContentPlugin::startImpl(QUrl loader) {
    MoeUrl::setDefaultContext(path());
    {
        MoeResourceRequest::reset();
        MoeResourceRequest* initRequest = new MoeResourceRequest(loader);
        connect(initRequest, SIGNAL(receivedString(QString)), engine(), SLOT(eval(QString)));
        connect(initRequest, SIGNAL(error(QString)), engine(), SLOT(abort(QString)));
        connect(initRequest, SIGNAL(completed(bool)), engine(), SLOT(deleteLater()), Qt::QueuedConnection);
    }
}

QString MoeScriptContent::name() const{
    return "ScriptContent";
}

QString MoeScriptContent::path() const{
    return _path;
}
