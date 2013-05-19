#ifndef SURFACEWIDGETPROVIDER_H
#define SURFACEWIDGETPROVIDER_H

#include "qpaintersurfacebackend.h"
#include <QWidget>
#include <QTimer>

class WidgetSurfaceBackend : public QPainterSurfaceBackend
{
    Q_OBJECT
public:
    explicit WidgetSurfaceBackend(QString title, QSize size, int type, QWidget* parent);
    explicit WidgetSurfaceBackend(QWidget*);

    inline void setGeometryImpl(QRect g) {
        if(_widget->geometry() != g)
            _widget->setGeometry(g);
    }

    inline void setCursorImpl(QCursor c) {
        _widget->setCursor(c);
    }

    inline void setTitleImpl(QString s) {
        _widget->setWindowTitle(s);
    }

    inline void repaint(QRect rect) {
        _repaintRect = rect;
        repaintTimer.start();
    }

    bool eventFilter(QObject *, QEvent *);

    inline QWidget* widget() const{return _widget;}
    inline int type() const{return _type;}

protected slots:
    void createWidget(QString title, QSize size, int type, QWidget* parent);
    inline void repaintRect() {_widget->repaint(_repaintRect);}
    void initWidget(QWidget*);

private:
    int _type;
    QPixmap buffer;
    QWidget* _widget;
    QTimer repaintTimer;
    QRect _repaintRect;
};

#endif // SURFACEWIDGETPROVIDER_H
