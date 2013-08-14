#include "moegraphicsimage.h"
#include "renderrecorder.h"
#include "renderbuffer.h"

MoeGraphicsImage::MoeGraphicsImage(MoeObject *parent) :
    MoeGraphicsObject(parent)
{
    renderBuffer = 0;
}

void MoeGraphicsImage::paintImpl(RenderRecorder* p, QRect) {
    if(!renderBuffer)
        return;
    p->drawBuffer(renderBuffer, localGeometry());
}

void MoeGraphicsImage::load(QByteArray data) {
    if(!renderBuffer)
        renderBuffer = RenderBuffer::instance(this);
    renderBuffer->load(data);
    repaint();
}

void MoeGraphicsImage::requestPixelData(QRect) {}
