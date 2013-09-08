#ifndef RENDERINSTRUCTION_H
#define RENDERINSTRUCTION_H

#include <QVariantList>
#include <QVector>

struct RenderInstruction {
    enum Type {
        FillRect,
        FillPolygon,

        DrawRect,
        DrawLine,
        DrawPixel,
        DrawPolygon,
        DrawText,

        UpdatePen,
        UpdateBrush,
        UpdateOpacity,
        UpdateClipRect,
        UpdateTransform,
        UpdateFont,

        RenderBuffer,
        BufferLoadImage,
        QueryBufferPixels,
        QueryBufferMetaData,
        StartBufferPaint,
        EndBufferPaint,
        DestroyBuffer,

        // OpenGL
        ResizeGLScene,
        UpdateGLMatrix,
        BeginRenderGLScene,
        FinishRenderGLScene,

        AllocateGLModel,
        RenderGLModel,
        UpdateGLModel,
        UpdateGLModelMatrix,
        DestroyGLModel
    } type;

    QVariantList arguments;
};

typedef quintptr BufferID;
typedef QVector<RenderInstruction> RenderInstructions;

#endif // RENDERINSTRUCTION_H
