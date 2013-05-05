#include "moeengine.h"
#include "moeobject.h"
#include <QMetaProperty>
#include <QStringList>
#include <QDebug>
#include <QColor>

QThreadStorage<MoeObjectPtrMap> MoeObject::instances;

MoeObject::MoeObject(MoeObject* parent) : QObject(parent)
{
    moveToThread(MoeEngine::threadEngine().data());
    instances.localData().insert(ptr(), MoeObjectPointer(this));
    connect(engine(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

MoeEngine* MoeObject::engine() const
{
    return qobject_cast<MoeEngine*>(thread());
}

QString MoeObject::toString() const {
    QString toString = metaObject()->className();
    toString += '(';
    for(int i=0; i<metaObject()->propertyCount(); i++) {
        if(i > 0)
            toString += ", ";

        QString debugBuffer;
        QDebug debug(&debugBuffer);
        QMetaProperty prop = metaObject()->property(i);
        debug << prop.read(this).toString();
        toString += QString("%1 = %2").arg(prop.name(), debugBuffer);
    }
    toString += ')';

    return toString;
}
