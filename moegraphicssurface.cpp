#include "moegraphicssurface.h"
#include "renderrecorder.h"

MoeGraphicsSurface::MoeGraphicsSurface()
{
    paintTimer.setInterval(0);
    paintTimer.setSingleShot(true);
    connect(&paintTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
    qRegisterMetaType<RenderInstructions>("RenderInstructions");
}

void MoeGraphicsSurface::render(RenderRecorder*, QRect region)
{
    RenderRecorder* p = new RenderRecorder();
    MoeGraphicsObject::render(p, region);
    emit renderReady(p->instructions());
    p->deleteLater();
}

void MoeGraphicsSurface::renderNow()
{
    render();
}

void MoeGraphicsSurface::updateSize(QSize size)
{
    setGeometry(QRect(QPoint(0, 0), size));
}

void MoeGraphicsSurface::repaintImpl()
{
    paintTimer.start();
}
