#include "moegraphicsimage.h"
#include "renderbuffer.h"

MoeGraphicsImage::MoeGraphicsImage(QString, MoeObject *parent) :
    MoeGraphicsObject(parent)
{
    renderBuffer = RenderBuffer::instance(this);
}

void MoeGraphicsImage::load(QByteArray data) {
    renderBuffer->load(data);
}

void MoeGraphicsImage::requestPixelData(QRect) {}
