#ifndef QPAINTERSURFACEBACKEND_H
#define QPAINTERSURFACEBACKEND_H

#include "abstractsurfacebackend.h"

#include <QApplication>
#include <QPainter>
#include <QDebug>


class QPainterSurfaceBackend : public AbstractSurfaceBackend {
public:
    inline void renderInstructions(RenderInstructions instructions, QRect paintRect, QSize bufferSize) {
        pendingInstructions = instructions;
        pendingBufferSize = bufferSize;
        pendingPaintRect = paintRect;

        repaint(paintRect);
    }

    void paint(QPainter& p) {
        p.setPen(Qt::black);
        p.setBrush(Qt::darkMagenta);
        p.setClipRect(pendingPaintRect);

        foreach(RenderInstruction inst, pendingInstructions) {
            switch(inst.type){
                case RenderInstruction::FillRect:
                    p.fillRect(inst.arguments.at(0).toRect(), QColor::fromRgba(inst.arguments.at(1).toUInt()));
                    break;

                case RenderInstruction::DrawLine:
                    p.drawLine(inst.arguments.at(0).toPoint(),
                               inst.arguments.at(1).toPoint());
                    break;

                case RenderInstruction::DrawRect:
                    p.drawRect(inst.arguments.at(0).toRect());
                    break;

                case RenderInstruction::DrawText:
                    p.drawText(inst.arguments.at(0).toRect(), inst.arguments.at(1).toString());
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
                        p.setClipRect(inst.arguments.first().toRect(), Qt::ReplaceClip);
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

                case RenderInstruction::RenderBuffer:
                {
                    quintptr id = inst.arguments.first().value<quintptr>();
                    QRect dest = inst.arguments.at(1).toRect();
                    QPixmap buffer;
                    if(inst.arguments.size() >= 3) {
                        buffer.loadFromData(inst.arguments.at(2).toByteArray());
                        renderBuffers.insert(id, buffer);
                    } else
                        buffer = renderBuffers.value(id);

                    if(buffer.isNull())
                        if(!renderBuffers.contains(id)) {
                            p.drawTiledPixmap(dest, getCheckerBoardImage());
                            return;
                        } else
                            p.drawTiledPixmap(dest, getCheckerBoardImage(Qt::red));
                    else {
                        QSize scaledSize(buffer.size().scaled(dest.width(), dest.height(), Qt::KeepAspectRatio));
                        p.drawPixmap(QRect(dest.topLeft() + QPoint(dest.width()/2-scaledSize.width()/2,
                                           dest.height()/2-scaledSize.height()/2), scaledSize), buffer);
                    }
                }
                break;

                default:
                    qWarning() << "Unhandled Recorder Instruction" << inst.type;
                break;
            }
        }
        pendingInstructions.clear();
        emit readyForFrame();
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
    RenderInstructions pendingInstructions;
};

#endif // QPAINTERSURFACEBACKEND_H
