#ifndef VERTEXMODEL_H
#define VERTEXMODEL_H

#include <core/moeobject.h>

#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QGLBuffer>
#include <QVector3D>

class MoeGLVertexModel : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLVertexModel();

    inline int addVertex(QVector3D vec, QColor color) {
        int index = _colours.size();
        _colours << (GLrgb){(GLfloat)color.redF(), (GLfloat)color.greenF(), (GLfloat)color.blueF()};
        _vectors << vec;

        return index;
    }

    void updateColour(int index, QColor);
    void updateVertex(int index, QVector3D);

    void render(QMatrix4x4& camera);
    bool compile();
    void clear();

    struct GLrgb {
        GLfloat r;
        GLfloat g;
        GLfloat b;
    };

private:
    QGLShaderProgram shaderProgram;

    QVector<QVector3D> _vectors;
    QVector<GLrgb> _colours;

    struct {
        int vector;
        int colour;
        int matrix;
        int camMatrix;
    } attrib;

    QMatrix4x4 matrix;
    
};

#endif // VERTEXMODEL_H
