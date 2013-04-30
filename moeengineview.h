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

public slots:
    void start();
    void quit();

protected slots:
    void renderInstructions(RenderInstructions);

signals:
    void sizeChanged(QSize);
    void repaintSurface();
    void readyForFrame();

private:
    QPixmap buffer;

    MoeEngine engine;
    MoeGraphicsSurface surface;
    RenderInstructions storedInstructions;

    QTimer repaintTimer;
};

#endif // MOEENGINEVIEW_H
