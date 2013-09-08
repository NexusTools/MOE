#ifndef QPAINTERSURFACEBACKEND_H
#define QPAINTERSURFACEBACKEND_H

#include "abstractsurfacebackend.h"

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QFile>

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QGLBuffer>

#include <GL/glu.h>


inline QString getSource(QString fName){
    QFile file(fName);
    if(file.open(QFile::ReadOnly))
        return QString::fromUtf8(file.readAll());
    else
        qWarning() << file.errorString() << fName;
    return "";
}

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

    void paint(QPainter& p) {
        p.setPen(Qt::black);
        p.setBrush(Qt::darkMagenta);
        p.setClipRect(pendingPaintRect);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setRenderHint(QPainter::HighQualityAntialiasing);

        foreach(RenderInstruction inst, pendingInstructions) {
            switch(inst.type){
                case RenderInstruction::FillRect:
                    p.fillRect(inst.arguments.at(0).toRectF(), QColor::fromRgba(inst.arguments.at(1).toUInt()));
                    break;

                case RenderInstruction::DrawLine:
                    p.drawLine(inst.arguments.at(0).toPointF(),
                               inst.arguments.at(1).toPointF());
                    break;

                case RenderInstruction::DrawPixel:
                    p.drawPoint(inst.arguments.first().toPointF());
                    break;

                case RenderInstruction::DrawRect:
                    if(inst.arguments.length() >= 2)
                        p.drawRoundedRect(inst.arguments.at(0).toRectF(), inst.arguments.at(1).toReal(), inst.arguments.at(1).toReal(), Qt::AbsoluteSize);
                    else
                        p.drawRect(inst.arguments.at(0).toRectF());
                    break;

                case RenderInstruction::DrawText:
                    p.drawText(inst.arguments.at(0).toRectF(), Qt::AlignCenter, inst.arguments.at(1).toString());
                    break;

                case RenderInstruction::UpdatePen:
                    if(inst.arguments.isEmpty())
                        p.setPen(Qt::NoPen);
                    else
                        p.setPen(QPen(QColor::fromRgba(inst.arguments.at(0).toUInt()), inst.arguments.at(1).toInt()));
                    break;

                case RenderInstruction::UpdateBrush:
                    if(inst.arguments.isEmpty())
                        p.setBrush(Qt::NoBrush);
                    else
                        p.setBrush(inst.arguments.at(0).value<QBrush>());
                    break;

                case RenderInstruction::UpdateClipRect:
                    if(inst.arguments.isEmpty())
                        p.setClipRect(pendingPaintRect, Qt::ReplaceClip);
                    else
                        p.setClipRect(inst.arguments.first().toRectF(), Qt::ReplaceClip);
                break;

                case RenderInstruction::UpdateOpacity:
                    p.setOpacity(inst.arguments.first().toFloat());
                break;

                case RenderInstruction::UpdateFont:
                    p.setFont(QFont(inst.arguments.at(0).toString(), inst.arguments.at(1).toInt()));
                break;

                case RenderInstruction::UpdateTransform:
                    if(inst.arguments.isEmpty())
                        p.setTransform(QTransform());
                    else
                        p.setTransform(inst.arguments.first().value<QTransform>());
                break;

                case RenderInstruction::UpdateGLScene:
                {
                    quintptr id = inst.arguments.first().value<quintptr>();
                    QSize size = inst.arguments.at(1).toSize();
                    qDebug() << "Updating GL Scene" << size;

                    QGLFramebufferObjectFormat bufferFormat;
                    bufferFormat.setInternalTextureFormat(GL_RGBA8);
                    bufferFormat.setMipmap(true);

                    QGLFramebufferObject* fbo = glBuffers.value(id);
                    if(!fbo) {
                        fbo = new QGLFramebufferObject(size, bufferFormat);
                        glBuffers.insert(id, fbo);
                    } else if(fbo->size() != size) {
                        delete fbo;
                        qDebug() << "Resizing FBO" << size;
                        fbo = new QGLFramebufferObject(size, bufferFormat);
                        glBuffers.insert(id, fbo);
                    }

                    p.end();
                    if(fbo->bind()) {
                        glViewport(0, 0, size.width(), size.height());
                        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_DEPTH_TEST);
                        glEnable(GL_CULL_FACE);
                        glLoadIdentity();

                        static int matrixAttrib;
                        static int colourAttrib;
                        static int vertexAttrib;
                        static QGLShaderProgram shaderProgram;

                        if(!shaderProgram.isLinked()) {
                            qDebug() << "Generating shader program for first time";
                            shaderProgram.addShaderFromSourceCode(QGLShader::Vertex, getSource(":/shaders/matrix.vert"));
                            shaderProgram.addShaderFromSourceCode(QGLShader::Fragment, getSource(":/shaders/colour.frag"));
                            if(!shaderProgram.link())
                                qWarning() << shaderProgram.log();

                            vertexAttrib = shaderProgram.attributeLocation("vertexPosition");
                            colourAttrib = shaderProgram.attributeLocation("vertexColour");
                            matrixAttrib = shaderProgram.uniformLocation("matrix");
                        }


                        shaderProgram.bind();


                        GLfloat vertices[] = {
                            // Front
                            -1, -1,  1,
                             1, -1,  1,
                            -1,  1,  1,

                             1,  1,  1,
                            -1,  1,  1,
                             1, -1,  1,

                            // Left
                            -1, -1, -1,
                            -1, -1,  1,
                            -1,  1, -1,

                            -1, -1,  1,
                            -1,  1,  1,
                            -1,  1, -1,

                            // Back
                            -1,  1, -1,
                             1, -1, -1,
                            -1, -1, -1,

                             1, -1, -1,
                            -1,  1, -1,
                             1,  1, -1,

                            // Right
                             1, -1, -1,
                             1,  1, -1,
                             1, -1,  1,

                             1, -1,  1,
                             1,  1, -1,
                             1,  1,  1,

                            // Top
                             1,  1, -1,
                            -1,  1, -1,
                            -1,  1,  1,

                             1,  1, -1,
                            -1,  1,  1,
                             1,  1,  1,

                            // Bottom
                            -1, -1, -1,
                             1, -1, -1,
                            -1, -1,  1,

                            -1, -1,  1,
                             1, -1, -1,
                             1, -1,  1
                            };

                        GLfloat colors[] = {
                            // Front
                            0.0f, 1.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,

                            0.0f, 1.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,

                            // Left
                            1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f,

                            1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f,

                            // Back
                            0.0f, 1.0f, 1.0f,
                            0.0f, 1.0f, 1.0f,
                            0.0f, 1.0f, 1.0f,

                            0.0f, 1.0f, 1.0f,
                            0.0f, 1.0f, 1.0f,
                            0.0f, 1.0f, 1.0f,

                            // Right
                            1.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 1.0f,

                            1.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 1.0f,

                            // Top
                            1.0f, 0.0f, 0.0f,
                            1.0f, 0.0f, 0.0f,
                            1.0f, 0.0f, 0.0f,

                            1.0f, 0.0f, 0.0f,
                            1.0f, 0.0f, 0.0f,
                            1.0f, 0.0f, 0.0f,

                            // Bottmo
                            0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f,

                            0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f
                        };
                        QMatrix4x4 matrix;
                        matrix.perspective(45, size.width()/size.height(), 0.1, 1000);

                        static int rot = 0;
                        matrix.translate(0, 0, -6);
                        matrix.rotate(rot, 0.3, 1, 0);
                        rot++;

                        shaderProgram.setAttributeArray(vertexAttrib, GL_FLOAT, vertices, 3);
                        shaderProgram.setAttributeArray(colourAttrib, GL_FLOAT, colors, 3);
                        shaderProgram.enableAttributeArray(vertexAttrib);
                        shaderProgram.enableAttributeArray(colourAttrib);
                        shaderProgram.setUniformValue(matrixAttrib, matrix);

                        glDrawArrays(GL_TRIANGLES, 0, 36);

                        shaderProgram.disableAttributeArray(colourAttrib);
                        shaderProgram.disableAttributeArray(vertexAttrib);
                        shaderProgram.release();

                        glPopMatrix();
                        glFlush();
                        fbo->release();

                        glDisable(GL_DEPTH_TEST);
                        glDisable(GL_CULL_FACE);
                    } else
                        qWarning() << "Cannot bind FBO";
                    begin(p);
                    break;
                }

                case RenderInstruction::BufferLoadImage:
                {
                    quintptr id = inst.arguments.first().value<quintptr>();
                    QByteArray data = inst.arguments.at(1).toByteArray();

                    QPixmap buffer;
                    buffer.loadFromData(data);
                    renderBuffers.insert(id, buffer);
                    break;
                }

                case RenderInstruction::RenderBuffer:
                {
                    quintptr id = inst.arguments.first().value<quintptr>();
                    QRectF dest = inst.arguments.at(1).toRectF();
                    QPixmap buffer = renderBuffers.value(id);

                    if(buffer.isNull()) {
                        QGLFramebufferObject* fbo = glBuffers.value(id);
                        if(fbo)
                            blitBuffer(dest.toRect(), fbo);
                        else if(!renderBuffers.contains(id)) {
                            p.drawTiledPixmap(dest, getCheckerBoardImage());
                            return;
                        } else
                            p.drawTiledPixmap(dest, getCheckerBoardImage(Qt::red));
                    } else {
                        QSizeF scaledSize(buffer.size().scaled(dest.width(), dest.height(), Qt::KeepAspectRatio));
                        p.drawPixmap(QRectF(dest.topLeft() + QPointF(dest.width()/2-scaledSize.width()/2,
                                           dest.height()/2-scaledSize.height()/2), scaledSize), buffer,
                                            QRectF(QPoint(0,0),buffer.size()));
                    }
                }
                break;

                default:
                    qWarning() << "Unhandled Recorder Instruction" << inst.type;
                break;
            }
        }
        pendingInstructions.clear();
        markRendered();
    }

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
    QRect pendingPaintRect;
    QSize pendingBufferSize;
    QMap<quintptr, QPixmap> renderBuffers;
    QMap<quintptr, QGLFramebufferObject*> glBuffers;
    RenderInstructions pendingInstructions;
};

#endif // QPAINTERSURFACEBACKEND_H
