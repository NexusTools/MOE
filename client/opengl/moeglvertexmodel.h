#ifndef VERTEXMODEL_H
#define VERTEXMODEL_H

#include <core/moeobject.h>

#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QGLBuffer>
#include <QVector3D>

#include "gltypes.h"

class RenderRecorder;
class MoeGLScene;

class MoeGLVertexModel : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLVertexModel();

    Q_INVOKABLE inline int addVertex(QVector3D vec, QColor color) {
        return addVertex(vec3(vec), vec3(color));
    }
    inline int addVertex(vec3 vec, vec3 color) {
        int index = _colours.size();
        needsCompiling = true;
        _colours << color;
        _vectors << vec;

        return index;
    }


    Q_INVOKABLE inline void translate(qreal x, qreal y, qreal z) {
        matrix.position += vec3(x, y, z);
        matrix.needsUpdate = true;
        emit contentChanged();
    }
    Q_INVOKABLE inline void rotate(qreal x, qreal y, qreal z) {
        matrix.rotation.x += x;
        matrix.rotation.y += y;
        matrix.rotation.z += z;
        matrix.needsUpdate = true;
        emit contentChanged();
    }

    inline qreal setX() const{return matrix.position.x;}
    inline qreal setY() const{return matrix.position.y;}
    inline qreal setZ() const{return matrix.position.z;}

    Q_INVOKABLE inline void x(qreal x) {
        matrix.position.x = x;
        matrix.needsUpdate = true;
        emit contentChanged();
    }
    Q_INVOKABLE inline void y(qreal y) {
        matrix.position.y = y;
        matrix.needsUpdate = true;
        emit contentChanged();
    }
    Q_INVOKABLE inline void z(qreal z) {
        matrix.position.z = z;
        matrix.needsUpdate = true;
        emit contentChanged();
    }

    void updateColour(int index, QColor);
    void updateVertex(int index, QVector3D);

    inline bool isContentCompiled() const{return !needsCompiling;}

    void render(RenderRecorder* p);
    void updateScene();
    void clear();

signals:
    void contentChanged();

private:
    GLRawMatrix matrix;

    QVector<vec3> _vectors;
    QVector<vec3> _colours;
    bool needsCompiling;
    
};

#endif // VERTEXMODEL_H
