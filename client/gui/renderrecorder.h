#ifndef RENDERRECORDER_H
#define RENDERRECORDER_H

#include <core/moeobject.h>
#include "renderbuffer.h"
#include "renderinstruction.h"

#include <QVariantList>
#include <QTransform>
#include <QPainter>

class MoeAbstractGraphicsSurface;

class RenderRecorder : public MoeObject
{
    Q_OBJECT
    Q_PROPERTY(QColor pen READ pen WRITE setPen)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)

public:
    explicit inline RenderRecorder(MoeAbstractGraphicsSurface* s, QRect rect) {
        surface = s;
        cbrush = QColor(Qt::darkMagenta);
        _brush = cbrush;
        cPenThick = 1;
        penThick = cPenThick;
        cpen = QColor(0,0,0);
        _pen = cpen;
        clipRect = rect;
        copacity = 1;
        opacity = 1;
    }

    inline QColor pen() const{return _pen;}
    inline QBrush brush() const{return _brush;}

    inline RenderInstructions instructions() const{return _instructions;}

public slots:
    inline void fillRect(QRectF rect, QColor color, qreal borderRadius = 0){
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
        instruction.arguments.append((unsigned int)color.rgba());
        _instructions.append(instruction);
    }

    inline void drawBuffer(RenderBuffer* obj, QRectF rect) {
        if(!obj || !clipRect.intersects(transform.mapRect(rect)))
            return;

        updateOpacity();
        requireClipRect();
        requireTransform();
        RenderInstruction instruction;
        instruction.type = RenderInstruction::RenderBuffer;
        instruction.arguments.append(obj->id());
        instruction.arguments.append(rect);
        _instructions.append(instruction);
        if(obj->updateSurfaceVersion(surface)) {
            instruction.type = RenderInstruction::BufferLoadImage;
            instruction.arguments.removeLast();
            instruction.arguments.append(obj->data());
            _instructions.prepend(instruction);
        }
    }

    inline void drawBuffer(QObject* obj, QRectF rect) {
        RenderBuffer* renderBuf = RenderBuffer::instance(obj, false);
        if(renderBuf)
            drawBuffer(renderBuf, rect);
    }

    inline void drawText(QRectF rect, QString text) {
        QRectF targetRect = transform.mapRect(rect);

        if(!clipRect.intersects(targetRect))
            return;

        updatePen();
        updateFont();
        updateOpacity();
        if(transform.isScaling()) {
            requireTransform();
            targetRect = rect;
        } else {
            noTransform();

            if(clipRect.contains(targetRect))
                noClipRect();
            else {
                QRectF oldClip = QRectF(clipRect.x()-2,clipRect.y()-2,
                                      clipRect.width()+4,clipRect.height()+4);
                requireClipRect();
                clipRect = oldClip;
            }
        }

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawText;
        instruction.arguments.append(targetRect);
        instruction.arguments.append(text);
        _instructions.append(instruction);

    }

    inline void drawRect(QRectF rect, qreal radius = 0){
        rect = transform.mapRect(rect);
        bool hasBorder = _pen.alpha() > 0;

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
            rect = QRectF(rect.topLeft() + QPointF(penThick, penThick),
                         rect.size() - QSize(penThick*2, penThick*2));
        instruction.arguments.append(rect);
        if(radius > 0)
            instruction.arguments.append(radius);
        _instructions.append(instruction);
    }

    inline void drawPixel(QPointF p) {
        p = transform.map(p);

        if(!clipRect.contains(p))
            return;

        updatePen();
        noTransform();
        updateOpacity();

        RenderInstruction instruction;
        instruction.type = RenderInstruction::DrawPixel;
        instruction.arguments.append(p);
        _instructions.append(instruction);
    }

    inline void drawLine(QPointF p1, QPointF p2){
        p1 = transform.map(p1);
        p2 = transform.map(p2);
        QRectF rect(p1, p2);

        if(!clipRect.intersects(rect))
            return;

        updatePen();
        noTransform();
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

    inline void setPenThickness(int thick) {
        if(thick > 0)
            penThick = thick;
    }

    inline void setPen(QColor c, int thickness =1){
        setPenColor(c);
        setPenThickness(thickness);
    }

    inline void setPenColor(QColor c){
        _pen = c;
    }

    inline void setBrush(QBrush c){
        _brush = c;
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
        if(cpen != _pen || penThick != cPenThick) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdatePen;
            if(_pen.alpha() > 0) {
                instruction.arguments.append((unsigned int)_pen.rgba());
                instruction.arguments.append(penThick);
            }
            _instructions.append(instruction);
            cPenThick = penThick;
            cpen = _pen;
        }
    }

    inline void updateBrush() {
        if(cbrush != _brush) {
            RenderInstruction instruction;
            instruction.type = RenderInstruction::UpdateBrush;
            if(_brush.color().alpha() > 0 || _brush.gradient())
                instruction.arguments.append(_brush);
            _instructions.append(instruction);
            cbrush = _brush;
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
            cClipRect = QRectF();
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
        if(cTransform.type()) {
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
    QRectF clipRect;
    QList<QRectF> clipRectStack;

    qreal opacity, copacity;
    QList<qreal> opacityStack;

    QTransform transform;
    QList<QTransform> transformStack;

    RenderInstructions _instructions;
    QTransform _transform;

    QRectF cClipRect;
    QColor _pen, cpen;
    QFont font, cfont;
    QBrush _brush, cbrush;
    QTransform cTransform;
    int penThick, cPenThick;
    MoeAbstractGraphicsSurface* surface;
};

#endif // RENDERRECORDER_H
