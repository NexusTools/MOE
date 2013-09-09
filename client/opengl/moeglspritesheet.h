#ifndef MOEGLSPRITESHEET_H
#define MOEGLSPRITESHEET_H

#include <core/moeobject.h>

#include <QHash>
#include <QPoint>
#include <QSize>

class MoeGraphicsImage;
class RenderBuffer;

inline uint qHash(const QPoint & r)
{
    return qHash(r.x()) + qHash(r.y());
}

class MoeGLSpriteSheet : public MoeObject
{
    Q_OBJECT

    friend class MoeGLVertexModel;
public:
    Q_INVOKABLE inline explicit MoeGLSpriteSheet(QSize spriteSize, QSize sheetSize = QSize(16, 16)) {this->spriteSize = spriteSize; this->sheetSize = sheetSize; needsUpdate = true; created = false;}

    //Q_INVOKABLE void setSprite(QPoint p, MoeGraphicsImage* img);
    Q_INVOKABLE void setSpriteData(QPoint p, QByteArray data);

    inline QSize size() const{return QSize(sheetSize.width()*spriteSize.width(),sheetSize.height()*spriteSize.height());}

private:
    typedef QHash<QPoint, quintptr> HashPTR;
    typedef QHash<QPoint, QByteArray> HashData;

    HashPTR blitBufferRequests;
    HashData dataBufferRequests;

    bool created;
    bool needsUpdate;
    QSize spriteSize;
    QSize sheetSize;
    
};

#endif // MOEGLSPRITESHEET_H
