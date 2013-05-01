#include "moegraphicssurface.h"
#include "renderrecorder.h"

MoeGraphicsSurface::MoeGraphicsSurface(QSize size)
{
    _background = qRgb(0, 0, 0);
    _foreground = qRgba(0, 0, 0, 0);
    _border = qRgba(0, 0, 0, 0);
    repaintDebug = RepaintDebugOff;
    qRegisterMetaType<RenderInstructions>("RenderInstructions");

    renderTimer.setInterval(0);
    renderTimer.setSingleShot(true);
    renderTimer.moveToThread(thread());
    connect(&renderTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
    updateSize(size);
}

void MoeGraphicsSurface::render(RenderRecorder* p, QRect region)
{
    bool renderForParent = p != NULL;
    if(!renderForParent)
        p = new RenderRecorder(repaintDebug == RepaintDebugFrame ? _localGeometry : region);

    MoeGraphicsContainer::render(p, repaintDebug == RepaintDebugFrame ? _localGeometry : region);
    if(repaintDebug) {
        foreach(QRect rect, repaintRegions) {
            p->fillRect(rect, qRgba(0, 250, 0, 60));
        }
        repaintRegions.clear();
    }
    if(!renderForParent) {
        emit renderReady(p->instructions(), repaintDebug == RepaintDebugFrame ? _localGeometry : region, _localGeometry.size());
        p->deleteLater();
    }
}

void MoeGraphicsSurface::updateSize(QSize size)
{
    setGeometry(QRect(QPoint(0, 0), size));
}
