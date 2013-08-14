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

QThreadStorage<MoeAbstractGraphicsSurfacePointer> MoeAbstractGraphicsSurface::_currentSurface;
