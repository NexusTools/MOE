#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moegraphicscontainer.h"
#include "renderrecorder.h"

#include <QSharedPointer>
#include <QWeakPointer>
#include <QScriptValue>
#include <QPointer>
#include <QTimer>

class MoeEngineView;
class MoeEngineGraphicsObject;

class MoeGraphicsSurface : public MoeGraphicsContainer
{
    Q_OBJECT
public:
    enum RenderStateFlag {
        NotReady = 0x0,
        SurfaceDirty = 0x1,
        ViewReady = 0x2
    };
    Q_DECLARE_FLAGS(RenderState, RenderStateFlag)

    MoeGraphicsSurface();
    void render(RenderRecorder*, QRect);

    Q_INVOKABLE inline bool isVisibleToSurface() {return true;}
    Q_INVOKABLE inline bool isSurface() const{return true;}
    Q_INVOKABLE inline MoeGraphicsSurface* surface() {return this;}
    Q_INVOKABLE inline void setTitle(QString title) {emit titleChanged(title);}

signals:
    void renderReady(RenderInstructions, QRect);
    void titleChanged(QString);

public slots:
    inline void repaint(QRect region =QRect()){
        if(region.isNull())
            region = _localGeometry;
        else
            region &= _localGeometry;

        if(region.isEmpty())
            return;

        if(repaintDebug)
            repaintRegions.append(region);

        if(repaintRegion.isEmpty())
            repaintRegion = region;
        else
            repaintRegion |= region;

        if(renderState.testFlag(ViewReady) && !renderState.testFlag(SurfaceDirty))
            renderTimer.start();

         renderState |= SurfaceDirty;
    }

    inline void enableRepaintDebug(bool fullFrame =true) {
        repaintDebug = fullFrame ? RepaintDebugFrame : RepaintDebugArea;
    }

    inline void disableRepaintDebug() {
        repaintDebug = RepaintDebugOff;
        repaintRegions.clear();
        repaint();
    }

protected slots:
    friend class MoeEngineView;

    void updateSize(QSize);

    inline void prepareNextFrame(){
        if(renderState.testFlag(SurfaceDirty) && !renderState.testFlag(ViewReady))
            renderTimer.start();

        renderState |= ViewReady;
    }

    inline void mouseMove(QPoint p){
        if(mouseDragFocus.data())
            mouseDragFocus.data()->mouseDraggedImpl(mouseDragFocus.data()->mapFromSurface(p));
        else {
            MoeGraphicsObject* oldHoverFocus = mouseHoverFocus.data();
            MoeGraphicsContainer::mouseMovedImpl(p);
            if(mouseHoverFocus.data() != oldHoverFocus) {
                oldHoverFocus->mouseLeaveImpl();
                mouseHoverFocus.data()->mouseEnterImpl();
            }
        }
    }

    inline void mousePress(QPoint p, int b){
        if(mouseDragFocus.data()) {
            mouseDragFocus.data()->mouseReleasedImpl(mouseDragFocus.data()->mapFromSurface(p), b);
            mouseDragFocus = 0;
        } else
            MoeGraphicsContainer::mousePressedImpl(p, b);
    }

    inline void mouseRelease(QPoint p, int b){
        if(mouseDragFocus.data()) {
            mouseDragFocus.data()->mouseReleasedImpl(mouseDragFocus.data()->mapFromSurface(p), b);
            if(!b) // All buttons released
                mouseDragFocus = 0;
        } else
            MoeGraphicsContainer::mouseReleasedImpl(p, b);
    }

private slots:
    inline void renderNow(){
        renderState = NotReady;
        render(0, repaintRegion);
        repaintRegion = QRect();
        renderState = NotReady;
    }

protected:
    friend class MoeGraphicsObject;

    inline void giveKeyFocus(MoeGraphicsObject* obj){
        keyboardFocus = obj;
    }
    inline void giveMouseDragFocus(MoeGraphicsObject* obj){
        mouseDragFocus = obj;
    }
    inline void giveMouseHoverFocus(MoeGraphicsObject* obj){
        mouseHoverFocus = obj;
    }

    bool hasHoverFocus(MoeGraphicsObject* obj) {
        return mouseHoverFocus.data() == obj;
    }

private:
    enum RepaintDebugMode {
        RepaintDebugOff,
        RepaintDebugArea,
        RepaintDebugFrame
    };

    QPointer<MoeGraphicsObject> keyboardFocus;
    QPointer<MoeGraphicsObject> mouseHoverFocus;
    QPointer<MoeGraphicsObject> mouseDragFocus;

    RepaintDebugMode repaintDebug;
    QList<QRect> repaintRegions;
    QRect repaintRegion;
    RenderState renderState;
    QTimer renderTimer;
};

#endif // MOEGRAPHICSSURFACE_H
