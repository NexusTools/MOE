#ifndef GLTYPES_H
#define GLTYPES_H

#include <GL/gl.h>

#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QColor>

struct vec3 {
    inline vec3(float x=0, float y=0, float z=0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    inline vec3(QVector3D& vec) {
        x = vec.x();
        y = vec.y();
        z = vec.z();
    }
    inline vec3(Qt::GlobalColor gCol) {
        QColor col(gCol);

        x = col.redF();
        y = col.greenF();
        z = col.blueF();
    }
    inline vec3(QColor& col) {
        x = col.redF();
        y = col.greenF();
        z = col.blueF();
    }

    inline QVector3D vector() const{return QVector3D(x,y,z);}
    typedef QVector<vec3> list;

    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct vec2 {
    inline vec2(float x=0, float y=0) {
        this->x = x;
        this->y = y;
    }
    inline vec2(QVector2D& vec) {
        x = vec.x();
        y = vec.y();
    }
    inline vec2(QPoint& p) {
        x = p.x();
        y = p.y();
    }
    inline vec2(QSize& s) {
        x = s.width();
        y = s.height();
    }

    inline QVector2D vector() const{return QVector2D(x,y);}
    typedef QVector<vec2> list;

    GLfloat x;
    GLfloat y;
};

struct GLRawMatrix {
    inline GLRawMatrix() : scale(1, 1, 1), pov(45) {}

    inline QMatrix4x4 toMatrix4x4(QSizeF size =QSizeF(0, 0)) {
        QMatrix4x4 matrix4D;
        if(size.width() > 0 && size.height() > 0 && pov > 0)
            matrix4D.perspective(pov, size.width()/size.height(), 0.1, 1000);

        matrix4D.scale(scale.x, scale.y, scale.y);
        matrix4D.translate(position.x, position.y, position.z);
        if(rotation.x != 0)
            matrix4D.rotate(rotation.x, 1, 0, 0);
        if(rotation.y != 0)
            matrix4D.rotate(rotation.y, 0, 1, 0);
        if(rotation.z != 0)
            matrix4D.rotate(rotation.z, 0, 0, 1);

        return matrix4D;
    }

    vec3 scale;
    vec3 position;
    vec3 rotation;
    GLfloat pov;

    bool needsUpdate;
};

Q_DECLARE_METATYPE(vec3)
Q_DECLARE_METATYPE(vec3::list)

#endif // GLTYPES_H
