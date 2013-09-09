#ifndef MOEGLGRAPHICSVIEW_H
#define MOEGLGRAPHICSVIEW_H

#include <gui/moegraphicsobject.h>

#include "gltypes.h"

class MoeGLScene;
class RenderBuffer;

class MoeGLGraphicsView : public MoeGraphicsObject
{

    Q_OBJECT
    Q_PROPERTY(qreal camX READ camX WRITE setCamX)
    Q_PROPERTY(qreal camY READ camY WRITE setCamY)
    Q_PROPERTY(qreal camZ READ camZ WRITE setCamZ)
    Q_PROPERTY(qreal camPOV READ camPOV WRITE setCamPOV)
public:
    Q_INVOKABLE explicit MoeGLGraphicsView(MoeObject* parent =0)
        : MoeGraphicsObject(parent) {renderBuffer = 0;needsUpdate=true;}
    virtual void render(RenderRecorder*, QRect);

    Q_INVOKABLE inline void camTranslate(qreal x, qreal y, qreal z) {
        matrix.position.x += x;
        matrix.position.y += y;
        matrix.position.z += z;
        matrix.needsUpdate = true;
        update();
    }
    Q_INVOKABLE inline void camRotate(qreal x, qreal y, qreal z) {
        matrix.rotation.x += x;
        matrix.rotation.y += y;
        matrix.rotation.z += z;
        matrix.needsUpdate = true;
        update();
    }

    inline qreal camX() const{return matrix.position.x;}
    inline qreal camY() const{return matrix.position.y;}
    inline qreal camZ() const{return matrix.position.z;}
    inline qreal camPOV() const{return matrix.pov;}

    Q_INVOKABLE inline void setCamX(qreal x) {
        matrix.position.x = x;
        matrix.needsUpdate = true;
        update();
    }
    Q_INVOKABLE inline void setCamY(qreal y) {
        matrix.position.y = y;
        matrix.needsUpdate = true;
        update();
    }
    Q_INVOKABLE inline void setCamZ(qreal z) {
        matrix.position.z = z;
        matrix.needsUpdate = true;
        update();
    }
    Q_INVOKABLE inline void setCamPOV(qreal pov) {
        matrix.pov = pov;
        matrix.needsUpdate = true;
        update();
    }
    Q_INVOKABLE inline void setCam(qreal x=0, qreal y=0, qreal z=0, qreal pov=45) {
        matrix.position.x = x;
        matrix.position.y = y;
        matrix.position.z = z;
        matrix.pov = pov;
        matrix.needsUpdate = true;
        update();
    }

    Q_INVOKABLE inline MoeGLScene* scene() const{return _scene;}
    Q_INVOKABLE void setScene(MoeGLScene* scene);

public slots:
    void update();

private:
    GLRawMatrix matrix;

    MoeGLScene* _scene;
    RenderBuffer* renderBuffer;
    QSize reportedSize;
    bool needsUpdate;
};

#endif // MOEGLGRAPHICSVIEW_H
