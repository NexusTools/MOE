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

    MoeEngine::registerQDebugHandler();
    if(parser.contains("stress")) {
        int count = parser.value("stress", 15).toInt();
        QList<MoeEngine*> engines;
        forever {
            {
                QList<MoeEngine*>::iterator iter = engines.begin();
                while(iter != engines.end()) {
                    if((*iter)->isFinished()) {
                        (*iter)->deleteLater();
                        iter = engines.erase(iter);
                    } else
                        iter++;
                }
            }
            while(engines.size() < count) {
                MoeEngine* engine = new MoeEngine();
                engine->startContent("qrc:/examples/crash/");
                engines << engine;
            }
        }
    } else {
        MoeEngine* engine = new MoeEngine();
        if(!isHeadless)
            CrashDialog::init(engine);
        engine->startWithArguments(parser.toMap());
    }

    return app->exec();
}
