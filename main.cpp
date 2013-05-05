#include "qargumentparser.h"
#include "crashdialog.h"
#include "moeengine.h"

#include <QApplication>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QArgumentParser parser(argc, argv);

    if(parser.contains("remote-connect")) {
        throw "Remote Connect isn't implemented yet...";
    }

    bool isHeadless = parser.contains("headless");
    QCoreApplication* app;
    if(isHeadless)
        app = new QCoreApplication(argc, argv);
    else
        app = new QApplication(argc, argv);

    MoeEngine* engine = new MoeEngine(parser.toMap());
    if(!isHeadless)
        CrashDialog::init(engine);
    engine->startContent(QString(":/data/%1/").arg(parser.value("content", "content-select").toString()));

    return app->exec();
}
