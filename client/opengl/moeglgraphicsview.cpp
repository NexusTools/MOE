#include "moeglgraphicsview.h"
#include "gui/renderrecorder.h"
#include "moeglscene.h"

#include <QMatrix4x4>

void MoeGLGraphicsView::render(RenderRecorder * p, QRect geom) {
    if(!renderBuffer)
        renderBuffer = RenderBuffer::instance(this);
    MoeGraphicsObject::render(p, geom);

    QSize cSize = size().toSize();
    if(reportedSize != cSize) {
        p->allocateGLBuffer(renderBuffer, cSize);
        matrix.needsUpdate = true;
        reportedSize = cSize;
    }
    if(matrix.needsUpdate) {
        p->updateGLMatrix(renderBuffer, matrix.toMatrix4x4(size()));
        matrix.needsUpdate = false;
        needsUpdate = true;
    }

    if(needsUpdate) {
        p->beginRenderGLScene(renderBuffer);
        _scene->render(p);
        p->finishRenderGLScene();
        needsUpdate = false;
    }
    p->drawBuffer(renderBuffer, localGeometry());
}

void MoeGLGraphicsView::setScene(MoeGLScene *scene) {
     _scene=scene;
     connect(_scene, SIGNAL(contentChanged()), this, SLOT(update()));
}

void MoeGLGraphicsView::update(){
    needsUpdate = true;
    repaint();
}
