#ifndef MOEGRAPHICSIMAGE_H
#define MOEGRAPHICSIMAGE_H

#include "moegraphicsobject.h"

#include <network/moeresourcerequest.h>

class RenderBuffer;

class MoeGraphicsImage : public MoeGraphicsObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGraphicsImage(MoeObject *parent = 0);
    void paintImpl(RenderRecorder*, QRect);

    QRgb mainColor();

signals:
    void loaded(QSize);
    void failed(QString);

public slots:
    void load(QByteArray data);
    void requestPixelData(QRect);

protected slots:

private:
    RenderBuffer* renderBuffer;
    QByteArray data;
};

#endif // MOEGRAPHICSIMAGE_H
