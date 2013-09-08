#include "moeglvertexmodel.h"
#include "gui/renderrecorder.h"

#include <QFile>

MoeGLVertexModel::MoeGLVertexModel()
{
    needsCompiling = false;
}

void MoeGLVertexModel::render(RenderRecorder *p) {
    if(needsCompiling) {
        p->updateGLModel(ptr(), _vectors, _colours);

        _vectors.clear();
        _colours.clear();

        needsCompiling = false;
    }
    if(matrix.needsUpdate) {
        p->updateGLModelMatrix(ptr(), matrix.toMatrix4x4());
        matrix.needsUpdate = false;
    }
    p->renderGLModel(ptr());
}

void MoeGLVertexModel::clear() {
    needsCompiling = true;
    _vectors.clear();
    _colours.clear();
}
