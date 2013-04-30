#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moegraphicscontainer.h"
#include "renderrecorder.h"

#include <QSharedPointer>
#include <QWeakPointer>
#include <QScriptValue>
#include <QTimer>

class MoeEngineView;
class MoeEngineGraphicsObject;

class MoeGraphicsSurface : public MoeGraphicsContainer
{
    Q_OBJECT
public:
    MoeGraphicsSurface();
    void render(RenderRecorder* =0, QRect =QRect());

signals:
    void renderReady(RenderInstructions);

public slots:
    void updateSize(QSize);

protected slots:
    void renderNow();

protected:
    void repaintImpl();

private:
    QTimer paintTimer;
};

#endif // MOEGRAPHICSSURFACE_H
