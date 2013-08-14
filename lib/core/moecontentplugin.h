#ifndef MOECONTENTPLUGIN_H
#define MOECONTENTPLUGIN_H

#include "moeplugin.h"

class MOEGAMEENGINE_EXPORT MoeContentPlugin : public MoePlugin
{
    Q_OBJECT

    friend class MoeEngine;
public:
    inline explicit MoeContentPlugin() : MoePlugin() {}
    virtual QString name() const;

protected:
    inline explicit MoeContentPlugin(MoeEngine* eng) : MoePlugin(eng) {}

    virtual void startImpl(QUrl loader);
    virtual void stopImpl() {}


};

#endif // MOECONTENTPLUGIN_H
