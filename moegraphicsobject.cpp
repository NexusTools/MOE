#include "moegraphicssurface.h"
#include "moeengine.h"

#include <QScriptEngine>
#include <QMetaMethod>
#include <QMetaEnum>
#include <qmath.h>
#include <QEvent>

bool MoeGraphicsObject::canUseKeyFocus(){
    return false;
}

 bool MoeGraphicsObject::isHookConnected(EventHook hook) {
     static QMetaMethod mouseMovedSignal = metaObject()->method(metaObject()->indexOfSignal("mouseMoved(QPoint)"));
     static QMetaMethod mousePressedSignal = metaObject()->method(metaObject()->indexOfSignal("mousePressed(QPoint,int)"));
     static QMetaMethod mouseReleasedSignal = metaObject()->method(metaObject()->indexOfSignal("mouseReleased(QPoint,int)"));

     switch(hook) {
     case mouseMovedHook:
         return isHookRequired(mouseMovedHook) || isSignalConnected(mouseMovedSignal);

     case mousePressedHook:
         return isHookRequired(mousePressedHook) || isSignalConnected(mousePressedSignal);

     case mouseReleasedHook:
         return isHookRequired(mouseReleasedHook) || isSignalConnected(mouseReleasedSignal);

     default:
         return false;
     }
 }

bool MoeGraphicsObject::canUseMouseFocus(){

    return  isHookConnected(mouseMovedHook) || isHookConnected(mousePressedHook) ||
            isHookConnected(mouseReleasedHook);
}

void MoeGraphicsObject::updateHoverFocus(){
    MoeGraphicsSurface::current()->updateHoverTarget(this);
}

void MoeGraphicsObject::notifyParentOfUpdate()
{
    if(container())
        container()->markChildCacheDirty();
}

bool MoeGraphicsObject::event(QEvent * ev)
{
    switch(ev->type()) {
    case QEvent::Destroy:
    case QEvent::ParentAboutToChange:
        if(container())
            container()->remove(this);
        break;

    case QEvent::ParentChange:
    {
        MoeGraphicsContainer* contain = qobject_cast<MoeGraphicsContainer*>(parent());
        qDebug() << "Parent Changed" << contain;
        if(contain)
            contain->add(this);
    }
        break;

    default:
        break;
    }

    return QObject::event(ev);
}

void MoeGraphicsObject::render(RenderRecorder *p, QRect region)
{
    p->pushOpacity(_opacity);
    p->setPen(_foreground);
    paintImpl(p, region);
    emit paint(p);
    p->popOpacity();
}

void MoeGraphicsObject::paintImpl(RenderRecorder* p, QRect)
{
    if(qAlpha(_border) > 0) {
        p->setPen(_border);
        p->setBrush(_background);
        p->drawRect(_localGeometry);
    } else if(qAlpha(_background) > 0)
        p->fillRect(_localGeometry, _background);
}

void MoeGraphicsObject::repaint(QRect region)
{
    if(!isVisibleToSurface())
        return;

    if(region.isNull()) {
        container()->repaint(_realGeometry);

    } else
        region &= _localGeometry;

    region = localTransform.mapRect(region);

    if(region.isEmpty())
        return;

    container()->repaint(QRect(region.topLeft()-QPoint(1,1),region.size()+QSize(2,2)));
}

void MoeGraphicsObject::setGeometry(QRectF geom){
    if(_geometry == geom)
        return;

    repaint();
    QRectF old = _geometry;
    _geometry = geom;
    if(old.size() != geom.size())
        emit resized(geom.size());
    if(old.topLeft() != geom.topLeft())
        emit moved(geom.topLeft());
    updateLayoutTransform();
    repaint();
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

bool MoeGraphicsObject::isVisible() {
    return !_localGeometry.isEmpty() && _opacity > 0;
}

bool MoeGraphicsObject::isVisibleToSurface()
{
    return container() && isVisible() && container()->childVisible(this) && container()->isVisibleToSurface();
}

MoeGraphicsSurface* MoeGraphicsObject::surface() {
    MoeGraphicsObject* par = container();
    MoeGraphicsSurface* surface;
    while(par) {
        surface = qobject_cast<MoeGraphicsSurface*>(par);
        if(surface)
            return surface;
        else
            par = par->container();
    }
    return NULL;
}

void MoeGraphicsObject::updateLayoutTransform() {
    if(isSurface() || container()) {
        _localGeometry = QRect(QPoint(0,0),QSize((int)qCeil(_geometry.width()),(int)qCeil(_geometry.height())));

        localTransform.reset();
        localTransform.translate(_geometry.x(), _geometry.y());

        /*if(_scale != QPointF(1, 1) || _rotate != QPointF(0, 0)) {
            QPointF halfSize(width()/2, height()/2);
            localTransform.translate(halfSize.x(), halfSize.y());
            localTransform.scale(_scale.x(), _scale.y());
            localTransform.rotate(_rotate.x(), Qt::XAxis);
            localTransform.rotate(_rotate.y(), Qt::YAxis);
            parentTransform = localTransform.inverted();
            halfSize = parentTransform.map(-halfSize);
            localTransform.translate(halfSize.x(), halfSize.y());
        }*/

        parentTransform = localTransform.inverted();
        _realGeometry = localTransform.mapRect(_localGeometry);

        notifyParentOfUpdate();
    }
}

void MoeGraphicsObject::setContainer(MoeGraphicsContainer* contain)
{
    if(container() == contain)
        return;

    if(container())
        container()->remove(this);
    if(contain)
        contain->add(this);
    else
        setParent(NULL);
}

MoeGraphicsContainer* MoeGraphicsObject::container() const
{
    return qobject_cast<MoeGraphicsContainer*>(parent());
}

void MoeGraphicsObject::setOpacity(qreal opacity) {
    opacity = qBound<qreal>(0, opacity, 1);
    if(opacity == _opacity)
        return;

    _opacity = opacity;
    if((_opacity == 0 && opacity < 0) ||
            (opacity == 0 && _opacity < 0))
        notifyParentOfUpdate();
    repaint();
}
