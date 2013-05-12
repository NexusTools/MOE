#include "moegraphicsobject.h"
#include "renderrecorder.h"

#include <QScriptEngine>
#include <QMetaMethod>
#include <qmath.h>
#include <QEvent>

bool MoeGraphicsObject::canUseKeyFocus(){
    return false;
}

 bool MoeGraphicsObject::isHookConnected(EventHook hook) {
     static QMetaMethod mouseMovedSignal = metaObject()->method(metaObject()->indexOfSignal("mouseMoved(QPoint)"));
     static QMetaMethod mousePressedSignal = metaObject()->method(metaObject()->indexOfSignal("mousePressed(QPoint,int)"));
     static QMetaMethod mouseReleasedSignal = metaObject()->method(metaObject()->indexOfSignal("mouseReleased(QPoint,int)"));
     static QMetaMethod mouseScrolledSignal = metaObject()->method(metaObject()->indexOfSignal("mouseScrolled(QPoint)"));
     static QMetaMethod mouseEnteredSignal = metaObject()->method(metaObject()->indexOfSignal("mouseEntered()"));
     static QMetaMethod mouseLeftSignal = metaObject()->method(metaObject()->indexOfSignal("mouseLeft()"));

     switch(hook) {
     case mouseMovedHook:
         return isHookRequired(mouseMovedHook) || isSignalConnected(mouseMovedSignal);

     case mousePressedHook:
         return isHookRequired(mousePressedHook) || isSignalConnected(mousePressedSignal);

     case mouseReleasedHook:
         return isHookRequired(mouseReleasedHook) || isSignalConnected(mouseReleasedSignal);

     case mouseScrolledHook:
         return isHookRequired(mouseScrolledHook) || isSignalConnected(mouseScrolledSignal);

     case mouseEnteredHook:
         return isHookRequired(mouseEnteredHook) || isSignalConnected(mouseEnteredSignal);

     case mouseLeftHook:
         return isHookRequired(mouseLeftHook) || isSignalConnected(mouseLeftSignal);

     default:
         return false;
     }
 }

bool MoeGraphicsObject::canUseMouseFocus(){
    return  _cursor.shape() != Qt::ArrowCursor || canUseKeyFocus() ||
            isHookConnected(mouseMovedHook) || isHookConnected(mousePressedHook) ||
            isHookConnected(mouseReleasedHook) || isHookConnected(mouseEnteredHook) ||
            isHookConnected(mouseLeftHook) || isHookConnected(mouseScrolledHook);
}

void MoeGraphicsObject::render(RenderRecorder *p, QRect region)
{
    p->setPen(_foreground, 1);
    paintImpl(p, region);
    emit paint(p);
}

void MoeGraphicsObject::paintImpl(RenderRecorder* p, QRect)
{
    if(_border.alpha() > 0 || _borderRadius > 0 ||
            !_background.isOpaque() || _background.gradient()) {
        p->setPen(_border);
        p->setBrush(_background);
        p->drawRect(_localGeometry, _borderRadius);
    } else
        p->fillRect(_localGeometry, _background.color());
}

void MoeGraphicsObject::setGeometry(QRectF geom){
    if(_geometry == geom)
        return;

    QRectF old(_geometry);
    _geometry = geom;
    if(old.topLeft() != geom.topLeft() && container())
        repaint();
    updateLayoutTransform();
    if(old.size() != geom.size()) {
        emit resized(geom.size());
        repaint();
    }
    if(old.topLeft() != geom.topLeft()) {
        emit moved(geom.topLeft());
        if(container())
            repaint();
    }
}

bool MoeGraphicsObject::isVisible() {
    return !_localGeometry.isEmpty() && _opacity > 0;
}

void MoeGraphicsObject::updateLayoutTransform() {
    if(isSurface() || container()) {
        _localGeometry = QRect(QPoint(0,0),QSize((int)qCeil(_geometry.width()),(int)qCeil(_geometry.height())));

        localTransform.reset();
        localTransform.translate(_geometry.x(), _geometry.y());

        if(_scale != QPointF(1, 1) || _rotate != QPointF(0, 0)) {
            QPointF halfSize(width()/2, height()/2);
            localTransform.translate(halfSize.x(), halfSize.y());
            localTransform.scale(_scale.x(), _scale.y());
            localTransform.rotate(_rotate.x(), Qt::XAxis);
            localTransform.rotate(_rotate.y(), Qt::YAxis);

            localTransform.translate(-halfSize.x(), -halfSize.y());
        }

        parentTransform = localTransform.inverted();
        _realGeometry = localTransform.mapRect(_localGeometry);

        notifyParentOfUpdate();
    }
}

void MoeGraphicsObject::setCursor(QCursor cursor){
    if(_cursor.shape() == cursor.shape())
        return;
    _cursor = cursor;
    emit cursorChanged(cursor);
    notifyParentOfUpdate();
}

void MoeGraphicsObject::setOpacity(qreal opacity) {
    opacity = qBound<qreal>(0, opacity, 1);
    if(opacity == _opacity)
        return;

    qreal oldOpacity = _opacity;
    _opacity = opacity;
    if((oldOpacity == 0 && opacity > 0) ||
            (opacity == 0 && oldOpacity > 0))
        updateLayoutTransform();
    repaint();
}
