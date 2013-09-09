#include "moeglvertexmodel.h"
#include "gui/renderrecorder.h"
#include "moeglspritesheet.h"

#include <QFile>

MoeGLVertexModel::MoeGLVertexModel(MoeGLSpriteSheet* texture)
{
    needsCompiling = false;
    this->texture = texture;
    shaderProgram = texture ? "textured" : "coloured";
    textureChanged = !!texture;
    shaderChanged = true;
}

void MoeGLVertexModel::setTexture(MoeGLSpriteSheet *texture) {
    this->texture = texture;
    emit contentChanged();
    shaderProgram = texture ? "textured" : "coloured";
    textureChanged = true;
    shaderChanged = true;
}

void MoeGLVertexModel::render(RenderRecorder *p) {
    if(needsCompiling) {
        p->allocateGLModel(ptr(), _vectors, _colours);

        _vectors.clear();
        _colours.clear();

        needsCompiling = false;
    }
    if(texture && texture->needsUpdate) {
        if(!texture->created) {
            p->allocateGLBuffer(texture->ptr(), texture->size());
            texture->created = true;
        }
        {
            MoeGLSpriteSheet::HashPTR::Iterator i = texture->blitBufferRequests.begin();
            while(i != texture->blitBufferRequests.end()) {
                qDebug() << i.key() << i.value();
                i++;
            }
            texture->blitBufferRequests.clear();
        }
        {
            MoeGLSpriteSheet::HashData::Iterator i = texture->dataBufferRequests.begin();
            while(i != texture->dataBufferRequests.end()) {
                p->blitGLBuffer(texture->ptr(), i.key(), i.value());
                qDebug() << i.key();
                i++;
            }
            texture->dataBufferRequests.clear();
        }

        texture->needsUpdate = false;
    }
    if(textureChanged) {
        p->updateGLModelTexture(ptr(), texture->ptr());
        textureChanged = false;
    }
    if(shaderChanged) {
        p->updateGLModelShader(ptr(), shaderProgram);
        shaderChanged = false;
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
