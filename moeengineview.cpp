#include "moeengineview.h"
#include "renderrecorder.h"
#include <QMouseEvent>

MoeEngineView::MoeEngineView(QWidget *parent) :
    QWidget(parent)
{
    repaintTimer.setInterval(0);
    repaintTimer.setSingleShot(true);
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaintRect()));

    connect(this, SIGNAL(repaintSurface()), &surface, SLOT(repaint()), Qt::QueuedConnection);
    connect(this, SIGNAL(sizeChanged(QSize)), &surface, SLOT(updateSize(QSize)), Qt::QueuedConnection);
    connect(this, SIGNAL(readyForFrame()), &surface, SLOT(prepareNextFrame()), Qt::QueuedConnection);
    connect(this, SIGNAL(mouseMove(QPoint)), &surface, SLOT(mouseMove(QPoint)), Qt::QueuedConnection);
    connect(this, SIGNAL(mousePress(QPoint,int)), &surface, SLOT(mousePress(QPoint,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(mouseRelease(QPoint,int)), &surface, SLOT(mouseRelease(QPoint,int)), Qt::QueuedConnection);

    connect(&surface, SIGNAL(titleChanged(QString)), this, SLOT(setWindowTitle(QString)));
    connect(&surface, SIGNAL(renderReady(RenderInstructions, QRect)), this, SLOT(renderInstructions(RenderInstructions, QRect)), Qt::QueuedConnection);
}


void MoeEngineView::mouseMoveEvent(QMouseEvent * ev){
    emit mouseMove(ev->pos());
}

void MoeEngineView::mousePressEvent(QMouseEvent * ev){
    emit mousePress(ev->pos(), (int)ev->buttons());
}

void MoeEngineView::mouseReleaseEvent(QMouseEvent * ev){
    emit mouseRelease(ev->pos(), (int)ev->buttons());
}

void MoeEngineView::paintEvent(QPaintEvent *){
    QPainter p;

    if(!storedInstructions.isEmpty()) {
        p.begin(&buffer);
        RenderRecorder::paint(storedInstructions, p, _repaintRect);
        storedInstructions.clear();
        p.end();
    }

    p.begin(this);
    p.drawPixmap(QPoint(0, 0), buffer);
    p.end();

    emit readyForFrame();
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
    emit readyForFrame();
    engine.start();
}

void MoeEngineView::quit()
{
    engine.quit();
}

void MoeEngineView::renderInstructions(RenderInstructions instructions, QRect rect)
{
    _repaintRect = rect;
    storedInstructions = instructions;
    repaintTimer.start();
}
