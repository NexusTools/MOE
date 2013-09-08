#include "moeglscene.h"

#include "moeglvertexmodel.h"

MoeGLScene::MoeGLScene()
{
}

void MoeGLScene::addModel(MoeGLVertexModel * model) {
    models << model;
    connect(model, SIGNAL(contentChanged()), this, SIGNAL(contentChanged()));
    emit contentChanged();
}

void MoeGLScene::render(RenderRecorder *p) {
    foreach(MoeGLVertexModel* model, models)
        model->render(p);
}
