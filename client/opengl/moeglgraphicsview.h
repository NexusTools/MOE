#ifndef MOEGLGRAPHICSVIEW_H
#define MOEGLGRAPHICSVIEW_H

#include <gui/moegraphicsobject.h>

class RenderBuffer;

class MoeGLGraphicsView : public MoeGraphicsObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLGraphicsView(MoeObject* parent =0)
            : MoeGraphicsObject(parent) {renderBuffer = 0;needsUpdate=true;}
    virtual void render(RenderRecorder*, QRect);

private:
    RenderBuffer* renderBuffer;
    bool needsUpdate;
};

#endif // MOEGLGRAPHICSVIEW_H
