#include "moegraphicscontainer.h"
#include <renderrecorder.h>

#include <QMetaMethod>
#include <QEvent>

void MoeGraphicsObject::notifyParentOfUpdate()
{
    if(container())
        container()->markChildCacheDirty();
}

bool MoeGraphicsObject::event(QEvent* event) {
    switch(event->type()) {
        case QEvent::Destroy:
        case QEvent::ParentAboutToChange:
            if(container())
                container()->remove(this);
            break;

        case QEvent::ParentChange:
            if(container())
                container()->add(this);
            break;

        default:
            break;
    }

    return QObject::event(event);
}

void MoeGraphicsObject::repaint(QRect region)
{
    if(!container())
        return;

    if(region.isNull()) {
        container()->repaint(_realGeometry);
        return;
    } else
        region &= _localGeometry;

    region = localTransform.mapRect(region);

    if(region.isEmpty())
        return;

    container()->repaint(QRect(region.topLeft(),region.size()));
}

QPoint MoeGraphicsObject::mapFromSurface(QPoint p){
    MoeGraphicsContainer* contain = container();
    while(contain) {
        p = contain->mapFromParent(p);
        contain = contain->container();
    }
    return p;
}

QRect MoeGraphicsObject::mapFromSurface(QRect rect){
    MoeGraphicsContainer* contain = container();
    while(contain) {
        rect = contain->mapFromParent(rect);
        contain = contain->container();
    }
    return rect;
}

bool MoeGraphicsObject::isVisibleToSurface()
{
    return container() && isVisible() && container()->childVisible(this) && container()->isVisibleToSurface();
}

MoeGraphicsContainer* MoeGraphicsObject::container() const{
    return qobject_cast<MoeGraphicsContainer*>(parent());
}

void MoeGraphicsObject::setContainer(MoeGraphicsContainer *contain) {
    contain->add(this);
}

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

    qDebug() << mouseMovedWatchers;
    QListIterator<MoeGraphicsObject*> iterator(mouseMovedWatchers);
    iterator.toBack();
    while(iterator.hasPrevious()) {
        MoeGraphicsObject* child = iterator.previous();
        qDebug() << child << child->realGeometry() << p;
        if(child->realGeometry().contains(p)) {
            child->mouseMovedEvent(child->mapFromParent(p));
            return;
        }
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

    mouseMovedWatchers.clear();
    visibleChildren.clear();
    foreach(MoeGraphicsObject* child, children) {
        MoeGraphicsContainer* contain = qobject_cast<MoeGraphicsContainer*>(child);
        if(contain)
            contain->updateChildCache();
        if(_localGeometry.intersects(child->realGeometry()) && child->isVisible()) {
            visibleChildren.append(child);
            if(child->canUseMouseFocus())
                mouseMovedWatchers.append(child);
        }
    }
    childCacheDirty = false;
    qDebug() << "Updated Child Layout" << this << visibleChildren << mouseMovedWatchers;
}

void MoeGraphicsContainer::updateLayoutTransform(){
    MoeGraphicsObject::updateLayoutTransform();
    markChildCacheDirty();
}
