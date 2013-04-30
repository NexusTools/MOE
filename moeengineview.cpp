#include "moeengineview.h"
#include "renderrecorder.h"

MoeEngineView::MoeEngineView(QWidget *parent) :
    QWidget(parent)
{
    repaintTimer.setInterval(0);
    repaintTimer.setSingleShot(true);
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    connect(this, SIGNAL(repaintSurface()), &surface, SLOT(repaint()), Qt::QueuedConnection);
    connect(this, SIGNAL(sizeChanged(QSize)), &surface, SLOT(updateSize(QSize)));
    connect(&surface, SIGNAL(renderReady(RenderInstructions)), this, SLOT(renderInstructions(RenderInstructions)));
}

void MoeEngineView::paintEvent(QPaintEvent *){
    QPainter p;

    if(!storedInstructions.isEmpty()) {
        qDebug() << "Painting" << storedInstructions.size() << "Instructions";
        p.begin(&buffer);
        RenderRecorder::paint(storedInstructions, p);
        storedInstructions.clear();
        p.end();
    }

    p.begin(this);
    p.drawPixmap(QPoint(0, 0), buffer);
    p.end();

}

void MoeEngineView::resizeEvent(QResizeEvent *)
{
    QPixmap newBuffer(size());
    QPainter p(&newBuffer);
    p.fillRect(QRect(QPoint(0,0),size()),Qt::black);
    if(!buffer.isNull())
        p.drawPixmap(QPoint(0,0),buffer);

    buffer = newBuffer;
    emit sizeChanged(size());
}

void MoeEngineView::inject(QString key, QObject *obj)
{
    engine.inject(key, obj);
}

void MoeEngineView::start()
{
    engine.inject("surface", &surface);
    emit repaintSurface();
    engine.start();
}

void MoeEngineView::quit()
{
    engine.quit();
}

void MoeEngineView::renderInstructions(RenderInstructions instructions)
{
    foreach(RenderInstruction inst, instructions) {
        storedInstructions.append(inst);
    }
    repaintTimer.start();
}
