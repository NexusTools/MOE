#ifndef MOEGRAPHICSIMAGE_H
#define MOEGRAPHICSIMAGE_H

#include "moegraphicsobject.h"

#include "moeresourcerequest.h"

class RenderBuffer;

class MoeGraphicsImage : public MoeGraphicsObject
{
    Q_OBJECT
public:
    explicit MoeGraphicsImage(QString path = QString(), MoeObject *parent = 0);

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
