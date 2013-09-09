#include "moeenginesystemhook.h"
#include "moeengine.h"

#ifdef Q_OS_LINUX
#include <sys/prctl.h>
#endif

#include <QCoreApplication>

void MoeEngineSystemHook::setup(MoeEngine* eng, char** sysName) {
    MoeEngineSystemHook* sysHook = new MoeEngineSystemHook(eng, sysName);
    sysHook->moveToThread(QCoreApplication::instance()->thread());
    connect(eng, SIGNAL(applicationNameChanged(QString)), sysHook, SLOT(updateSystemAppName(QString)), Qt::QueuedConnection);
}

MoeEngineSystemHook::MoeEngineSystemHook(MoeEngine *engine, char** sysName) {
    qDebug() << "Attaching" << this << this->engine;

    this->sysName = sysName;
    this->engine = engine;
}

void MoeEngineSystemHook::updateSystemAppName(QString name) {
    qDebug() << "Name Update Request" << this << this->engine << name;

#ifdef PR_SET_NAME
    QByteArray nameBytes = name.toLocal8Bit();
    if(nameBytes.startsWith("MOE "))
        nameBytes = nameBytes.mid(4);
    prctl(PR_SET_NAME, nameBytes.left(16).data());
#endif
    QCoreApplication::instance()->setApplicationName(name);
}
