#include "moeengineview.h"

MoeEngineView::MoeEngineView(QWidget *parent) :
    QWidget(parent)
{
    surface = new MoeGraphicsSurface();
    engine = new MoeEngine();
}

void MoeEngineView::inject(QString key, QObject *obj)
{
    engine->inject(key, obj);
}

void MoeEngineView::start()
{
    engine->inject("surface", surface);
    engine->start();
}

void MoeEngineView::quit()
{
    engine->quit();
}

void MoeEngineView::renderInstructions()
{

}
