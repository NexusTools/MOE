#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include "moeabstractgraphicssurface.h"
#include "widgetsurfacebackend.h"

class MoeGraphicsSurface : public MoeAbstractGraphicsSurface
{
    Q_OBJECT
    friend class MoeEngineWidget;
    friend class MoeEngineGLWidget;

    Q_ENUMS(BackendWidgetType)
public:
    enum BackendWidgetType {
        Auto = 0x0,
        MainWindow,
        Dialog,
        Widget,
        GLWidget
    };

    Q_INVOKABLE inline explicit MoeGraphicsSurface(QString title, QSize size, BackendWidgetType type =Auto, MoeGraphicsSurface* parent =0)
        : MoeAbstractGraphicsSurface(new WidgetSurfaceBackend(title, size, (int)type, parent ? parent->backendWidget() : 0)) {}

    BackendWidgetType backendWidgetType() {return (BackendWidgetType)((WidgetSurfaceBackend*)backend())->type();}
    QWidget* backendWidget() {return ((WidgetSurfaceBackend*)backend())->widget();}

private:
    inline explicit MoeGraphicsSurface(QWidget* widget) : MoeAbstractGraphicsSurface(new WidgetSurfaceBackend(widget)) {}
    void initWidget(QWidget*);

    BackendWidgetType widgetType;
    QWidget* _widget;

};

#endif // MOEGRAPHICSSURFACE_H
