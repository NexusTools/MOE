#include "gui/moegraphicssurface.h"
#include "core/qargumentparser.h"
#include "debug/crashdialog.h"
#include "core/moeengine.h"

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
    else {
        app = new QApplication(argc, argv);
        ((QApplication*)app)->setQuitOnLastWindowClosed(false);
        if(parser.contains("force-opengl"))
            MoeGraphicsSurface::setDefaultType(MoeGraphicsSurface::GLWidget);
    }

    MoeClientEngine::registerQDebugHandler();
    if(parser.contains("stress")) {
        int count = parser.value("stress", 15).toInt();
        QList<MoeClientEngine*> engines;
        forever {
            {
                QList<MoeClientEngine*>::iterator iter = engines.begin();
                while(iter != engines.end()) {
                    if((*iter)->isFinished()) {
                        (*iter)->deleteLater();
                        iter = engines.erase(iter);
                    } else
                        iter++;
                }
            }
            while(engines.size() < count) {
                MoeClientEngine* engine = new MoeClientEngine();
                engine->startContent("qrc:/examples/crash/");
                engines << engine;
            }
        }
    } else {
        MoeClientEngine* engine = new MoeClientEngine();
        if(!isHeadless)
            CrashDialog::init(engine);
        engine->startWithArguments(parser.toMap());
        QObject::connect(engine, SIGNAL(stopped()), app, SLOT(quit()), Qt::QueuedConnection);
    }

    return app->exec();
}
