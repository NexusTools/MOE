#include "moeglcubemodel.h"

MoeGLCubeModel::MoeGLCubeModel()
{
    // Top
    addVertex(QVector3D(-1, -1,  1), Qt::red);
    addVertex(QVector3D(1, -1,  1), Qt::red);
    addVertex(QVector3D( -1,  1,  1), Qt::red);

    addVertex(QVector3D(  1,  1,  1), Qt::red);
    addVertex(QVector3D( -1,  1,  1), Qt::red);
    addVertex(QVector3D(  1, -1,  1), Qt::red);

    // Left
    addVertex(QVector3D( -1, -1, -1), Qt::green);
    addVertex(QVector3D( -1, -1,  1), Qt::green);
    addVertex(QVector3D( -1,  1, -1), Qt::green);

    addVertex(QVector3D( -1, -1,  1), Qt::green);
    addVertex(QVector3D( -1,  1,  1), Qt::green);
    addVertex(QVector3D( -1,  1, -1), Qt::green);

    // Back
    addVertex(QVector3D( -1,  1, -1), Qt::blue);
    addVertex(QVector3D(  1, -1, -1), Qt::blue);
    addVertex(QVector3D( -1, -1, -1), Qt::blue);

    addVertex(QVector3D(  1, -1, -1), Qt::blue);
    addVertex(QVector3D( -1,  1, -1), Qt::blue);
    addVertex(QVector3D(  1,  1, -1), Qt::blue);

    // Right
    addVertex(QVector3D(  1, -1, -1), Qt::magenta);
    addVertex(QVector3D(  1,  1, -1), Qt::magenta);
    addVertex(QVector3D(  1, -1,  1), Qt::magenta);

    addVertex(QVector3D(  1, -1,  1), Qt::magenta);
    addVertex(QVector3D(  1,  1, -1), Qt::magenta);
    addVertex(QVector3D(  1,  1,  1), Qt::magenta);

    // Top
    addVertex(QVector3D(  1,  1, -1), Qt::yellow);
    addVertex(QVector3D( -1,  1, -1), Qt::yellow);
    addVertex(QVector3D( -1,  1,  1), Qt::yellow);

    addVertex(QVector3D(  1,  1, -1), Qt::yellow);
    addVertex(QVector3D( -1,  1,  1), Qt::yellow);
    addVertex(QVector3D(  1,  1,  1), Qt::yellow);

    // Bottom
    addVertex(QVector3D( -1, -1, -1), Qt::black);
    addVertex(QVector3D(  1, -1, -1), Qt::black);
    addVertex(QVector3D( -1, -1,  1), Qt::black);

    addVertex(QVector3D( -1, -1,  1), Qt::black);
    addVertex(QVector3D(  1, -1, -1), Qt::black);
    addVertex(QVector3D(  1, -1,  1), Qt::black);
}
