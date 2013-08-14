#ifndef ABSTRACTSURFACEBACKEND_H
#define ABSTRACTSURFACEBACKEND_H

#include "renderinstruction.h"

#include <QCursor>
#include <QTimer>
#include <QDebug>
#include <QRect>

class AbstractSurfaceBackend : public QObject {
    Q_OBJECT

    friend class MoeAbstractGraphicsSurface;
public:
    virtual bool renderInstructions(RenderInstructions instructions, QRect, QSize) =0;
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
        //qDebug() << "Set Geometry" << geom << _geom;
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
        emit geometryChanged(geom);
    }
    inline void updatePos(QPoint pos) {
        updateGeometry(QRect(pos, _geom.size()));
    }
    inline void updateSize(QSize size) {
        updateGeometry(QRect(_geom.topLeft(), size));
    }

protected:
    explicit inline AbstractSurfaceBackend(QRect geom =QRect()) {
        connect(this, SIGNAL(updateCursor(QCursor)), this, SLOT(setCursorImpl(QCursor)), Qt::QueuedConnection);
        connect(this, SIGNAL(updateTitle(QString)), this, SLOT(setTitleImpl(QString)), Qt::QueuedConnection);
        _markedReadyForFrame = false;
        _waitForGeomResize = true;
        _geom = geom;
    }

    inline void markDisconnected() {
        _markedReadyForFrame = false;
        emit disconnected();
    }

    inline void markReadyForFrame() {
        if(_markedReadyForFrame)
            return;

        _markedReadyForFrame = true;
        emit readyForFrame();
    }

    inline void markRendered() {
        _markedReadyForFrame = true;
        emit readyForFrame();
    }

signals:
    void updateTitle(QString);
    void updateCursor(QCursor);
    void geometryChanged(QRect);
    void readyForFrame();
    void disconnected();
    void resetFocus();

    void mouseMove(QPoint);
    void mousePress(QPoint,int);
    void mouseRelease(QPoint,int);

    void keyType(char);
    void keyPress(int);
    void keyRelease(int);

private:
    bool _waitForGeomResize;
    bool _markedReadyForFrame;
    QRect _geom;
};

#endif // ABSTRACTSURFACEBACKEND_H
