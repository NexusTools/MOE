#include "widgetsurfacebackend.h"
#include "moegraphicssurface.h"
#include "../core/moeengine.h"

#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>
#include <QDialog>
#include <QWidget>


WidgetSurfaceBackend::WidgetSurfaceBackend(QString title, QSize size, int type, QWidget* parent) : QPainterSurfaceBackend(QRect())
{
    _widget = 0;
    _type = type;
    _ownsWidget = true;
    static QMetaMethod createWidgetMethod = metaObject()->method(metaObject()->indexOfMethod("createWidget(QString,QSize,int,QWidget*)"));
    createWidgetMethod.invoke(this, Qt::QueuedConnection, Q_ARG(QString,title), Q_ARG(QSize,size), Q_ARG(int,_type), Q_ARG(QWidget*,parent));
}

WidgetSurfaceBackend::WidgetSurfaceBackend(QWidget* widget) : QPainterSurfaceBackend(QRect())
{
    static QMetaMethod initWidgetMethod = metaObject()->method(metaObject()->indexOfMethod("initWidget(QWidget*)"));
    initWidgetMethod.invoke(this, Qt::QueuedConnection, Q_ARG(QWidget*,widget));
    _ownsWidget = false;
    _widget = 0;
    _type = 0;
}

WidgetSurfaceBackend::~WidgetSurfaceBackend() {
    if(_ownsWidget && _widget) {
        qDebug() << "Caching surface widget of type" << _type;
        _widget->setWindowTitle("Changing Content (MOE Game Engine)");
        _widget->setObjectName("<cached surface>");
        if(_widget->parentWidget() && _widget->isWindow())
            _widget->hide();
        else {
            QTimer* hideTimer = new QTimer();
            hideTimer->setInterval(500);
            hideTimer->setSingleShot(true);
            connect(hideTimer, SIGNAL(timeout()), _widget, SLOT(hide()));
            connect(_widget, SIGNAL(destroyed()), hideTimer, SLOT(deleteLater()));
            connect(_widget, SIGNAL(objectNameChanged(QString)), hideTimer, SLOT(deleteLater()));
            connect(hideTimer, SIGNAL(timeout()), hideTimer, SLOT(deleteLater()), Qt::QueuedConnection);
            hideTimer->start();
        }

        _surfaceCache.localData().insert(_type, _widget);
    }
}

bool WidgetSurfaceBackend::eventFilter(QObject * obj, QEvent * event) {
    if(obj == _widget) {
        switch(event->type()) {
            case QEvent::Close:
                deleteLater();
            break;

            case QEvent::Paint:
            {
                QPainter p;
                if(hasPendingInstructions()) {
                    if(buffer.size() != bufferSize()) {
                        //qDebug() << "Resized Buffer" << bufferSize();
                        buffer = QPixmap(bufferSize());
                    }

                    p.begin(&buffer);
                    p.setRenderHint(QPainter::Antialiasing);
                    paint(p);
                    p.end();
                }

                p.begin(widget());
                QRect geom(QPoint(0,0),widget()->size());
                if(buffer.isNull())
                    p.fillRect(geom, Qt::darkMagenta);
                else
                    p.drawPixmap(geom, buffer);
                p.end();

                markReadyForFrame();
            }
            return true;

            case QEvent::KeyPress:
            {
                QKeyEvent* keyEv = (QKeyEvent*)event;
                if(keyEv->count())
                    foreach(char c, keyEv->text().toUtf8())
                        emit keyType(c);

                if(keyEv->isAutoRepeat())
                    break;
                emit keyPress(keyEv->key());
                break;
            }

            case QEvent::KeyRelease:
            {
                QKeyEvent* keyEv = (QKeyEvent*)event;
                if(keyEv->isAutoRepeat())
                    break;
                emit keyRelease(keyEv->key());
                break;
            }

            case QEvent::Leave:
                emit mouseMove(QPoint(-1,-1));
            break;

            case QEvent::Move:
            case QEvent::Resize:
                updateGeometry(_widget->geometry());
            break;

            case QEvent::FocusOut:
            case QEvent::WindowDeactivate:
            {
                emit resetFocus();
                break;
            }

            case QEvent::WindowStateChange:
            {
                if(!_widget->isMinimized())
                    break;
            }

            case QEvent::Hide:
            {
                emit disconnected();
                break;
            }

            case QEvent::MouseMove:
                emit mouseMove(((QMouseEvent*)event)->pos());
            break;

            case QEvent::MouseButtonPress:
                emit mousePress(((QMouseEvent*)event)->pos(), ((QMouseEvent*)event)->buttons());
            break;

            case QEvent::MouseButtonRelease:
                emit mouseRelease(((QMouseEvent*)event)->pos(), ((QMouseEvent*)event)->buttons());
            break;

            default:
            break;
        }
    }

    return false;
}

void WidgetSurfaceBackend::createWidget(QString title, QSize size, int type, QWidget *parent) {
    QWidget* widget = _surfaceCache.localData().take(type);
    if(widget) {
        qDebug() << "Reusing cached widget surface";

        _widget = widget;
        repaintTimer.setInterval(0);
        repaintTimer.setSingleShot(true);
        connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaintRect()));

        widget->installEventFilter(this);

        if(parent)
            widget->hide();
    } else {
        qDebug() << "Creating new surface widget of type" << type;

        switch((MoeGraphicsSurface::BackendWidgetType)type) {
            case MoeGraphicsSurface::Widget:
                widget = new QWidget();
            break;

            case MoeGraphicsSurface::Dialog:
                widget = new QDialog();
            break;

            case MoeGraphicsSurface::GLWidget:
                widget = new QGLWidget();
            break;

            default:
                widget = new QMainWindow();
            break;
        }

        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->resize(size);
    }

    widget->setObjectName("MoeGraphicsSurfaceWidget");
    widget->setWindowTitle(title);
    widget->setParent(parent);
    initWidget(widget);
}

void WidgetSurfaceBackend::initWidget(QWidget* widget) {
    //qDebug() << "Initializing Widget for Surface" << widget;

    _widget = widget;
    repaintTimer.setInterval(0);
    repaintTimer.setSingleShot(true);
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaintRect()));
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->installEventFilter(this);
    widget->setMouseTracking(true);

    if(widget->isVisible()) {
        qDebug() << "Widget is already visible";
        updateGeometry(widget->geometry());
        repaintTimer.start();
    } else
        widget->show();
}

void WidgetSurfaceBackend::repaintRect() {
     if(_repaintRect.isNull())
         _widget->repaint();
     else
         _widget->repaint(_repaintRect);
     _repaintRect = QRect();
}


QThreadStorage<SurfaceCache> WidgetSurfaceBackend::_surfaceCache;
