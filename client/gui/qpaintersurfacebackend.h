#ifndef QPAINTERSURFACEBACKEND_H
#define QPAINTERSURFACEBACKEND_H

#include "abstractsurfacebackend.h"
#include "opengl/moeglcubemodel.h"

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QFile>

#include <QGLFramebufferObject>
#include <QMatrix4x4>

#ifdef DrawText
#undef DrawText
#endif

class QPainterSurfaceBackend : public AbstractSurfaceBackend {
public:
    inline bool renderInstructions(RenderInstructions instructions, QRect paintRect, QSize bufferSize) {
        if(instructions.isEmpty())
            return false;

        if(!pendingInstructions.isEmpty()) {
            qWarning() << this << "received new instructions while old instructions pending";
            if(paintRect == QRect(QPoint(0,0),bufferSize)) {
                RenderInstructions oldInstructions = pendingInstructions;
                pendingInstructions.clear();
                bool breakLoop = false;
                foreach(RenderInstruction inst, oldInstructions) {
                    switch(inst.type) {
                        case RenderInstruction::BufferLoadImage:
                            pendingInstructions.append(inst);
                            break;

                        default:
                            breakLoop = true;
                            break;
                    }
                    if(breakLoop)
                        break;
                }
            }
            foreach(RenderInstruction inst, instructions)
                pendingInstructions.append(inst);
        } else
            pendingInstructions = instructions;
        pendingBufferSize = bufferSize;
        pendingPaintRect = paintRect;

        repaint(paintRect);
        return true;
    }

    void paint(QPainter&);
    virtual void repaint(QRect) =0;

    inline bool hasPendingInstructions() const{
        return !pendingInstructions.isEmpty();
    }

    inline QSize bufferSize() const{
        return pendingBufferSize;
    }

protected:
    explicit inline QPainterSurfaceBackend(QRect geom) : AbstractSurfaceBackend(geom) {
        moveToThread(qApp->thread());
    }

    inline QPixmap getCheckerBoardImage(Qt::GlobalColor color =Qt::magenta) {
        static const QSize boardSize(10, 10);
        static const Qt::GlobalColor color2(Qt::white);
        static QMap<Qt::GlobalColor, QPixmap> checkboards;

        QPixmap pixmap = checkboards.value(color);
        if(pixmap.isNull()) {
            pixmap = QPixmap(QSize(boardSize.width()*2, boardSize.height()*2));
            QPainter p(&pixmap);
            p.fillRect(QRect(QPoint(0,0),boardSize),color);
            p.fillRect(QRect(QPoint(boardSize.width(),0),boardSize),color2);
            p.fillRect(QRect(QPoint(0,boardSize.height()),boardSize),color2);
            p.fillRect(QRect(QPoint(boardSize.width(),boardSize.height()),boardSize),color);
            checkboards.insert(color, pixmap);
        }

        return pixmap;
    }

private:
    struct ShaderProgram {
        bool link(QString name) {
            if(!program.link())
                return false;

            attrib.vector = program.attributeLocation("vertexPosition");
            attrib.colour = program.attributeLocation("vertexColour");
            attrib.matrix = program.uniformLocation("modelMatrix");
            attrib.camMatrix = program.uniformLocation("matrix");
            attrib.texture = program.uniformLocation("texture");
            attrib.texCoord = program.attributeLocation("texCoord");

            qDebug() << "Compiled Shader Program" << name
                                 << "Attributes"
                                 << attrib.vector
                                 << attrib.colour
                                 << attrib.texCoord
                                 << "Uniforms"
                                 << attrib.matrix
                                 << attrib.camMatrix
                                 << attrib.texture;

            return true;
        }

        inline QString log() const{return program.log();}

        struct {
            int vector;
            int colour;
            int matrix;
            int camMatrix;
            int texCoord;
            int texture;
        } attrib;
        QGLShaderProgram program;
    };
    struct GLRenderBuffer {
        inline GLRenderBuffer() : fbo(0), shadow(0),
            lightColor(Qt::white), lightPos(0, 2, 2), shadowFBO(0) {}

        QGLFramebufferObject* fbo;
        QMatrix4x4 camMatrix;

        float shadow;
        QColor lightColor;
        QVector3D lightPos;
        QMatrix4x4 shadowMatrix;
        QGLFramebufferObject* shadowFBO;
    };
    struct GLModel {
        inline GLModel() : texture(0), shader(0) {}

        GLuint texture;
        QGLBuffer vectors;
        QGLBuffer colours;
        QMatrix4x4 matrix;
        QGLBuffer textCoords;
        ShaderProgram* shader;
    };

    inline QGLShader* getShader(QString path, QGLShader::ShaderTypeBit type=
                                                (QGLShader::ShaderTypeBit)-1) {
        static QHash<QString, QGLShader*> shaders;
        QGLShader* shader = shaders.value(path);
        if(!shader) {
            if(type == -1) {
                if(path.endsWith(".vert"))
                    type = QGLShader::Vertex;
                else if(path.endsWith(".frag"))
                    type = QGLShader::Fragment;
                else if(path.endsWith(".geom"))
                    type = QGLShader::Geometry;
                else
                    throw "Invalid shader";
            }
            shader = new QGLShader(type);

            if(!shader->compileSourceFile(path))
                qWarning() << path << shader->log();
        }
        return shader;
    }

    inline ShaderProgram* getShaderProgram(QString name) {
        ShaderProgram* shader = shaderPrograms.value(name);
        if(!shader) {
            shader = new ShaderProgram;

            QString vert;
            QString frag;
            if(name == "coloured") {
                vert = ":/shaders/matrix.vert";
                frag = ":/shaders/colour.frag";
            } else if(name == "textured") {
                vert = ":/shaders/matrix-texture.vert";
                frag = ":/shaders/texture.frag";
            } else if(name == "shadow") {
                vert = ":/shaders/shadow.vert";
                frag = ":/shaders/shadow.frag";
            } else if(name == "shadow-coloured") {
                vert = ":/shaders/shadow-coloured.vert";
                frag = ":/shaders/shadow-coloured.frag";
            } else if(name == "shadow-textured") {
                vert = ":/shaders/shadow-textured.vert";
                frag = ":/shaders/shadow-textured.frag";
            }

            shader->program.addShader(getShader(vert));
            shader->program.addShader(getShader(frag));
            if(!shader->link(name))
                qWarning() << shader->log();


            shaderPrograms.insert(name, shader);
        }
        return shader;
    }

    QHash<QString, ShaderProgram*> shaderPrograms;

    QRect pendingPaintRect;
    QSize pendingBufferSize;
    RenderInstructions pendingInstructions;

    QMap<quintptr, QPixmap> renderBuffers;
    QMap<quintptr, GLRenderBuffer*> glBuffers;
    RenderInstructions::Iterator shadowPass;
    QMap<quintptr, GLModel*> glModels;
    GLRenderBuffer* activeGLBuffer;
    bool inShadowPass;
};

#endif // QPAINTERSURFACEBACKEND_H
