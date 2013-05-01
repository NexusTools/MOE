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
        _background = qRgba(0, 0, 0, 0);
        _foreground = qRgb(0, 0, 0);
        _border = qRgba(0, 0, 0, 0);
        setContainer(parent);
    }

    bool event(QEvent *);
    virtual void render(RenderRecorder*, QRect);
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

    inline QPoint mapToParent(QPoint p){
        return localTransform.map(p);
    }

    Q_INVOKABLE inline QRect mapToParent(QRect rect){
        return localTransform.mapRect(rect);
    }

    inline QPoint mapFromParent(QPoint p){
        return parentTransform.map(p);
    }

    Q_INVOKABLE inline QRect mapFromParent(QRect rect){
        return parentTransform.mapRect(rect);
    }

    QPoint mapFromSurface(QPoint rect);
    Q_INVOKABLE QRect mapFromSurface(QRect rect);

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
    virtual void updateLayoutTransform();

signals:
    void paint(RenderRecorder*);
    void resized(QSizeF);
    void moved(QPointF);

    void mousePressed(QPoint, int);
    void mouseReleased(QPoint, int);
    void mouseMoved(QPoint);
    void mouseDragged(QPoint);
    void mouseScrolled(QPoint);
    void mouseEntered();
    void mouseLeft();

    void keyTyped(char);
    void keyPressed(int);
    void keyReleased(int);

protected:
    friend class MoeGraphicsContainer;
    friend class MoeGraphicsSurface;

    enum EventHook {
        mouseMovedHook,
        mouseDraggedHook,
        mousePressedHook,
        mouseReleasedHook,
        mouseScrolledHook,

        keyTypedHook,
        keyPressedHook,
        keyReleasedHook
    };

    bool usesKeyboardEvents();
    bool usesDragEvent();

    void takeKeyFocus();
    void takeMouseFocus();
    void takeHoverFocus();

    virtual inline bool requireHook(EventHook) {
        return false;
    }

    virtual inline void mouseEnterImpl(){
        emit mouseEntered();
    }

    virtual inline void mouseLeaveImpl(){
        emit mouseLeft();
    }

    virtual inline void mousePressedImpl(QPoint p, int i) {
        if(usesKeyboardEvents())
            takeKeyFocus();
        if(usesDragEvent())
            takeMouseFocus();
        emit mousePressed(p, i);
    }
    virtual inline void mouseReleasedImpl(QPoint p, int i) {
        emit mouseReleased(p, i);
    }
    virtual inline void mouseMovedImpl(QPoint p) {
        takeHoverFocus();
        emit mouseMoved(p);
    }
    virtual inline void mouseDraggedImpl(QPoint p) {
        emit mouseDragged(p);
    }
    virtual inline void mouseScrolledImpl(QPoint s){
        emit mouseScrolled(s);
    }

    virtual inline void keyTypedImpl(char c) {
        emit keyTyped(c);
    }
    virtual inline void keyReleasedImpl(int k) {
        emit keyReleased(k);
    }
    virtual inline void keyPressedImpl(int k) {
        emit keyPressed(k);
    }

    void connectNotify(const QMetaMethod &);
    void disconnectNotify(const QMetaMethod &);

    bool usePixmapBuffer;

    QRgb _background;
    QRgb _foreground;
    QRgb _border;

    QTransform localTransform;
    QTransform parentTransform;
    QTransform surfaceTransform;
    QRect _realGeometry;
    QRect _localGeometry;
    QRectF _geometry;
    QPointF _scale;
    QPointF _rotate;
};

#endif // MOEGRAPHICSOBJECT_H
