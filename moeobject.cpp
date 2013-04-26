#include "moeengine.h"
#include "moeobject.h"

MoeObject::MoeObject()
{
    setParent((QObject*)MoeEngine::threadEngine());
}

MoeEngine* MoeObject::engine() const
{
    return qobject_cast<MoeEngine*>(parent());
}
