#ifndef QPAINTERSURFACEBACKEND_H
#define QPAINTERSURFACEBACKEND_H

#include "abstractsurfacebackend.h"

#include <QApplication>
#include <QPainter>
#include <QDebug>

class QPainterSurfaceBackend : public AbstractSurfaceBackend {
public:
    inline void renderInstructions(RenderInstructions instructions, QRect paintRect, QSize bufferSize) {
        qDebug() << "Received" << instructions.size() << "Instructions";

        pendingInstructions = instructions;
        pendingBufferSize = bufferSize;
        pendingPaintRect = paintRect;

        repaint(paintRect);
    }

    void paint(QPainter& p) {
        qDebug() << "Painting" << pendingInstructions.size() << "Instructions" << pendingPaintRect << pendingBufferSize;

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
                        p.setPen(QColor::fromRgba(inst.arguments.at(0).toUInt()));
                    break;

                case RenderInstruction::UpdateBrush:
                    if(inst.arguments.isEmpty())
                        p.setBrush(Qt::NoBrush);
                    else
                        p.setBrush(QColor::fromRgba(inst.arguments.at(0).toUInt()));
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

private:
    QRect pendingPaintRect;
    QSize pendingBufferSize;
    RenderInstructions pendingInstructions;
};

#endif // QPAINTERSURFACEBACKEND_H
