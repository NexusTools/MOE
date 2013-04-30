#include "moegraphicssurface.h"
#include "moeengine.h"

#include <QScriptEngine>
#include <QMetaEnum>
#include <qmath.h>
#include <QEvent>

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
    if(region.isNull())
        region = _localGeometry;

    if(region.isEmpty()) // Nothing to Render
        return;

    p->setPen(_foreground);
    paintImpl(p, region);
    emit paint(p);
}

void MoeGraphicsObject::paintImpl(RenderRecorder* p, QRect)
{
    if(qAlpha(_border) > 0) {
        p->setPen(_border);
        p->setBrush(_background);
        p->drawRect(QRect(QPoint(0,0),_localGeometry.size()-QSize(1,1)));
    } else if(qAlpha(_background) > 0)
        p->fillRect(_localGeometry, _background);
}

void MoeGraphicsObject::repaint(QRect region)
{
    if(!isVisibleToSurface())
        return;

    if(region.isNull())
        region = _localGeometry;
    else
        region = _localGeometry & region;
    region = localTransform.mapRect(region);

    if(region.isEmpty())
        return;

    container()->repaint(region);
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
    updateLayoutCaches();
    repaint();
}

bool MoeGraphicsObject::isVisibleToSurface()
{
    return container() && container()->childVisible(this) && container()->isVisibleToSurface();
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

void MoeGraphicsObject::updateLayoutCaches() {
    if(isSurface() || container()) {
        _localGeometry = QRect(QPoint(0,0),QSize((int)qCeil(_geometry.width()),(int)qCeil(_geometry.height())));

        localTransform.reset();
        localTransform.translate((int)qFloor(_geometry.x()), (int)qFloor(_geometry.y()));
        localTransform.scale(_scale.x(), _scale.y());
        localTransform.rotate(_rotate.x(), Qt::XAxis);
        localTransform.rotate(_rotate.y(), Qt::YAxis);

        parentTransform = localTransform.inverted();
        _realGeometry = localTransform.mapRect(_localGeometry);

        if(container())
            container()->markChildCacheDirty();
    }
}

void MoeGraphicsObject::connectNotify(const QMetaMethod &){
    if(container())
        container()->markChildCacheDirty();
}

void MoeGraphicsObject::disconnectNotify(const QMetaMethod &){
    if(container())
        container()->markChildCacheDirty();
}

void MoeGraphicsObject::setContainer(MoeGraphicsContainer* contain)
{
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
