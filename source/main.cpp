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

    MoeEngine* engine = new MoeEngine();
    if(!isHeadless)
        CrashDialog::init(engine);
    engine->startWithArguments(parser.toMap());

    return app->exec();
}
