#ifndef MOEGRAPHICSOBJECT_H
#define MOEGRAPHICSOBJECT_H

#include <core/moeobject.h>

#include <QMetaMethod>
#include <QTransform>
#include <QCursor>
#include <qmath.h>

#include <QRectF>
#include <QColor>
#include <QBrush>

class RenderRecorder;
class MoeGraphicsObject;
class MoeGraphicsContainer;
class MoeAbstractGraphicsSurface;

typedef QPointer<MoeGraphicsObject> MoeGraphicsObjectPointer;

class MoeGraphicsObject : public MoeObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal posX READ posX WRITE setPosX)
    Q_PROPERTY(qreal posY READ posY WRITE setPosY)
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    Q_PROPERTY(qreal width READ width WRITE setWidth)
    Q_PROPERTY(qreal height READ height WRITE setHeight)
    Q_PROPERTY(qreal borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QColor border READ border WRITE setBorder)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

    friend class MoeGraphicsContainer;
    friend class MoeAbstractGraphicsSurface;
public:
    Q_INVOKABLE explicit inline MoeGraphicsObject(MoeObject* parent =0) : MoeObject(parent), _scale(1, 1) {
        if(container())
            setContainer(container());

        _disabled = false;
        _foreground = Qt::black;
        _background = Qt::NoBrush;
        _border = Qt::transparent;
        _borderRadius = 0;
        _opacity = 1;
    }

    virtual bool event(QEvent *);
    virtual void render(RenderRecorder*, QRect);
    Q_INVOKABLE virtual void paintImpl(RenderRecorder*, QRect);

    inline QCursor cursor() const{return _cursor;}
    void setCursor(QCursor);

    inline qreal scale() const{
        return _scale.x() > _scale.y() ? _scale.y() + (_scale.x() - _scale.y())/2
                                       : _scale.x() + (_scale.y() - _scale.x())/2;
    }

    Q_INVOKABLE void setScale(qreal s) {
        repaint();
        _scale = QPointF(s, s);
        updateLayoutTransform();
        repaint();
    }

    inline qreal borderRadius() const{return _borderRadius;}
    Q_INVOKABLE void setBorderRadius(qreal borderRadius) {
        if(_borderRadius==borderRadius)return;
        _borderRadius = borderRadius;repaint();}

    inline qreal posX() const{return _geometry.x();}
    inline qreal posY() const{return _geometry.y();}
    inline QPointF pos() const{return _geometry.topLeft();}

    inline QSizeF size() const{return _geometry.size();}
    inline qreal width() const{return _geometry.width();}
    inline qreal height() const{return _geometry.height();}

    Q_INVOKABLE void setDisabled(bool d);
    Q_INVOKABLE inline bool isDisabled() {return _disabled;}
    Q_INVOKABLE inline QRectF geomtry() const{return _geometry;}
    Q_INVOKABLE inline QRect realGeometry() const{return _realGeometry;}

    inline QBrush background() const{return _background;}
    inline QColor foreground() const{return _foreground;}
    inline QColor border() const{return _border;}
    Q_INVOKABLE inline void setBackground(QBrush c) {if(c==_background)return;_background=c;repaint();}
    Q_INVOKABLE inline void setForeground(QColor c) {if(c==_foreground)return;_foreground=c;repaint();}
    Q_INVOKABLE inline void setBorder(QColor c) {if(c==_border)return;_border=c;repaint();}

    Q_INVOKABLE inline bool contains(QPointF p) const{return _geometry.contains(p);}
    Q_INVOKABLE inline bool contains(QRectF r) const{return _geometry.contains(r);}

    Q_INVOKABLE void setContainer(MoeGraphicsContainer* contain);
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
    Q_INVOKABLE virtual MoeAbstractGraphicsSurface* surface();

public slots:
    inline void repaint() {repaint(localGeometry());}
    inline void repaint(QRectF rec) {repaint(QRect((int)qFloor(rec.x()),(int)qFloor(rec.y()),(int)qCeil(rec.width()),(int)qCeil(rec.height())));}
    inline void setPos(QPointF p) {setGeometry(QRectF(p, size()));}
    inline void setPos(qreal x, qreal y) {setPos(QPointF(x, y));}
    inline void setPosX(qreal x) {setGeometry(QRectF(QPointF(x,posY()),size()));}
    inline void setPosY(qreal y) {setGeometry(QRectF(QPointF(posX(),y),size()));}
    inline void setSize(QSizeF s) {setGeometry(QRectF(pos(), s));}
    inline void setSize(qreal w, qreal h) {setSize(QSizeF(w, h));}
    inline void setWidth(qreal w) {setGeometry(QRectF(pos(),QSizeF(w,height())));}
    inline void setHeight(qreal h) {setGeometry(QRectF(pos(),QSizeF(width(),h)));}
    inline void setGeometry(qreal x, qreal y, qreal w, qreal h) {setGeometry(QRectF(x,y,w,h));}
    void setGeometry(QRectF);

protected slots:
    virtual void updateLayoutTransform();

signals:
    void cursorChanged(QCursor);
    void paint(RenderRecorder*);
    void resized(QSizeF);
    void moved(QPointF);

    void mousePressed(QPoint, int button, int buttons);
    void mouseReleased(QPoint, int button, int buttons);
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
    virtual void repaint(QRect);

    enum EventHook {
        mouseMovedHook,
        mouseDraggedHook,
        mousePressedHook,
        mouseReleasedHook,
        mouseScrolledHook,
        mouseEnteredHook,
        mouseLeftHook,

        keyTypedHook,
        keyPressedHook,
        keyReleasedHook
    };

    bool canUseKeyFocus();
    bool canUseMouseFocus();
    void notifyParentOfUpdate();
    bool isHookConnected(EventHook);

    virtual inline bool isHookRequired(EventHook) {
        return false;
    }

    void connectNotify(const QMetaMethod& meta) {
        if(meta.methodSignature().startsWith("mouse") ||
                meta.methodSignature().startsWith("key"))
            notifyParentOfUpdate();
    }

    void disconnectNotify(const QMetaMethod& meta) {
        if(meta.methodSignature().startsWith("mouse") ||
                meta.methodSignature().startsWith("key"))
            notifyParentOfUpdate();
    }

    inline QSize localSize() const{
        return _localGeometry.size();
    }

    inline QRect localGeometry() const{
        return _localGeometry;
    }

    bool _disabled;
    QCursor _cursor;
    QBrush _background;
    QColor _foreground;
    QColor _border;
    QPointF _scale;
    QPointF _rotate;
    qreal _borderRadius;
    qreal _opacity;

private:
    inline void mouseEnterEvent(){
        emit mouseEntered();
    }
    inline virtual void mouseLeaveEvent(){
        emit mouseLeft();
    }

    inline void mousePressedEvent(QPoint p, int b, int bs) {
        emit mousePressed(p, b, bs);
    }
    inline void mouseReleasedEvent(QPoint p, int b, int bs) {
        emit mouseReleased(p, b, bs);
    }
    virtual inline MoeGraphicsObjectPointer mouseMovedEvent(QPoint p) {
        emit mouseMoved(p);
        return MoeGraphicsObjectPointer(this);
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
