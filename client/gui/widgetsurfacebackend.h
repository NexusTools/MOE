#ifndef SURFACEWIDGETPROVIDER_H
#define SURFACEWIDGETPROVIDER_H

#include "qpaintersurfacebackend.h"

#include <QtOpenGL/QGLFramebufferObject>
#include <QThreadStorage>
#include <QWidget>
#include <QCache>
#include <QTimer>

typedef QCache<int, QWidget> SurfaceCache;

class WidgetSurfaceBackend : public QPainterSurfaceBackend
{
    Q_OBJECT
public:
    explicit WidgetSurfaceBackend(QString title, QSize size, int type, QWidget* parent);
    explicit WidgetSurfaceBackend(QWidget*);
    virtual ~WidgetSurfaceBackend();

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
    void initWidget(QWidget*);
    void repaintRect();

private:
    int _type;

    // Buffers
    QPixmap pixmap;
    QGLFramebufferObject* glfbo, *sglfbo;

    QWidget* _widget;
    QRect _repaintRect;
    QTimer repaintTimer;
    bool _ownsWidget;

    static QThreadStorage<SurfaceCache> _surfaceCache;
};

#endif // SURFACEWIDGETPROVIDER_H
