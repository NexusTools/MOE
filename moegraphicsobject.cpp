#include "moegraphicscontainer.h"
#include "moeengine.h"

#include <QScriptEngine>
#include <QDebug>
#include <qmath.h>

MoeGraphicsObject::MoeGraphicsObject(MoeGraphicsContainer* parent)
{
    setParent(parent);
}

void MoeGraphicsObject::render(RenderRecorder *p, QRect region)
{
    renderImpl(p, region);
    emit paint(p);
}

void MoeGraphicsObject::repaint(QRect region)
{
    qDebug() << "Repaint" << region << this;
    addRepaintRegion(region);
    repaintImpl();
}

void MoeGraphicsObject::setGeometry(QRectF geom){
    repaint(_geometry);
    _geometry = geom;
    repaint(_geometry);
}

MoeGraphicsContainer* MoeGraphicsObject::container() const
{
    return qobject_cast<MoeGraphicsContainer*>(parent());
}
