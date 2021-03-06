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
    shaderProgram = texture ? "textured" : "coloured";
    textureChanged = true;
    shaderChanged = true;

    emit needsUpdateModel();
    emit contentChanged();
}

void MoeGLVertexModel::render(RenderRecorder *p) {
    if(needsCompiling) {
        if(!texture)
            _texCoords.clear();
        p->allocateGLModel(ptr(), _vectors, _colours, _texCoords);

        _vectors.clear();
        _colours.clear();
        _texCoords.clear();
        needsCompiling = false;
    }
    if(texture) {
        if(!texture->created) {
            p->allocateGLBuffer(texture->ptr(), texture->size());
            texture->created = true;
        }
        if(texture->needsUpdate) {
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
    }
    if(shaderChanged) {
        p->updateGLModelShader(ptr(), shaderProgram);
        shaderChanged = false;
    }
    if(textureChanged) {
        p->updateGLModelTexture(ptr(), texture->ptr());
        textureChanged = false;
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
    _texCoords.clear();
}
