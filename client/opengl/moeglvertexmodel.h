#ifndef VERTEXMODEL_H
#define VERTEXMODEL_H

#include <core/moeobject.h>

#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QGLBuffer>
#include <QVector3D>

#include "gltypes.h"

class RenderRecorder;
class MoeGLSpriteSheet;
class MoeGLScene;

class MoeGLVertexModel : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLVertexModel(MoeGLSpriteSheet* texture =0);
    Q_INVOKABLE void setTexture(MoeGLSpriteSheet* texture =0);
    Q_INVOKABLE bool hasTexture() const{return !!texture;}

    Q_INVOKABLE inline int addVertex(QVector3D vec, QColor color, QPoint texCoord =QPoint(0, 0)) {
        return addVertex(vec3(vec), vec3(color), vec2(texCoord));
    }
    inline int addVertex(vec3 vec, vec3 color, vec2 tex =vec2()) {
        int index = _colours.size();
        needsCompiling = true;
        _texCoords << vec2(tex.x, 1-tex.y);
        _colours << color;
        _vectors << vec;

        return index;
    }


    Q_INVOKABLE inline void translate(qreal x, qreal y, qreal z) {
        matrix.position.x += x;
        matrix.position.y += y;
        matrix.position.z += z;
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
    void needsUpdateModel();

private:
    GLRawMatrix matrix;
    QString shaderProgram;

    QVector<vec3> _vectors;
    QVector<vec3> _colours;
    QVector<vec2> _texCoords;
    MoeGLSpriteSheet* texture;
    bool needsCompiling;
    bool textureChanged;
    bool shaderChanged;
    
};

#endif // VERTEXMODEL_H
