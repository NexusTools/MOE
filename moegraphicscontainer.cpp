#include "moegraphicscontainer.h"
#include <renderrecorder.h>

#include <QMetaMethod>

void MoeGraphicsContainer::render(RenderRecorder * p, QRect region) {
    if(region.isNull())
        region = _localGeometry;

    if(region.isEmpty()) // Nothing to Render
        return;

    p->setPen(_foreground);

    paintImpl(p, region);
    emit paint(p);

    renderChildren(p, region);
}

void MoeGraphicsContainer::renderChildren(RenderRecorder * p, QRect region){
    if(region.isNull())
        region = _localGeometry;

    if(region.isEmpty()) // Nothing to Render
        return;
    updateChildCache();
    foreach(MoeGraphicsObject* graphicsObject, visibleChildren) {
        p->pushClipRect(graphicsObject->realGeometry());
        p->pushTransform(graphicsObject->localTransform);
        graphicsObject->render(p, graphicsObject->mapFromParent(graphicsObject->realGeometry() & region));
        p->popTransform();
        p->popClipRect();
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
    UPDATE_SIGNAL_CACHE(mousePress, "int", obj)
    UPDATE_SIGNAL_CACHE(mouseRelease, "int", obj)
    UPDATE_SIGNAL_CACHE(mouseMove, "QPoint", obj)
    UPDATE_SIGNAL_CACHE(mouseScroll, "QPoint", obj)
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
        if(_localGeometry.intersects(child->realGeometry())) {
            _visibleChildren.append(child);
            if(!visibleChildren.contains(child)) {
                SET_SIGNAL_CACHE(mousePress, "int", child)
                SET_SIGNAL_CACHE(mouseRelease, "int", child)
                SET_SIGNAL_CACHE(mouseMove, "QPoint", child)
                SET_SIGNAL_CACHE(mouseScroll, "QPoint", child)
            }
        } else if(visibleChildren.contains(child))
            removeWatchers(child);
    }
    visibleChildren = _visibleChildren;
    childCacheDirty = false;
}

void MoeGraphicsContainer::updateLayoutCaches(){
    MoeGraphicsObject::updateLayoutCaches();
    markChildCacheDirty();
}
