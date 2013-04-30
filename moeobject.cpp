#include "moeengine.h"
#include "moeobject.h"

MoeObject::MoeObject()
{
    moveToThread(MoeEngine::threadEngine().data());
    connect(engine(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

MoeEngine* MoeObject::engine() const
{
    return qobject_cast<MoeEngine*>(thread());
}
