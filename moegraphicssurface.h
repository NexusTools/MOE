#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moegraphicscontainer.h"
#include "renderrecorder.h"

#include <QThreadStorage>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QScriptValue>
#include <QPointer>
#include <QDebug>
#include <QTimer>

class MoeEngineView;
class MoeGraphicsSurface;

typedef QPointer<MoeGraphicsSurface> MoeGraphicsSurfacePointer;

class MoeGraphicsSurface : public MoeGraphicsContainer
{
    Q_OBJECT

    friend class MoeGraphicsObject;
public:
    enum RenderStateFlag {
        NotReady = 0x0,
        SurfaceDirty = 0x1,
        ViewReady = 0x2
    };

    Q_DECLARE_FLAGS(RenderState, RenderStateFlag)

    MoeGraphicsSurface(QSize size);
    void render(RenderRecorder*, QRect);

    Q_INVOKABLE inline bool isVisibleToSurface() {return true;}
    Q_INVOKABLE inline bool isSurface() const{return true;}
    Q_INVOKABLE inline MoeGraphicsSurface* surface() {return this;}
    Q_INVOKABLE inline void setTitle(QString title) {emit titleChanged(title);}

signals:
    void renderReady(RenderInstructions, QRect, QSize);
    void cursorChanged(QCursor);
    void titleChanged(QString);

public slots:
    inline void repaint(QRect region =QRect()){
        if(region.isNull() || qAlpha(_background) < 255)
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

    inline void updateSize(QSize size){
        setGeometry(QRect(QPoint(0, 0), size));
    }

    inline void prepareNextFrame(){
        if(renderState.testFlag(SurfaceDirty) && !renderState.testFlag(ViewReady))
            renderTimer.start();

        renderState |= ViewReady;
    }

    inline void mouseMove(QPoint p){
        makeCurrent();

        if(!mouseDragFocus.isNull())
            mouseDragFocus.data()->mouseDraggedEvent(mouseDragFocus.data()->mapFromSurface(p));
        else {
            MoeGraphicsObject* oldHoverFocus = mouseHoverFocus.data();
            MoeGraphicsContainer::mouseMovedEvent(p);
            if(mouseHoverFocus.data() != oldHoverFocus) {
                if(oldHoverFocus)
                    oldHoverFocus->mouseLeaveEvent();
                if(!mouseHoverFocus.isNull()) {
                    mouseHoverFocus.data()->mouseEnterEvent();
                    updateCursor(mouseHoverFocus.data()->cursor());
                }
            }
        }
    }

    inline void mousePress(QPoint p, int b){
        if(!mouseDragFocus.isNull())
            mouseDragFocus.data()->mousePressedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
        else if(!mouseHoverFocus.isNull())
            mouseHoverFocus.data()->mousePressedEvent(mouseHoverFocus.data()->mapFromSurface(p), b);
        else {
            MoeGraphicsContainer::mousePressedEvent(p, b);
            if(mouseHoverFocus.data() != mouseDragFocus.data()) {
                qDebug() << "Started dragging" << mouseHoverFocus.data();
                mouseDragFocus = mouseHoverFocus;
            }
        }
    }

    inline void mouseRelease(QPoint p, int b){
        if(mouseDragFocus.data()) {
            mouseDragFocus.data()->mouseReleasedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
            if(!b) // All buttons released
                mouseDragFocus = 0;
        } else
            MoeGraphicsContainer::mouseReleasedEvent(p, b);
    }

private slots:
    inline void renderNow(){
        renderState = NotReady;
        render(0, repaintRegion);
        repaintRegion = QRect();
    }

protected:
    inline void updateHoverTarget(MoeGraphicsObject* obj){
        qDebug() << mouseHoverFocus.data() << obj;
        mouseHoverFocus = obj;
    }

    inline bool isHoverTarget(MoeGraphicsObject* obj) {
        return mouseHoverFocus.data() == obj;
    }

    inline void updateCursor(QCursor cur) {
        if(activeCursor.shape() == cur.shape())
            return;

        emit cursorChanged(cur);
        activeCursor = cur;
    }

private:
    inline void makeCurrent() {
        _currentSurface.setLocalData(MoeGraphicsSurfacePointer(this));
    }

    static inline MoeGraphicsSurface* current() {
        return _currentSurface.localData().data();
    }

    enum RepaintDebugMode {
        RepaintDebugOff,
        RepaintDebugArea,
        RepaintDebugFrame
    };

    QPointer<MoeGraphicsObject> keyboardFocus;
    QPointer<MoeGraphicsObject> mouseHoverFocus;
    QPointer<MoeGraphicsObject> mouseDragFocus;

    static QThreadStorage<MoeGraphicsSurfacePointer> _currentSurface;
    RepaintDebugMode repaintDebug;
    QList<QRect> repaintRegions;
    RenderState renderState;
    QCursor activeCursor;
    QRect repaintRegion;
    QTimer renderTimer;
};

#endif // MOEGRAPHICSSURFACE_H
