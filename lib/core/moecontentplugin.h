#ifndef MOECONTENTPLUGIN_H
#define MOECONTENTPLUGIN_H

#include "moeplugin.h"

class MOEGAMEENGINE_EXPORT MoeContentPlugin : public MoePlugin
{
    Q_OBJECT
protected:
    virtual void startImpl() =0;
    virtual void stopImpl() =0;

};

#endif // MOECONTENTPLUGIN_H
