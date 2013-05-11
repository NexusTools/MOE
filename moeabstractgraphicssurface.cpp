#include "moeabstractgraphicssurface.h"

QThreadStorage<RenderBufferStorage*> RenderBuffer::renderBufferStorage;

MoeAbstractGraphicsSurface* MoeGraphicsObject::surface() {
    MoeGraphicsObject* par = container();
    MoeAbstractGraphicsSurface* surface;
    while(par) {
        surface = qobject_cast<MoeAbstractGraphicsSurface*>(par);
        if(surface)
            return surface;
        else
            par = par->container();
    }
    return NULL;
}

void MoeAbstractGraphicsSurface::render(RenderRecorder* p, QRect region)
{
    //qDebug() << "Rendering" << this;

    bool renderForParent = p != 0;
    if(!renderForParent)
        p = new RenderRecorder(this, repaintDebug == RepaintDebugFrame ? _localGeometry : region);

    MoeGraphicsContainer::render(p, repaintDebug == RepaintDebugFrame ? _localGeometry : region);
    if(!renderForParent) {
        if(repaintDebug) {
            foreach(QRect rect, repaintRegions)
                p->fillRect(rect, repaintColor);

            repaintRegions.clear();
        }

        backend()->renderInstructions(p->instructions(), repaintDebug == RepaintDebugFrame ? _localGeometry : region, _localGeometry.size());
        p->deleteLater();
    }
}

QThreadStorage<MoeAbstractGraphicsSurfacePointer> MoeAbstractGraphicsSurface::_currentSurface;
