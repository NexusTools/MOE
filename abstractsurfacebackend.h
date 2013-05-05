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
    virtual void renderInstructions(RenderInstructions instructions, QRect, QSize) =0;
    inline QRect geom() const{return _geom;}
    inline QPoint pos() const{return _geom.topLeft();}
    inline QSize size() const{return _geom.size();}

protected slots:
    virtual void setCursorImpl(QCursor g) =0;
    virtual void setGeometryImpl(QRect g) =0;
    virtual void setTitleImpl(QString g) =0;

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

public slots:
    inline void setCursor(QCursor c) {
        emit updateCursor(c);
    }

    inline void setTitle(QString t) {
        emit updateTitle(t);
    }

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
        connect(&geometryUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGeometryNow()), Qt::QueuedConnection);
        connect(this, SIGNAL(updateCursor(QCursor)), this, SLOT(setCursorImpl(QCursor)), Qt::QueuedConnection);
        connect(this, SIGNAL(updateTitle(QString)), this, SLOT(setTitleImpl(QString)), Qt::QueuedConnection);
        _waitForGeomResize = true;
        _geom = geom;
    }

signals:
    void updateTitle(QString);
    void updateCursor(QCursor);
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
