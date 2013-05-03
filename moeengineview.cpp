#include "moeengineview.h"
#include "renderrecorder.h"
#include <QMouseEvent>
#include <QKeyEvent>

MoeEngineView::MoeEngineView(QWidget *parent) :
    QWidget(parent)
{
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaintRect()));
    connect(&engine, SIGNAL(stopped()), this, SLOT(repaint()));

    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_OpaquePaintEvent);

    repaintTimer.setSingleShot(true);
    repaintTimer.setInterval(0);

    setMouseTracking(true);
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

void MoeEngineView::keyPressEvent(QKeyEvent * ev){
    if(ev->key() == Qt::Key_Space && !engine.isRunning())
        start();
}

void MoeEngineView::paintEvent(QPaintEvent *){
    QPainter p;

    if(engine.isRunning()) {
        if(!storedInstructions.isEmpty()) {
            if(buffer.size() != _bufferSize) {
                qDebug() << "Updating Buffer Size" << _bufferSize;
                QPixmap oldBuffer = buffer;
                buffer = QPixmap(_bufferSize);
                if(!oldBuffer.isNull()) {
                    p.begin(&buffer);
                    p.drawPixmap(QRect(QPoint(0,0),_bufferSize),oldBuffer);
                    p.end();
                }
            }

            p.begin(&buffer);
            RenderRecorder::paint(storedInstructions, p, _repaintRect);
            storedInstructions.clear();
            p.end();
        }
        emit readyForFrame();
    }

    p.begin(this);
    if(buffer.isNull())
        p.fillRect(QRect(QPoint(0,0),size()), Qt::darkMagenta);
    else
        p.drawPixmap(QRect(QPoint(0,0),size()), buffer);

    // TODO: Draw Overlay
    p.end();
}

void MoeEngineView::resizeEvent(QResizeEvent *)
{
    emit sizeChanged(size());
}

void MoeEngineView::inject(QString key, QObject *obj)
{
    engine.inject(key, obj);
}

void MoeEngineView::start()
{
    if(surface.data())
        return;

    _bufferSize = size();
    engine.makeCurrent();
    surface = new MoeGraphicsSurface(size());
    connect(this, SIGNAL(repaintSurface()), surface.data(), SLOT(repaint()), Qt::QueuedConnection);
    connect(this, SIGNAL(readyForFrame()), surface.data(), SLOT(prepareNextFrame()), Qt::QueuedConnection);
    connect(this, SIGNAL(sizeChanged(QSize)), surface.data(), SLOT(updateSize(QSize)), Qt::QueuedConnection);

    connect(this, SIGNAL(mouseMove(QPoint)), surface.data(), SLOT(mouseMove(QPoint)), Qt::QueuedConnection);
    connect(this, SIGNAL(mousePress(QPoint,int)), surface.data(), SLOT(mousePress(QPoint,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(mouseRelease(QPoint,int)), surface.data(), SLOT(mouseRelease(QPoint,int)), Qt::QueuedConnection);

    connect(surface.data(), SIGNAL(cursorChanged(QCursor)), this, SLOT(setCursor(QCursor)), Qt::QueuedConnection);
    connect(surface.data(), SIGNAL(titleChanged(QString)), this, SLOT(setWindowTitle(QString)), Qt::QueuedConnection);
    connect(surface.data(), SIGNAL(renderReady(RenderInstructions, QRect, QSize)), this, SLOT(renderInstructions(RenderInstructions, QRect, QSize)), Qt::QueuedConnection);
    connect(&engine, SIGNAL(stopped()), surface.data(), SLOT(deleteLater()));

    emit readyForFrame();
    engine.inject("surface", surface.data());
    engine.start();
}

void MoeEngineView::quit()
{
    engine.quit();
}

void MoeEngineView::setCursor(QCursor cur) {
    QWidget::setCursor(cur);
}

void MoeEngineView::renderInstructions(RenderInstructions instructions, QRect repaintRect, QSize size)
{
    _repaintRect = repaintRect;
    _bufferSize = size;
    storedInstructions = instructions;
    repaintTimer.start();
}
