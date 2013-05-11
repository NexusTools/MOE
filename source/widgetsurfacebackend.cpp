#include "widgetsurfacebackend.h"
#include "moegraphicssurface.h"
#include "moeengine.h"

#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>
#include <QDialog>
#include <QWidget>


WidgetSurfaceBackend::WidgetSurfaceBackend(QString title, QSize size, int type, QWidget* parent) : QPainterSurfaceBackend(QRect())
{
    _type = type;
    if(_type == 0)
        _type = MoeGraphicsSurface::Widget;
    static QMetaMethod createWidgetMethod = metaObject()->method(metaObject()->indexOfMethod("createWidget(QString,QSize,int,QWidget*)"));
    createWidgetMethod.invoke(this, Qt::QueuedConnection, Q_ARG(QString,title), Q_ARG(QSize,size), Q_ARG(int,_type), Q_ARG(QWidget*,parent));
}

WidgetSurfaceBackend::WidgetSurfaceBackend(QWidget* widget) : QPainterSurfaceBackend(QRect())
{
    static QMetaMethod initWidgetMethod = metaObject()->method(metaObject()->indexOfMethod("initWidget(QWidget*)"));
    initWidgetMethod.invoke(this, Qt::QueuedConnection, Q_ARG(QWidget*,widget));
    _type = 0;
}

bool WidgetSurfaceBackend::eventFilter(QObject * obj, QEvent * event) {
    if(obj == _widget) {
        //static QMetaEnum typeEnum = QEvent::staticMetaObject.enumerator(QEvent::staticMetaObject.indexOfEnumerator("Type"));
        ////qDebug() << "Unhandled Event" << event->type() << typeEnum.key(event->type());

        switch(event->type()) {
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

                    emit readyForFrame();
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
    QWidget* widget;
    //qDebug() << "Creating" << type << "Widget";
    switch((MoeGraphicsSurface::BackendWidgetType)type) {
        case MoeGraphicsSurface::MainWindow:
            widget = new QMainWindow();
            break;

        case MoeGraphicsSurface::Dialog:
            widget = new QDialog();
            break;

        //case MoeGraphicsSurface::GLWidget:
        //    widget = new QGLWidget();
        //    break;

        default:
            widget = new QWidget();
            break;
    }

    connect(this, SIGNAL(destroyed()), widget, SLOT(deleteLater()));
    widget->setParent(parent);
    widget->setWindowTitle(title);
    widget->resize(size);
    widget->show();

    initWidget(widget);
}

void WidgetSurfaceBackend::initWidget(QWidget* widget) {
    //qDebug() << "Initializing Widget for Surface" << widget;

    _widget = widget;
    repaintTimer.setInterval(0);
    repaintTimer.setSingleShot(true);
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaintRect()));
    updateGeometry(widget->geometry());
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->installEventFilter(this);
    widget->setMouseTracking(true);
    widget->repaint();
}
