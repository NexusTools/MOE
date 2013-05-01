#ifndef MOEGRAPHICSCONTAINER_H
#define MOEGRAPHICSCONTAINER_H

#include "moegraphicsobject.h"

class MoeGraphicsContainer : public MoeGraphicsObject
{
    Q_OBJECT
public:
    Q_INVOKABLE MoeGraphicsContainer(MoeGraphicsContainer* parent =0) {
        setContainer(parent);
    }

    virtual void render(RenderRecorder*, QRect);
    Q_INVOKABLE void renderChildren(RenderRecorder*, QRect);

    Q_INVOKABLE inline void add(MoeGraphicsObject* obj) {
        if(obj->parent() != this) {
            obj->setParent(this);
            if(!children.contains(obj)) {
                children.prepend(obj);
                obj->updateLayoutTransform();
                obj->repaint();
            }
        }
    }

    Q_INVOKABLE inline void remove(MoeGraphicsObject* obj) {
        if(obj->parent() == this) {
            obj->setParent(NULL);
             if(children.contains(obj)) {
                children.removeOne(obj);

                if(visibleChildren.contains(obj)) {
                    visibleChildren.removeOne(obj);
                    repaint(obj->realGeometry());
                }

                removeWatchers(obj);
            }
        }
    }

    Q_INVOKABLE inline bool childVisible(MoeGraphicsObject* child) {
        return visibleChildren.contains(child);
    }

protected slots:
    virtual void updateLayoutTransform();

protected:
    friend class MoeGraphicsObject;
    friend class MoeGraphicsSurface;

    virtual inline bool requireHook(EventHook event) {
        switch(event) {
        case mouseMovedHook:
            return !mouseMovedWatchers.isEmpty();

        case mousePressedHook:
            return !mousePressedWatchers.isEmpty();

        case mouseReleasedHook:
            return !mouseReleasedWatchers.isEmpty();

        case mouseScrolledHook:
            return !mouseScrolledWatchers.isEmpty();

        default:
            return false;
        }

    }

    inline bool hasKeyoardEvent(MoeGraphicsObject* obj){
        return keyTypedWatchers.contains(obj)
                && keyPressedWatchers.contains(obj)
                && keyReleasedWatchers.contains(obj);
    }

    inline bool hasDragEvent(MoeGraphicsObject* obj) {
        return mouseDraggedWatchers.contains(obj);
    }

    void mouseMovedImpl(QPoint p);

    void updateWatcherCache(MoeGraphicsObject* obj);
    void updateChildCache();
    void markChildCacheDirty();

    inline void removeWatchers(MoeGraphicsObject* obj) {
        mouseMovedWatchers.removeOne(obj);
        mouseDraggedWatchers.removeOne(obj);
        mousePressedWatchers.removeOne(obj);
        mouseReleasedWatchers.removeOne(obj);
        mouseScrolledWatchers.removeOne(obj);

        keyTypedWatchers.removeOne(obj);
        keyPressedWatchers.removeOne(obj);
        keyReleasedWatchers.removeOne(obj);
    }

private:
    bool childCacheDirty;

    QList<MoeGraphicsObject*> mouseMovedWatchers;
    QList<MoeGraphicsObject*> mouseDraggedWatchers;
    QList<MoeGraphicsObject*> mousePressedWatchers;
    QList<MoeGraphicsObject*> mouseReleasedWatchers;
    QList<MoeGraphicsObject*> mouseScrolledWatchers;

    QList<MoeGraphicsObject*> keyTypedWatchers;
    QList<MoeGraphicsObject*> keyPressedWatchers;
    QList<MoeGraphicsObject*> keyReleasedWatchers;

    QList<MoeGraphicsObject*> visibleChildren;
    QList<MoeGraphicsObject*> children;
};

#endif // MOEGRAPHICSCONTAINER_H
