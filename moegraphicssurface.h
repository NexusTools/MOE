#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moegraphicscontainer.h"
#include "renderrecorder.h"

#include <QSharedPointer>
#include <QWeakPointer>
#include <QScriptValue>
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
    void render(RenderRecorder* =0, QRect =QRect());

    Q_INVOKABLE inline bool isVisibleToSurface() {return true;}
    Q_INVOKABLE inline bool isSurface() const{return true;}
    Q_INVOKABLE MoeGraphicsSurface* surface() {return this;}

signals:
    void renderReady(RenderInstructions);

public slots:
    void updateSize(QSize);
    inline void prepareNextFrame(){
        if(renderState.testFlag(SurfaceDirty) && !renderState.testFlag(ViewReady))
            renderTimer.start();

        renderState |= ViewReady;
    }

    inline void repaint(QRect region =QRect()){
        if(region.isNull())
            region = _localGeometry;

        if(region.isEmpty())
            return;

        if(repaintDebug)
            repaintRegions.append(region);

        repaintRegion |= region;
        if(renderState.testFlag(ViewReady) && !renderState.testFlag(SurfaceDirty))
            renderTimer.start();

         renderState |= SurfaceDirty;
    }

    inline void enableRepaintDebug() {
        repaintDebug = true;
    }

    inline void disableRepaintDebug() {
        repaintDebug = false;
        repaintRegions.clear();
        repaint();
    }

protected slots:
    inline void renderNow(){
        render(0, repaintRegion);
        repaintRegion = QRect();
        renderState = NotReady;
    }

private:
    bool repaintDebug;
    QList<QRect> repaintRegions;
    QRect repaintRegion;
    RenderState renderState;
    QTimer renderTimer;
};

#endif // MOEGRAPHICSSURFACE_H
