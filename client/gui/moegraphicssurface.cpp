#include "moegraphicssurface.h"

MoeGraphicsSurface::BackendWidgetType MoeGraphicsSurface::_defaultType = MoeGraphicsSurface::MainWindow;

void MoeGraphicsSurface::hadOGLErr(QString err) {
    _oglError = err;
    qCritical() << "OpenGL Error" << err;
    emit OpenGLError(err);
}
