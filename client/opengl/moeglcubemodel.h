#ifndef MOEGLCUBEMODEL_H
#define MOEGLCUBEMODEL_H

#include "moeglvertexmodel.h"

class MoeGLCubeModel : public MoeGLVertexModel
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLCubeModel();

public slots:
    void genModel();
    
};

#endif // MOEGLCUBEMODEL_H
