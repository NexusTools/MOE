#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moegraphicscontainer.h"

class MoeEngineView;
class MoeEngineGraphicsObject;

class MoeGraphicsSurface : public MoeGraphicsContainer
{
public:
    struct RenderInstruction
    {
        enum Type {

        } type;


    };

    MoeGraphicsSurface();

signals:
    void preparedLayer(QVector<RenderInstruction> instructions);
};

#endif // MOEGRAPHICSSURFACE_H
