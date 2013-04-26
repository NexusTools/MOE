#ifndef MOEGRAPHICSOBJECT_H
#define MOEGRAPHICSOBJECT_H

#include "moeobject.h"

class QRenderRecorder;

class MoeGraphicsObject : public MoeObject
{
public:
    MoeGraphicsObject();

    virtual void render(QRenderRecorder&);

protected:
    void callRenderFunction();
    virtual inline void renderImpl(QRenderRecorder&) {}
};

#endif // MOEGRAPHICSOBJECT_H
