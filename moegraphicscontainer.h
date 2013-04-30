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

    virtual void render(RenderRecorder*, QRect =QRect());
    Q_INVOKABLE void renderChildren(RenderRecorder*, QRect =QRect());

    Q_INVOKABLE inline void add(MoeGraphicsObject* obj) {
        if(obj->parent() != this) {
            obj->setParent(this);
            if(!children.contains(obj)) {
                children.prepend(obj);
                obj->updateLayoutCaches();
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
    virtual void updateLayoutCaches();

protected:
    friend class MoeGraphicsObject;

    virtual inline bool requireHook(EventHook event) {
        switch(event) {
        case mouseMoveHook:
            return !mouseMoveWatchers.isEmpty();
        case mousePressHook:
            return !mousePressWatchers.isEmpty();
        case mouseReleaseHook:
            return !mouseReleaseWatchers.isEmpty();
        case mouseScrollHook:
            return !mouseReleaseWatchers.isEmpty();
        }

        return false;
    }

    void updateWatcherCache(MoeGraphicsObject* obj);
    void updateChildCache();
    void markChildCacheDirty();

    inline void removeWatchers(MoeGraphicsObject* obj) {
        if(mouseMoveWatchers.contains(obj))
            mouseMoveWatchers.removeOne(obj);
        if(mousePressWatchers.contains(obj))
            mousePressWatchers.removeOne(obj);
        if(mouseReleaseWatchers.contains(obj))
            mouseReleaseWatchers.removeOne(obj);
        if(mouseScrollWatchers.contains(obj))
            mouseScrollWatchers.removeOne(obj);
    }

private:
    bool childCacheDirty;

    QList<MoeGraphicsObject*> mouseMoveWatchers;
    QList<MoeGraphicsObject*> mousePressWatchers;
    QList<MoeGraphicsObject*> mouseReleaseWatchers;
    QList<MoeGraphicsObject*> mouseScrollWatchers;

    QList<MoeGraphicsObject*> visibleChildren;
    QList<MoeGraphicsObject*> children;
};

#endif // MOEGRAPHICSCONTAINER_H
