#ifndef MOEGRAPHICSCONTAINER_H
#define MOEGRAPHICSCONTAINER_H

#include "moegraphicsobject.h"


typedef QPointer<MoeGraphicsContainer> MoeGraphicsContainerPointer;

class MoeGraphicsContainer : public MoeGraphicsObject
{
    Q_OBJECT

    friend class MoeGraphicsObject;
public:
    Q_INVOKABLE MoeGraphicsContainer(MoeObject* parent =0) : MoeGraphicsObject(parent) {}

    virtual void render(RenderRecorder*, QRect);
    Q_INVOKABLE void renderChildren(RenderRecorder*, QRect);

    Q_INVOKABLE inline void add(MoeGraphicsObject* obj) {
        if(obj->parent() != this)
            obj->setParent(this);

        if(!children.contains(obj)) {
            children.append(obj);
            obj->updateLayoutTransform();
            obj->repaint();
        }
    }

    Q_INVOKABLE inline void remove(MoeGraphicsObject* obj) {
        if(obj->parent() == this)
            obj->setParent(NULL);

        if(children.contains(obj)) {
            children.removeOne(obj);

            if(visibleChildren.contains(obj)) {
                mouseMovedWatchers.removeOne(obj);
                visibleChildren.removeOne(obj);
                repaint(obj->realGeometry());
            }
        }
    }

    Q_INVOKABLE inline bool childVisible(MoeGraphicsObject* child) {
        return visibleChildren.contains(child);
    }

protected slots:
    virtual void updateLayoutTransform();

protected:
    friend class MoeAbstractGraphicsSurface;

    virtual inline bool isHookRequired(EventHook event) {
        switch(event) {
        case mouseMovedHook:
            return !mouseMovedWatchers.isEmpty();

        default:
            return false;
        }
    }

private:
    void updateChildCache();
    void markChildCacheDirty();
    MoeGraphicsObject* mouseMovedEvent(QPoint p);

    bool childCacheDirty;
    QList<MoeGraphicsObject*> mouseMovedWatchers;
    QList<MoeGraphicsObject*> visibleChildren;
    QList<MoeGraphicsObject*> children;

    MoeGraphicsObjectPointer _hoverFocus;

};

#endif // MOEGRAPHICSCONTAINER_H
