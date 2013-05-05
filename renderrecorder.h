#ifndef RENDERRECORDER_H
#define RENDERRECORDER_H

#include "moeobject.h"
#include "renderbuffer.h"
#include "renderinstruction.h"

#include <QVariantList>
#include <QTransform>
#include <QPainter>

class RenderRecorder : public MoeObject
{
    Q_OBJECT
public:
    explicit inline RenderRecorder(QRect rect) {
        cbrush = qRgba(0,0,0,0);
        brush = qRgba(0,0,0,0);
        cpen = qRgb(0,0,0);
        pen = qRgb(0,0,0);
        clipRect = rect;
        copacity = 1;
        opacity = 1;
    }

    inline RenderInstructions instructions() const{return _instructions;}

public slots:
    inline void fillRect(QRect rect, QRgb color, qreal borderRadius = 0){
        if(borderRadius > 0) {
            setPen(qRgba(0,0,0,0));
            setBrush(color);
            drawRect(rect, borderRadius);
            return;
        }

        rect = transform.mapRect(rect);

        if(!clipRect.intersects(rect))
            return;
        rect &= clipRect;

        noTransform();
        noClipRect();
        updateOpacity();
        RenderInstruction instruction;
        instruction.type = RenderInstruction::FillRect;
        instruction.arguments.append(rect);
        instruction.arguments.append((unsigned int)color);
        _instructions.append(instruction);
    }

    inline void drawText(QRect rect, QString text) {
        QRect targetRect = transform.mapRect(rect);

        if(!clipRect.intersects(targetRect))
            return;

        if(transform.isScaling() || transform.isRotating()) {
            requireTransform();
            targetRect = rect;
        } else {
            noTransform();

            updatePen();
            updateFont();
            updateOpacity();
            if(clipRect.contains(targetRect))
                noClipRect();
            else
                requireClipRect();
        }

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawText;
        instruction.arguments.append(targetRect);
        instruction.arguments.append(text);
        _instructions.append(instruction);

    }

    inline void drawRect(QRect rect, qreal radius = 0){
        rect = transform.mapRect(rect);
        bool hasBorder = qAlpha(pen) > 0;

        if(!clipRect.intersects(rect))
            return;

        noTransform();
        updatePen();
        updateBrush();
        updateOpacity();
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
        if(radius > 0)
            instruction.arguments.append(radius);
        _instructions.append(instruction);
    }

    inline void drawLine(QPoint p1, QPoint p2){
        p1 = transform.map(p1);
        p2 = transform.map(p2);
        QRect rect(p1, p2);

        if(!clipRect.intersects(rect))
            return;

        noTransform();
        updatePen();
        updateOpacity();
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

    inline void pushOpacity(qreal opac) {
        opacityStack.append(opacity);
        opacity *= opac;
    }

    inline void popOpacity() {
        opacity = opacityStack.takeLast();
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

    inline void requireTransform() {
        if(cTransform != transform) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateTransform;
            instruction.arguments.append(transform);
            _instructions.append(instruction);
            cTransform = transform;
        }
    }

    inline void noTransform() {
        if(transform.type()) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateTransform;
            _instructions.append(instruction);
            cTransform.reset();
        }
    }

    inline void updateOpacity() {
        if(copacity != opacity) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateOpacity;
            instruction.arguments.append(opacity);
            _instructions.append(instruction);
            copacity = opacity;
        }
    }

private:
    QRect clipRect;
    QList<QRect> clipRectStack;

    qreal opacity, copacity;
    QList<qreal> opacityStack;

    QTransform transform;
    QList<QTransform> transformStack;

    RenderInstructions _instructions;
    QTransform _transform;

    QFont font, cfont;
    QRgb pen, brush;
    QRgb cpen, cbrush;
    QRect cClipRect;
    QTransform cTransform;
};

#endif // RENDERRECORDER_H
