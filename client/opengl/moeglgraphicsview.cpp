#include "moeglgraphicsview.h"
#include "gui/renderrecorder.h"

void MoeGLGraphicsView::render(RenderRecorder * p, QRect geom) {
    if(!renderBuffer)
        renderBuffer = RenderBuffer::instance(this);
    MoeGraphicsObject::render(p, geom);

    if(needsUpdate) {
        p->updateGLScene(renderBuffer, size().toSize());
        needsUpdate = false;
    }
    p->drawBuffer(renderBuffer, localGeometry());
}
