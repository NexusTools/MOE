#include "moegraphicscontainer.h"
#include <renderrecorder.h>

#include <QMetaMethod>

void MoeGraphicsContainer::render(RenderRecorder * p, QRect region) {
    p->setPen(_foreground);

    paintImpl(p, region);
    emit paint(p);

    renderChildren(p, region);
}

void MoeGraphicsContainer::renderChildren(RenderRecorder * p, QRect region){
    updateChildCache();
    foreach(MoeGraphicsObject* graphicsObject, visibleChildren) {
        if(!region.intersects(graphicsObject->realGeometry()))
            continue;

        p->pushClipRect(graphicsObject->realGeometry());
        p->pushTransform(graphicsObject->localTransform);
        graphicsObject->render(p, graphicsObject->mapFromParent(graphicsObject->realGeometry() & region));
        p->popTransform();
        p->popClipRect();
    }
}

void MoeGraphicsContainer::mouseMovedEvent(QPoint p){
    updateChildCache();
    if(canUseMouseFocus())
        MoeGraphicsObject::mouseMovedEvent(p);
    qDebug() << "Mouse Moved Event" << p << mouseMovedWatchers.size();
    foreach(MoeGraphicsObject* child, mouseMovedWatchers)
        if(child->realGeometry().contains(p)) {
            child->mouseMovedEvent(child->mapFromParent(p));
            return;
        }
}

void MoeGraphicsContainer::markChildCacheDirty() {
    childCacheDirty = true;
    if(container())
        container()->markChildCacheDirty();
}

void MoeGraphicsContainer::updateChildCache(){
    if(!childCacheDirty)
        return;

    visibleChildren.clear();
    foreach(MoeGraphicsObject* child, children) {
        MoeGraphicsContainer* contain = qobject_cast<MoeGraphicsContainer*>(child);
        if(contain)
            contain->updateChildCache();
        if(_localGeometry.intersects(child->realGeometry()) && child->isVisible()) {
            visibleChildren.append(child);
            if(child->canUseMouseFocus()) {
                if(!mouseMovedWatchers.contains(child))
                    mouseMovedWatchers.append(child);
            } else if(mouseMovedWatchers.contains(child))
                mouseMovedWatchers.removeOne(child);
        } else if(mouseMovedWatchers.contains(child))
            mouseMovedWatchers.removeOne(child);
    }
    childCacheDirty = false;
}

void MoeGraphicsContainer::updateLayoutTransform(){
    MoeGraphicsObject::updateLayoutTransform();
    markChildCacheDirty();
}
