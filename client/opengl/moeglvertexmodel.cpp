#include "moeglvertexmodel.h"

#include <QFile>

inline QString getSource(QString fName){
    QFile file(fName);
    if(file.open(QFile::ReadOnly))
        return QString::fromUtf8(file.readAll());
    else
        qWarning() << file.errorString() << fName;
    return "";
}

MoeGLVertexModel::MoeGLVertexModel()
{
}

bool MoeGLVertexModel::compile() {
    if(!shaderProgram.isLinked()) {
        shaderProgram.addShaderFromSourceCode(QGLShader::Vertex, getSource(":/shaders/matrix.vert"));
        shaderProgram.addShaderFromSourceCode(QGLShader::Fragment, getSource(":/shaders/colour.frag"));
        if(!shaderProgram.link()) {
            qWarning() << shaderProgram.log();
            return false;
        }

        attrib.vector = shaderProgram.attributeLocation("vertexPosition");
        attrib.colour = shaderProgram.attributeLocation("vertexColour");
        attrib.matrix = shaderProgram.uniformLocation("modelMatrix");
        attrib.camMatrix = shaderProgram.uniformLocation("matrix");
    }

    return shaderProgram.isLinked();
}

void MoeGLVertexModel::render(QMatrix4x4 &camera) {
    shaderProgram.bind();

    shaderProgram.setAttributeArray(attrib.vector, GL_FLOAT, _vectors.data(), 3);
    shaderProgram.setAttributeArray(attrib.colour, GL_FLOAT, (const GLfloat*)_colours.data(), 3);

    shaderProgram.enableAttributeArray(attrib.vector);
    shaderProgram.enableAttributeArray(attrib.colour);
    shaderProgram.setUniformValue(attrib.matrix, matrix);
    shaderProgram.setUniformValue(attrib.camMatrix, camera);

    glDrawArrays(GL_TRIANGLES, 0, _vectors.size());

    shaderProgram.disableAttributeArray(attrib.colour);
    shaderProgram.disableAttributeArray(attrib.vector);
    shaderProgram.release();
}
