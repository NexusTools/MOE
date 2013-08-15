#ifndef MOEPLUGIN_H
#define MOEPLUGIN_H

#include "global.h"
#include "moe-macros.h"
#include <moduleplugin.h>

class MoeEngine;

class MOEGAMEENGINE_EXPORT MoePlugin : public ModulePlugin
{
    Q_OBJECT

public:
    inline explicit MoePlugin() {}

    virtual QString name() const=0;
    virtual QString path() const;

    inline MoeEngine* engine() const{return (MoeEngine*)moduleCore();}

protected:
    inline explicit MoePlugin(MoeEngine* eng) : ModulePlugin((ModularCore*)eng) {}

};

#endif // MOEPLUGIN_H
