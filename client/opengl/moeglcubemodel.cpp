#include "moeglcubemodel.h"

MoeGLCubeModel::MoeGLCubeModel() {
    connect(this, SIGNAL(needsUpdateModel()), this, SLOT(genModel()));
    genModel();
}

void MoeGLCubeModel::genModel() {
    qDebug() << "Generating Cube Model" << hasTexture();
    clear();

    // Top
    Qt::GlobalColor color = hasTexture() ? Qt::white : Qt::red;

    addVertex(QVector3D(-1, -1,  1), color);
    addVertex(QVector3D(1, -1,  1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), color, QPoint(0, 1));

    addVertex(QVector3D(  1,  1,  1), color, QPoint(1, 1));
    addVertex(QVector3D( -1,  1,  1), color, QPoint(0, 1));
    addVertex(QVector3D(  1, -1,  1), color, QPoint(1, 0));

    // Left
    if(!hasTexture())
        color = Qt::green;
    addVertex(QVector3D( -1, -1, -1), color);
    addVertex(QVector3D( -1, -1,  1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), color, QPoint(0, 1));

    addVertex(QVector3D( -1, -1,  1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), color, QPoint(1, 1));
    addVertex(QVector3D( -1,  1, -1), color, QPoint(0, 1));

    // Back
    if(!hasTexture())
        color = Qt::blue;
    addVertex(QVector3D( -1,  1, -1), color, QPoint(0, 1));
    addVertex(QVector3D(  1, -1, -1), color, QPoint(1, 0));
    addVertex(QVector3D( -1, -1, -1), color);

    addVertex(QVector3D(  1, -1, -1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), color, QPoint(0, 1));
    addVertex(QVector3D(  1,  1, -1), color, QPoint(1, 1));

    // Right
    if(!hasTexture())
        color = Qt::magenta;
    addVertex(QVector3D(  1, -1, -1), color);
    addVertex(QVector3D(  1,  1, -1), color, QPoint(0, 1));
    addVertex(QVector3D(  1, -1,  1), color, QPoint(1, 0));

    addVertex(QVector3D(  1, -1,  1), color, QPoint(1, 0));
    addVertex(QVector3D(  1,  1, -1), color, QPoint(0, 1));
    addVertex(QVector3D(  1,  1,  1), color, QPoint(1, 1));

    // Top
    if(!hasTexture())
        color = Qt::yellow;
    addVertex(QVector3D(  1,  1, -1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1, -1), color);
    addVertex(QVector3D( -1,  1,  1), color, QPoint(0, 1));

    addVertex(QVector3D(  1,  1, -1), color, QPoint(1, 0));
    addVertex(QVector3D( -1,  1,  1), color, QPoint(0, 1));
    addVertex(QVector3D(  1,  1,  1), color, QPoint(1, 1));

    // Bottom
    addVertex(QVector3D( -1, -1, -1), Qt::white);
    addVertex(QVector3D(  1, -1, -1), Qt::white, QPoint(1, 0));
    addVertex(QVector3D( -1, -1,  1), Qt::white, QPoint(0, 1));

    addVertex(QVector3D( -1, -1,  1), Qt::white, QPoint(0, 1));
    addVertex(QVector3D(  1, -1, -1), Qt::white, QPoint(1, 0));
    addVertex(QVector3D(  1, -1,  1), Qt::white, QPoint(1, 1));
}
