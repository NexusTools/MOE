#ifndef MOEGLSCENE_H
#define MOEGLSCENE_H

#include <core/moeobject.h>

class MoeGLVertexModel;
class RenderRecorder;

class MoeGLScene : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLScene();
    Q_INVOKABLE void addModel(MoeGLVertexModel*);

    void render(RenderRecorder *p);

signals:
    void contentChanged();
    
private:
    QList<MoeGLVertexModel*> models;
};

#endif // MOEGLSCENE_H
