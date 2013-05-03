#ifndef MOEENGINEVIEW_H
#define MOEENGINEVIEW_H

#include <QWidget>
#include <QSharedPointer>

#include "moeengine.h"
#include "moegraphicssurface.h"

class MoeEngineView : public QWidget
{
    Q_OBJECT
public:
    explicit MoeEngineView(QWidget *parent = 0);

    void inject(QString key, QObject* obj);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void keyPressEvent(QKeyEvent *);

public slots:
    void start();
    void quit();

protected slots:
    void renderInstructions(RenderInstructions, QRect, QSize);
    inline void repaintRect(){repaint(_repaintRect);}
    void setCursor(QCursor);

signals:
    void sizeChanged(QSize);
    void repaintSurface();
    void readyForFrame();

    void mouseMove(QPoint);
    void mousePress(QPoint,int);
    void mouseRelease(QPoint,int);

private:
    QPixmap buffer;

    MoeEngine engine;
    QPointer<MoeGraphicsSurface> surface;
    RenderInstructions storedInstructions;

    QTimer repaintTimer;
    QRect _repaintRect;
    QSize _bufferSize;
};

#endif // MOEENGINEVIEW_H
