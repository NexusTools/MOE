#ifndef RENDERINSTRUCTION_H
#define RENDERINSTRUCTION_H

#include <QVariantList>
#include <QVector>

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

        RenderBuffer,
        LoadBufferData,
        QueryBufferPixels,
        DestroyBuffer
    } type;

    QVariantList arguments;
};

typedef quintptr BufferID;
typedef QVector<RenderInstruction> RenderInstructions;

#endif // RENDERINSTRUCTION_H
