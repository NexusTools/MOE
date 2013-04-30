#ifndef MOEGRAPHICSOBJECT_H
#define MOEGRAPHICSOBJECT_H

#include "moeobject.h"

#include <QTransform>
#include <qmath.h>
#include <QRectF>
#include <QRgb>

class RenderRecorder;
class MoeGraphicsContainer;
class MoeGraphicsSurface;

class MoeGraphicsObject : public MoeObject
{
    Q_OBJECT
    Q_PROPERTY(QRgb background READ background WRITE setBackground)
    Q_PROPERTY(QRgb foreground READ foreground WRITE setForeground)
    Q_PROPERTY(QRgb border READ border WRITE setBorder)
public:
    Q_INVOKABLE MoeGraphicsObject(MoeGraphicsContainer* parent =0) : _scale(1, 1) {
        _foreground = qRgb(0, 0, 0);
        setContainer(parent);
    }

    bool event(QEvent *);
    virtual void render(RenderRecorder*, QRect =QRect());
    Q_INVOKABLE virtual void paintImpl(RenderRecorder*, QRect);

    Q_INVOKABLE inline qreal x() const{return _geometry.x();}
    Q_INVOKABLE inline qreal y() const{return _geometry.y();}
    Q_INVOKABLE inline QPointF pos() const{return _geometry.topLeft();}

    Q_INVOKABLE inline QSizeF size() const{return _geometry.size();}
    Q_INVOKABLE inline qreal width() const{return _geometry.width();}
    Q_INVOKABLE inline qreal height() const{return _geometry.height();}

    Q_INVOKABLE inline QRectF geomtry() const{return _geometry;}
    Q_INVOKABLE inline QRect realGeometry() const{return _realGeometry;}

    inline QRgb background() const{return _background;}
    inline QRgb foreground() const{return _foreground;}
    inline QRgb border() const{return _border;}
    Q_INVOKABLE inline void setBackground(QRgb c) {_background=c;repaint();}
    Q_INVOKABLE inline void setForeground(QRgb c) {_foreground=c;repaint();}
    Q_INVOKABLE inline void setBorder(QRgb c) {_border=c;repaint();}

    Q_INVOKABLE inline bool contains(QPointF p) const{return _geometry.contains(p);}
    Q_INVOKABLE inline bool contains(QRectF r) const{return _geometry.contains(r);}

    Q_INVOKABLE void setContainer(MoeGraphicsContainer*);
    Q_INVOKABLE MoeGraphicsContainer* container() const;

    Q_INVOKABLE QRect mapToParent(QRect rect){
        return localTransform.mapRect(rect);
    }

    Q_INVOKABLE QRect mapFromParent(QRect rect){
        return parentTransform.mapRect(rect);
    }

    Q_INVOKABLE virtual bool isVisibleToSurface();
    Q_INVOKABLE inline virtual bool isSurface() const{return false;}
    Q_INVOKABLE virtual MoeGraphicsSurface* surface();

public slots:
    virtual void repaint(QRect =QRect());
    inline void repaint(QRectF rec) {repaint(QRect((int)qFloor(rec.x()),(int)qFloor(rec.y()),(int)qCeil(rec.width()),(int)qCeil(rec.height())));}
    inline void setPos(QPointF p) {setGeometry(QRectF(p, size()));}
    inline void setPos(qreal x, qreal y) {setPos(QPointF(x, y));}
    inline void setSize(QSizeF s) {setGeometry(QRectF(pos(), s));}
    inline void setSize(qreal w, qreal h) {setSize(QSizeF(w, h));}
    inline void setGeometry(qreal x, qreal y, qreal w, qreal h) {setGeometry(QRectF(x,y,w,h));}
    void setGeometry(QRectF);

protected slots:
    virtual void updateLayoutCaches();

signals:
    void paint(RenderRecorder*);
    void resized(QSizeF);
    void moved(QPointF);

    void mousePress(int);
    void mouseRelease(int);
    void mouseMoved(QPoint);
    void mouseScroll(QPoint);

    void keyTyped(char);
    void keyPressed(int);
    void keyReleased(int);

protected:
    friend class MoeGraphicsContainer;

    enum EventHook {
        mouseMoveHook,
        mousePressHook,
        mouseReleaseHook,
        mouseScrollHook
    };

    virtual inline bool requireHook(EventHook) {
        return false;
    }

    virtual inline void mousePressImpl(int) {}
    virtual inline void mouseReleaseImpl(int) {}
    virtual inline void mouseMoveImpl(QPoint) {}

    virtual inline void keyTypedImpl(char) {}
    virtual inline void keyReleaseImpl(int) {}
    virtual inline void keyPressImpl(int) {}

    void connectNotify(const QMetaMethod &);
    void disconnectNotify(const QMetaMethod &);

    bool usePixmapBuffer;

    QRgb _background;
    QRgb _foreground;
    QRgb _border;

    QTransform localTransform;
    QTransform parentTransform;
    QRect _realGeometry;
    QRect _localGeometry;
    QRectF _geometry;
    QPointF _scale;
    QPointF _rotate;
};

#endif // MOEGRAPHICSOBJECT_H
