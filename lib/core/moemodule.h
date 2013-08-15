#ifndef MOEMODULE_H
#define MOEMODULE_H

#include "moeplugin.h"

class MoeModule : public MoePlugin
{
public:
    virtual QString name() const;

protected:
    inline explicit MoeModule() {}

};

#endif // MOEMODULE_H
