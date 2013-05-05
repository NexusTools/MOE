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

    if(parser.contains("example"))
        engine->startContent(QString(":/data/examples/%1/").arg(parser.value("example").toString()));
    else if(parser.contains("content") || parser.hasDefaultValue())
        engine->startContent(parser.value("content", parser.defaultValue()).toString());
    else
        engine->startContent(":/data/content-select/");

    return app->exec();
}
