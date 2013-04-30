#include "moegraphicssurface.h"
#include "renderrecorder.h"

MoeGraphicsSurface::MoeGraphicsSurface()
{
    _background = qRgb(0, 0, 0);
    qRegisterMetaType<RenderInstructions>("RenderInstructions");

    renderTimer.setInterval(0);
    renderTimer.setSingleShot(true);
    renderTimer.moveToThread(thread());
    connect(&renderTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
}

void MoeGraphicsSurface::render(RenderRecorder* p, QRect region)
{
    if(region.isNull())
        region = _localGeometry;
    if(!p)
        p = new RenderRecorder(_localGeometry);
    MoeGraphicsContainer::render(p, region);
    if(repaintDebug) {
        foreach(QRect rect, repaintRegions) {
            p->fillRect(rect, qRgba(0, 250, 0, 60));
        }
        repaintRegions.clear();
        p->fillRect(region, qRgba(0, 250, 0, 60));
    }
    emit renderReady(p->instructions());
    p->deleteLater();
}

void MoeGraphicsSurface::updateSize(QSize size)
{
    qDebug() << "Updating Size" << size;
    setGeometry(QRect(QPoint(0, 0), size));
}
