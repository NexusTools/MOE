#ifndef MOE_H
#define MOE_H

#include <QGraphicsView>

class MOE : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MOE(QWidget *parent = 0);

    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);

private:
};

#endif // MOE_H
