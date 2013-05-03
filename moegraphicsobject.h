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
    Q_PROPERTY(Qt::CursorShape cursor READ cursor WRITE setCursor)
    Q_PROPERTY(QRgb background READ background WRITE setBackground)
    Q_PROPERTY(QRgb foreground READ foreground WRITE setForeground)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QRgb border READ border WRITE setBorder)

    friend class MoeGraphicsContainer;
    friend class MoeGraphicsSurface;
public:
    Q_INVOKABLE MoeGraphicsObject(MoeGraphicsContainer* parent =0) : _scale(1, 1) {
        _background = qRgba(0, 0, 0, 0);
        _foreground = qRgb(0, 0, 0);
        _border = qRgba(0, 0, 0, 0);
        _opacity = 1;
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
    Q_INVOKABLE void setOpacity(qreal opacity);

    inline qreal opacity() const{
        return _opacity;
    }

    Q_INVOKABLE inline void show() {
        setOpacity(1);
    }

    Q_INVOKABLE inline void hide() {
        setOpacity(0);
    }

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

    Q_INVOKABLE virtual bool isVisible();
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

    void keyFocusGained();
    void keyFocusLost();

    void keyTyped(char);
    void keyPressed(int);
    void keyReleased(int);

protected:
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

    bool canUseKeyFocus();
    bool canUseMouseFocus();
    void updateHoverFocus();
    void notifyParentOfUpdate();
    bool isHookConnected(EventHook);

    virtual inline bool isHookRequired(EventHook) {
        return false;
    }

    void connectNotify(const QMetaMethod &) {
        notifyParentOfUpdate();
    }

    void disconnectNotify(const QMetaMethod &) {
        notifyParentOfUpdate();
    }

    inline QSize localSize() const{
        return _localGeometry.size();
    }

    inline QRect localGeometry() const{
        return _localGeometry;
    }

    Qt::CursorShape _cursor;
    QRgb _background;
    QRgb _foreground;
    QRgb _border;
    QPointF _scale;
    QPointF _rotate;
    qreal _opacity;

private:
    inline void mouseEnterEvent(){
        emit mouseEntered();
    }
    inline void mouseLeaveEvent(){
        emit mouseLeft();
    }

    inline void mousePressedEvent(QPoint p, int i) {
        emit mousePressed(p, i);
    }
    inline void mouseReleasedEvent(QPoint p, int i) {
        emit mouseReleased(p, i);
    }
    virtual inline void mouseMovedEvent(QPoint p) {
        updateHoverFocus();
        emit mouseMoved(p);
    }
    inline void mouseDraggedEvent(QPoint p) {
        emit mouseDragged(p);
    }
    inline void mouseScrolledEvent(QPoint s){
        emit mouseScrolled(s);
    }

    inline void keyTypedEvent(char c) {
        emit keyTyped(c);
    }
    inline void keyReleasedEvent(int k) {
        emit keyReleased(k);
    }
    inline void keyPressedEvent(int k) {
        emit keyPressed(k);
    }

    QRectF _geometry;
    QRect _realGeometry;
    QRect _localGeometry;

    QTransform localTransform;
    QTransform parentTransform;
    QTransform surfaceTransform;
};

#endif // MOEGRAPHICSOBJECT_H
