#include "moe.h"
#include "qargumentparser.h"
#include "moeengineview.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QArgumentParser parser(argc, argv);
    QApplication a(argc, argv);

    MoeEngineView* engine = new MoeEngineView();
    engine->inject("args", &parser);
    engine->show();
    engine->start();

    return a.exec();
}
