#ifndef GLTYPES_H
#define GLTYPES_H

#include <GL/gl.h>

#include <QVector3D>
#include <QMatrix4x4>
#include <QColor>

struct vec3 {
    inline vec3(float x=0, float y=0, float z=0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    inline vec3(QVector3D& vec) {
        *this = vec;
    }
    inline vec3(Qt::GlobalColor gCol) {
        *this = gCol;
    }
    inline vec3(QColor& col) {
        *this = col;
    }

    inline vec3 &operator=(QVector3D& vec) {
        x = vec.x();
        y = vec.y();
        z = vec.z();

        return *this;
    }
    inline vec3 &operator=(Qt::GlobalColor gCol) {
        QColor col(gCol);

        x = col.redF();
        y = col.greenF();
        z = col.blueF();

        return *this;
    }
    inline vec3 &operator=(const QColor& col) {
        x = col.redF();
        y = col.greenF();
        z = col.blueF();

        return *this;
    }

    inline vec3 &operator+=(const vec3 &other) {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }
    inline vec3 &operator-=(const vec3 &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;

        return *this;
    }

    inline bool operator==(const vec3 &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    inline bool operator!=(const vec3 &other) const {
        return !(*this == other);
    }

    inline QVector3D vector() const{return QVector3D(x,y,z);}

    typedef QVector<vec3> list;

    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct GLRawMatrix {
    inline GLRawMatrix() : scale(1, 1, 1), pov(45) {}

    inline QMatrix4x4 toMatrix4x4(QSize size =QSize()) {
        QMatrix4x4 matrix4D;
        if(!size.isEmpty() && pov > 0)
            matrix4D.perspective(pov, size.width()/size.height(), 0.1, 1000);

        if(scale != vec3(1, 1, 1))
            matrix4D.scale(scale.x, scale.y, scale.y);
        if(rotation.x != 0)
            matrix4D.rotate(rotation.x, 1, 0, 0);
        if(rotation.y != 0)
            matrix4D.rotate(rotation.y, 0, 1, 0);
        if(rotation.z != 0)
            matrix4D.rotate(rotation.z, 0, 0, 1);
        if(position != vec3(0, 0, 0))
            matrix4D.translate(position.x, position.y, position.y);

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
