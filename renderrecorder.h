#ifndef RENDERRECORDER_H
#define RENDERRECORDER_H

#include "moeobject.h"

#include <QVariantList>
#include <QTransform>
#include <QPainter>
#include <QDebug>

struct RenderInstruction {
    enum Type {
        Initialize,

        FillRect,
        FillPolygon,

        DrawRect,
        DrawLine,
        DrawPolygon,
        DrawText,

        UpdatePen,
        UpdateBrush,
        UpdateOpacity,
        UpdateClipRect,
        UpdateTransform,
        UpdateFont,

        RenderImage
    } type;

    QVariantList arguments;
};

typedef QVector<RenderInstruction> RenderInstructions;

class RenderRecorder : public MoeObject
{
    Q_OBJECT
public:
    explicit inline RenderRecorder(QRect rect) {
        clipRect = rect;
        cbrush = qRgba(0,0,0,0);
        cpen = qRgb(0,0,0);
    }

    inline static void paint(RenderInstructions instructions, QPainter& p, QRect clipRect) {
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);

        foreach(RenderInstruction inst, instructions) {
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
                    p.setClipRect(clipRect, Qt::ReplaceClip);
                else
                    p.setClipRect(inst.arguments.first().toRect(), Qt::ReplaceClip);
                break;

            case RenderInstruction::UpdateFont:
                p.setFont(QFont(inst.arguments.at(0).toString(), inst.arguments.at(1).toInt()));
                break;

            default:
                qWarning() << "Unhandled Recorder Instruction" << inst.type;
            }
        }
    }

    inline RenderInstructions instructions() const{return _instructions;}

public slots:
    inline void fillRect(QRect rect, QRgb color){
        rect = transform.mapRect(rect);

        if(!clipRect.intersects(rect))
            return;
        rect &= clipRect;

        noClipRect();
        RenderInstruction instruction;
        instruction.type = RenderInstruction::FillRect;
        instruction.arguments.append(rect);
        instruction.arguments.append((unsigned int)color);
        _instructions.append(instruction);
    }

    inline void drawText(QRect rect, QString text) {
        rect = transform.mapRect(rect);

        if(!clipRect.intersects(rect))
            return;

        updatePen();
        updateFont();
        if(clipRect.contains(rect))
            noClipRect();
        else
            requireClipRect();

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawText;
        instruction.arguments.append(rect);
        instruction.arguments.append(text);
        _instructions.append(instruction);
    }

    inline void drawRect(QRect rect){
        rect = transform.mapRect(rect);
        bool hasBorder = qAlpha(pen) > 0;

        if(!clipRect.intersects(rect))
            return;

        updatePen();
        updateBrush();
        if(clipRect.contains(rect))
            noClipRect();
        else if(!hasBorder) {
            rect &= clipRect;
            noClipRect();
        } else
            requireClipRect();

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawRect;
        if(hasBorder)
            rect = QRect(rect.topLeft(), rect.size() - QSize(1, 1));
        instruction.arguments.append(rect);
        _instructions.append(instruction);
    }

    inline void drawLine(QPoint p1, QPoint p2){
        p1 = transform.map(p1);
        p2 = transform.map(p2);
        QRect rect(p1, p2);

        if(!clipRect.intersects(rect))
            return;

        updatePen();
        if(clipRect.contains(rect))
            noClipRect();
        else
            requireClipRect();

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawLine;
        instruction.arguments.append(p1);
        instruction.arguments.append(p2);
        _instructions.append(instruction);
    }

    inline void setPen(QRgb c){
        pen = c;
    }

    inline void setBrush(QRgb c){
        brush = c;
    }

    inline void setFont(QFont fon){
        font = fon;
    }

public:
    inline void pushClipRect(QRect rect) {
        clipRectStack.append(clipRect);
        clipRect &= transform.mapRect(rect);
    }

    inline void popClipRect() {
        clipRect = clipRectStack.takeLast();
    }

    inline void pushTransform(QTransform trans) {
        transformStack.append(transform);
        transform *= trans;
    }

    inline void popTransform() {
        transform = transformStack.takeLast();
    }

protected:
    inline void updatePen() {
        if(cpen != pen) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdatePen;
            if(qAlpha(pen) > 0)
                instruction.arguments.append((unsigned int)pen);
            _instructions.append(instruction);
            cpen = pen;
        }
    }

    inline void updateBrush() {
        if(cbrush != brush) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateBrush;
            if(qAlpha(brush) > 0)
                instruction.arguments.append((unsigned int)brush);
            _instructions.append(instruction);
            cbrush = brush;
        }
    }

    inline void updateFont() {
        if(cfont != font) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateFont;
            instruction.arguments.append(font.family());
            instruction.arguments.append(font.pointSize());
            _instructions.append(instruction);
            cfont = font;
        }
    }

    inline void requireClipRect() {
        if(cClipRect != clipRect) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateClipRect;
            instruction.arguments.append(clipRect);
            _instructions.append(instruction);
            cClipRect = clipRect;
        }
    }

    inline void noClipRect() {
        if(!cClipRect.isNull()) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateClipRect;
            _instructions.append(instruction);
            cClipRect = QRect();
        }
    }

private:
    QRect clipRect;
    QList<QRect> clipRectStack;

    QTransform transform;
    QList<QTransform> transformStack;

    RenderInstructions _instructions;
    QTransform _transform;

    QFont font, cfont;
    QRgb pen, brush;
    QRgb cpen, cbrush;
    QRect cClipRect;
};

#endif // RENDERRECORDER_H
