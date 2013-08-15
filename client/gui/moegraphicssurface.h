#ifndef MOEGRAPHICSSURFACE_H
#define MOEGRAPHICSSURFACE_H

#include <QDesktopServices>

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
        GLWidget,
        Dialog,
        Tools,
        Widget
    };

    Q_INVOKABLE inline explicit MoeGraphicsSurface(QString title, QSize size, BackendWidgetType type =Auto, MoeGraphicsSurface* parent =0)
        : MoeAbstractGraphicsSurface(new WidgetSurfaceBackend(title, size, type == Auto ? _defaultType : type, parent ? parent->backendWidget() : 0)) {
        connect(backend(), SIGNAL(OpenGLError(QString)), this, SLOT(hadOGLErr(QString)));
    }
    virtual ~MoeGraphicsSurface() {qDebug() << this << "surface destroyed";}

    void openUrl(QUrl url) {QDesktopServices::openUrl(url);}
    static inline void setDefaultType(BackendWidgetType type) {_defaultType = type;}
    BackendWidgetType backendWidgetType() const{return (BackendWidgetType)((WidgetSurfaceBackend*)backend())->type();}
    QWidget* backendWidget() {return ((WidgetSurfaceBackend*)backend())->widget();}

    virtual bool hasOpenGL() const{return backendWidgetType() == GLWidget;}
    virtual QString lastOpenGLError() const{return _oglError.isEmpty() ? "No Error" : _oglError;}

signals:
    void OpenGLError(QString);

protected slots:
    void hadOGLErr(QString);

private:
    inline explicit MoeGraphicsSurface(QWidget* widget) : MoeAbstractGraphicsSurface(new WidgetSurfaceBackend(widget)) {}
    void initWidget(QWidget*);

    QString _oglError;
    QWidget* _widget;

    static BackendWidgetType _defaultType;
};

#endif // MOEGRAPHICSSURFACE_H
