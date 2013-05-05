#ifndef ABSTRACTSURFACEBACKEND_H
#define ABSTRACTSURFACEBACKEND_H

#include "renderinstruction.h"

#include <QDebug>
#include <QCursor>
#include <QRect>

class AbstractSurfaceBackend : public QObject {
    Q_OBJECT
public:
    inline QRect geom() const{return _geom;}
    inline QPoint pos() const{return _geom.topLeft();}
    inline QSize size() const{return _geom.size();}

public slots:
    virtual void renderInstructions(RenderInstructions instructions, QRect, QSize) =0;
    virtual void setCursor(QCursor c) =0;
    virtual void setGeometryImpl(QRect g) =0;
    virtual void setTitle(QString t) =0;

    inline void setGeometry(QRect geom) {
        if(_geom == geom)
            return;

        _geom = geom;
        qDebug() << "Set Geometry" << geom << _geom;
        setGeometryImpl(geom);
    }
    inline void setSize(QSizeF s) {
        setGeometry(QRect(_geom.topLeft(), s.toSize()));
    }
    inline void setPos(QPointF p) {
        setGeometry(QRect(p.toPoint(), _geom.size()));
    }

protected slots:
    inline void updateGeometry(QRect geom) {
        if(_geom == geom)
            return;

        _geom = geom;
        qDebug() << "Geometry Updated" << geom << _geom;
        emit geometryChanged(geom);
    }
    inline void updatePos(QPoint pos) {
        updateGeometry(QRect(pos, _geom.size()));
    }
    inline void updateSize(QSize size) {
        updateGeometry(QRect(_geom.topLeft(), size));
    }

protected:
    explicit inline AbstractSurfaceBackend(QRect geom) {
        _geom = geom;
    }

signals:
    void geometryChanged(QRect);
    void readyForFrame();
    void disconnected();

    void mouseMove(QPoint);
    void mousePress(QPoint,int);
    void mouseRelease(QPoint,int);

private:
    QRect _geom;
};

#endif // ABSTRACTSURFACEBACKEND_H
