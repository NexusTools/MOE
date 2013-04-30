#ifndef RENDERRECORDER_H
#define RENDERRECORDER_H

#include "moeobject.h"

#include <QVariantList>
#include <QTransform>
#include <QPainter>
#include <QDebug>

struct RenderInstruction {
    enum Type {
        FillRect,
        FillPolygon,

        DrawRect,
        DrawLine,
        DrawPolygon,

        UpdatePen,
        UpdateBrush,
        UpdateOpacity,
        UpdateClipRect,

        RenderImage
    } type;

    QVariantList arguments;
};

typedef QVector<RenderInstruction> RenderInstructions;

class RenderRecorder : public MoeObject
{
    Q_OBJECT
public:
    inline RenderRecorder() {}

    inline static void paint(RenderInstructions instructions, QPainter& p) {
        foreach(RenderInstruction inst, instructions) {
            switch(inst.type){
            case RenderInstruction::FillRect:
                p.fillRect(inst.arguments.at(0).toRect(), (QRgb)inst.arguments.at(1).toUInt());
                break;

            default:
                qWarning() << "Unhandled Recorder Instruction" << inst.type;
            }
        }
    }

    inline RenderInstructions instructions() const{return _instructions;}

public slots:
    inline void fillRect(QRect rect, QRgb color){
        RenderInstruction instruction;
        instruction.type = RenderInstruction::FillRect;
        instruction.arguments.append(rect);
        instruction.arguments.append((unsigned int)color);
        _instructions.append(instruction);
    }

    inline void drawRect(QRect){
    }

    inline void setPen(QRgb){
    }

    inline void setBrush(QRgb){

    }

private:
    RenderInstructions _instructions;
    QTransform _transform;
};

#endif // RENDERRECORDER_H
