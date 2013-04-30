#include "moeengine.h"
#include "moeobject.h"

MoeObject::MoeObject()
{
    setParent(MoeEngine::threadEngine().data());
}

MoeEngine* MoeObject::engine() const
{
    return qobject_cast<MoeEngine*>(parent());
}
