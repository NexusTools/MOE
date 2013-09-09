#include "moeglcubemodel.h"

MoeGLCubeModel::MoeGLCubeModel()
{
    // Top
    addVertex(QVector3D(-1, -1,  1), Qt::red);
    addVertex(QVector3D(1, -1,  1), Qt::red, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), Qt::red, QPoint(0, 1));

    addVertex(QVector3D(  1,  1,  1), Qt::red, QPoint(1, 1));
    addVertex(QVector3D( -1,  1,  1), Qt::red, QPoint(0, 1));
    addVertex(QVector3D(  1, -1,  1), Qt::red, QPoint(1, 0));

    // Left
    addVertex(QVector3D( -1, -1, -1), Qt::green);
    addVertex(QVector3D( -1, -1,  1), Qt::green, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), Qt::green, QPoint(0, 1));

    addVertex(QVector3D( -1, -1,  1), Qt::green, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), Qt::green, QPoint(1, 1));
    addVertex(QVector3D( -1,  1, -1), Qt::green, QPoint(0, 1));

    // Back
    addVertex(QVector3D( -1,  1, -1), Qt::blue, QPoint(0, 1));
    addVertex(QVector3D(  1, -1, -1), Qt::blue, QPoint(1, 0));
    addVertex(QVector3D( -1, -1, -1), Qt::blue);

    addVertex(QVector3D(  1, -1, -1), Qt::blue, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), Qt::blue, QPoint(0, 1));
    addVertex(QVector3D(  1,  1, -1), Qt::blue, QPoint(1, 1));

    // Right
    addVertex(QVector3D(  1, -1, -1), Qt::magenta);
    addVertex(QVector3D(  1,  1, -1), Qt::magenta, QPoint(0, 1));
    addVertex(QVector3D(  1, -1,  1), Qt::magenta, QPoint(1, 0));

    addVertex(QVector3D(  1, -1,  1), Qt::magenta, QPoint(1, 0));
    addVertex(QVector3D(  1,  1, -1), Qt::magenta, QPoint(0, 1));
    addVertex(QVector3D(  1,  1,  1), Qt::magenta, QPoint(1, 1));

    // Top
    addVertex(QVector3D(  1,  1, -1), Qt::yellow, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), Qt::yellow);
    addVertex(QVector3D( -1,  1,  1), Qt::yellow, QPoint(0, 1));

    addVertex(QVector3D(  1,  1, -1), Qt::yellow, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), Qt::yellow, QPoint(0, 1));
    addVertex(QVector3D(  1,  1,  1), Qt::yellow, QPoint(1, 1));

    // Bottom
    addVertex(QVector3D( -1, -1, -1), Qt::white);
    addVertex(QVector3D(  1, -1, -1), Qt::white, QPoint(1, 0));
    addVertex(QVector3D( -1, -1,  1), Qt::white, QPoint(0, 1));

    addVertex(QVector3D( -1, -1,  1), Qt::white, QPoint(0, 1));
    addVertex(QVector3D(  1, -1, -1), Qt::white, QPoint(1, 0));
    addVertex(QVector3D(  1, -1,  1), Qt::white, QPoint(1, 1));
}
