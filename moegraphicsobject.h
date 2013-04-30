#ifndef MOEGRAPHICSOBJECT_H
#define MOEGRAPHICSOBJECT_H

#include "moeobject.h"
#include "renderrecorder.h"

#include <QScriptValue>
#include <QRectF>
#include <qmath.h>

class RenderRecorder;
class MoeGraphicsContainer;

class MoeGraphicsObject : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE MoeGraphicsObject(MoeGraphicsContainer* parent =0);

    virtual void render(RenderRecorder*, QRect =QRect());

    Q_INVOKABLE inline qreal x() const{return _geometry.x();}
    Q_INVOKABLE inline qreal y() const{return _geometry.y();}
    Q_INVOKABLE inline QPointF pos() const{return _geometry.topLeft();}

    Q_INVOKABLE inline qreal width() const{return _geometry.width();}
    Q_INVOKABLE inline qreal height() const{return _geometry.height();}
    Q_INVOKABLE inline QSizeF size() const{return _geometry.size();}

    Q_INVOKABLE inline QRectF geomtry() const{return _geometry;}

    Q_INVOKABLE inline bool contains(QPointF p) const{return _geometry.contains(p);}
    Q_INVOKABLE inline bool contains(QRectF r) const{return _geometry.contains(r);}

    Q_INVOKABLE MoeGraphicsContainer* container() const;

public slots:
    void repaint(QRect =QRect());
    inline void repaint(QRectF rec) {repaint(QRect((int)qFloor(rec.x()),(int)qFloor(rec.y()),(int)qCeil(rec.width()),(int)qCeil(rec.height())));}
    inline void setPos(QPointF p) {setGeometry(QRectF(p, size()));}
    inline void setPos(qreal x, qreal y) {setPos(QPointF(x, y));}
    inline void setSize(QSizeF s) {setGeometry(QRectF(pos(), s));}
    inline void setSize(qreal w, qreal h) {setSize(QSizeF(w, h));}
    inline void setGeometry(qreal x, qreal y, qreal w, qreal h) {setGeometry(QRectF(x,y,w,h));}
    void setGeometry(QRectF);

signals:
    void paint(RenderRecorder*);

protected:
    virtual inline void repaintImpl() {}
    virtual inline void renderImpl(RenderRecorder*, QRect) {}
    inline void addRepaintRegion(QRect region) {repaintRegion |= region;}

private:
    bool usePixmapBuffer;

    QRect repaintRegion;
    QRectF _geometry;
};

#endif // MOEGRAPHICSOBJECT_H
