#include "moeglspritesheet.h"
#include "gui/moegraphicsimage.h"

/*void MoeGLSpriteSheet::setSprite(QPoint p, MoeGraphicsImage *img) {
    blitBufferRequests.insert(p, img->bufferID());
    dataBufferRequests.remove(p);
}*/

void MoeGLSpriteSheet::setSpriteData(QPoint p, QByteArray data) {
    dataBufferRequests.insert(p, data);
    blitBufferRequests.remove(p);
}
