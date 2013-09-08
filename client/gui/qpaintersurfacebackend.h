#ifndef QPAINTERSURFACEBACKEND_H
#define QPAINTERSURFACEBACKEND_H

#include "abstractsurfacebackend.h"

#include <QGLFramebufferObject>
#include <QApplication>
#include <QPainter>
#include <QDebug>

#include <GL/glu.h>

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

                    QGLFramebufferObjectFormat bufferFormat;
                    bufferFormat.setInternalTextureFormat(GL_RGBA8);
                    bufferFormat.setMipmap(true);

                    QGLFramebufferObject* fbo = glBuffers.value(id);
                    if(!fbo) {
                        fbo = new QGLFramebufferObject(bufferSize(), bufferFormat);
                        glBuffers.insert(id, fbo);
                    } else if(fbo->size() != size) {
                        delete fbo;
                        qDebug() << "Resizing FBO" << bufferSize();
                        fbo = new QGLFramebufferObject(bufferSize(), bufferFormat);
                        glBuffers.insert(id, fbo);
                    }

                    p.end();
                    if(fbo->bind()) {
                        glClearColor(0, 0, 0, 1);
                        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                        glViewport(0, 0, size.width(), size.height());
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();
                        gluPerspective(65.0, (float)size.width() / size.height(), 0, 10000);
                        glMatrixMode(GL_MODELVIEW);

                        //Multi-colored side - FRONT
                        glBegin(GL_POLYGON);

                        glColor3f( 1.0, 0.0, 0.0 );     glVertex3f(  0.5, -0.5, -0.5 );      // P1 is red
                        glColor3f( 0.0, 1.0, 0.0 );     glVertex3f(  0.5,  0.5, -0.5 );      // P2 is green
                        glColor3f( 0.0, 0.0, 1.0 );     glVertex3f( -0.5,  0.5, -0.5 );      // P3 is blue
                        glColor3f( 1.0, 0.0, 1.0 );     glVertex3f( -0.5, -0.5, -0.5 );      // P4 is purple

                        glEnd();

                        // White side - BACK
                        glBegin(GL_POLYGON);
                        glColor3f(   1.0,  1.0, 1.0 );
                        glVertex3f(  0.5, -0.5, 0.5 );
                        glVertex3f(  0.5,  0.5, 0.5 );
                        glVertex3f( -0.5,  0.5, 0.5 );
                        glVertex3f( -0.5, -0.5, 0.5 );
                        glEnd();

                        // Purple side - RIGHT
                        glBegin(GL_POLYGON);
                        glColor3f(  1.0,  0.0,  1.0 );
                        glVertex3f( 0.5, -0.5, -0.5 );
                        glVertex3f( 0.5,  0.5, -0.5 );
                        glVertex3f( 0.5,  0.5,  0.5 );
                        glVertex3f( 0.5, -0.5,  0.5 );
                        glEnd();

                        // Green side - LEFT
                        glBegin(GL_POLYGON);
                        glColor3f(   0.0,  1.0,  0.0 );
                        glVertex3f( -0.5, -0.5,  0.5 );
                        glVertex3f( -0.5,  0.5,  0.5 );
                        glVertex3f( -0.5,  0.5, -0.5 );
                        glVertex3f( -0.5, -0.5, -0.5 );
                        glEnd();

                        // Blue side - TOP
                        glBegin(GL_POLYGON);
                        glColor3f(   0.0,  0.0,  1.0 );
                        glVertex3f(  0.5,  0.5,  0.5 );
                        glVertex3f(  0.5,  0.5, -0.5 );
                        glVertex3f( -0.5,  0.5, -0.5 );
                        glVertex3f( -0.5,  0.5,  0.5 );
                        glEnd();

                        // Red side - BOTTOM
                        glBegin(GL_POLYGON);
                        glColor3f(   1.0,  0.0,  0.0 );
                        glVertex3f(  0.5, -0.5, -0.5 );
                        glVertex3f(  0.5, -0.5,  0.5 );
                        glVertex3f( -0.5, -0.5,  0.5 );
                        glVertex3f( -0.5, -0.5, -0.5 );
                        glEnd();

                        glFlush();
                        fbo->release();
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
