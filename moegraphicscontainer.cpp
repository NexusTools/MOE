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

void MoeGraphicsContainer::mouseMovedImpl(QPoint p){
    updateChildCache();
    foreach(MoeGraphicsObject* child, mouseMovedWatchers)
        if(child->realGeometry().contains(p)) {
            child->mouseMovedImpl(child->mapFromParent(p));
            break;
        }
}

#define UPDATE_SIGNAL_CACHE(signal, params, object) \
    static const QMetaMethod signal##Meta = metaObject()->method(metaObject()->indexOfSignal(#signal "(" params ")")); \
    if(visibleChildren.contains(object) && (object->requireHook(signal##Hook) \
                || object->isSignalConnected(signal##Meta))) { \
        if(!signal##Watchers.contains(object)) \
            signal##Watchers.append(object); \
    } else if(signal##Watchers.contains(object)) \
        signal##Watchers.removeOne(object);

#define SET_SIGNAL_CACHE(signal, params, object) \
    static const QMetaMethod signal##Meta = metaObject()->method(metaObject()->indexOfSignal(#signal "(" params ")")); \
    if(object->requireHook(signal##Hook) || object->isSignalConnected(signal##Meta)) { \
        if(!signal##Watchers.contains(object)) \
            signal##Watchers.append(object); \
    } else if(signal##Watchers.contains(object)) \
        signal##Watchers.removeOne(object);

void MoeGraphicsContainer::updateWatcherCache(MoeGraphicsObject* obj){
    UPDATE_SIGNAL_CACHE(mousePressed, "int", obj)
    UPDATE_SIGNAL_CACHE(mouseReleased, "int", obj)
    UPDATE_SIGNAL_CACHE(mouseMoved, "QPoint", obj)
    UPDATE_SIGNAL_CACHE(mouseDragged, "QPoint", obj)
    UPDATE_SIGNAL_CACHE(mouseScrolled, "QPoint", obj)

    UPDATE_SIGNAL_CACHE(keyPressed, "int", obj)
    UPDATE_SIGNAL_CACHE(keyReleased, "int", obj)
    UPDATE_SIGNAL_CACHE(keyTyped, "char", obj)
}

void MoeGraphicsContainer::markChildCacheDirty() {
    childCacheDirty = true;
    if(container())
        container()->markChildCacheDirty();
}

void MoeGraphicsContainer::updateChildCache(){
    if(!childCacheDirty)
        return;

    QList<MoeGraphicsObject*> _visibleChildren;
    foreach(MoeGraphicsObject* child, children) {
        MoeGraphicsContainer* contain = qobject_cast<MoeGraphicsContainer*>(child);
        if(contain)
            contain->updateChildCache();
        if(_localGeometry.intersects(child->realGeometry()) && child->isVisible()) {
            _visibleChildren.append(child);
            if(!visibleChildren.contains(child)) {
                SET_SIGNAL_CACHE(mousePressed, "int", child)
                SET_SIGNAL_CACHE(mouseReleased, "int", child)
                SET_SIGNAL_CACHE(mouseMoved, "QPoint", child)
                SET_SIGNAL_CACHE(mouseDragged, "QPoint", child)
                SET_SIGNAL_CACHE(mouseScrolled, "QPoint", child)

                SET_SIGNAL_CACHE(keyPressed, "int", child)
                SET_SIGNAL_CACHE(keyReleased, "int", child)
                SET_SIGNAL_CACHE(keyTyped, "char", child)
            }
        } else if(visibleChildren.contains(child))
            removeWatchers(child);
    }
    visibleChildren = _visibleChildren;
    childCacheDirty = false;
}

void MoeGraphicsContainer::updateLayoutTransform(){
    MoeGraphicsObject::updateLayoutTransform();
    markChildCacheDirty();
}
