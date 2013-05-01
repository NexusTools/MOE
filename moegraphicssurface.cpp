#include "moegraphicssurface.h"
#include "renderrecorder.h"

MoeGraphicsSurface::MoeGraphicsSurface()
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
}

void MoeGraphicsSurface::render(RenderRecorder* p, QRect region)
{
    if(!p)
        p = new RenderRecorder(repaintDebug == RepaintDebugFrame ? _localGeometry : region);

    MoeGraphicsContainer::render(p, repaintDebug == RepaintDebugFrame ? _localGeometry : region);
    if(repaintDebug) {
        foreach(QRect rect, repaintRegions) {
            p->fillRect(rect, qRgba(0, 250, 0, 60));
        }
        repaintRegions.clear();
    }
    emit renderReady(p->instructions(), repaintDebug == RepaintDebugFrame ? _localGeometry : region);
    p->deleteLater();
}

void MoeGraphicsSurface::updateSize(QSize size)
{
    setGeometry(QRect(QPoint(0, 0), size));
}
