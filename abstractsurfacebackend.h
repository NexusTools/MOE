#ifndef ABSTRACTSURFACEBACKEND_H
#define ABSTRACTSURFACEBACKEND_H

#include "renderinstruction.h"

#include <QCursor>
#include <QTimer>
#include <QDebug>
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
        if(_geom == geom) {
            _waitForGeomResize = false;
            return;
        }
        if(_waitForGeomResize)
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
        _waitForGeomResize = true;
        geometryUpdateTimer.start();
    }
    inline void updatePos(QPoint pos) {
        updateGeometry(QRect(pos, _geom.size()));
    }
    inline void updateSize(QSize size) {
        updateGeometry(QRect(_geom.topLeft(), size));
    }

private slots:
    inline void updateGeometryNow() {
        emit geometryChanged(_geom);
    }

protected:
    explicit inline AbstractSurfaceBackend(QRect geom =QRect()) {
        geometryUpdateTimer.setSingleShot(true);
        geometryUpdateTimer.setInterval(0);
        connect(&geometryUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGeometryNow()));
        _waitForGeomResize = true;
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
    QTimer geometryUpdateTimer;
    bool _waitForGeomResize;
    QRect _geom;
};

#endif // ABSTRACTSURFACEBACKEND_H
