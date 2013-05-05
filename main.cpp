#include "qargumentparser.h"
#include "moeengine.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

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
    if(isHeadless) {
        QObject::connect(engine, &MoeEngine::crashed, [=] (QString error) {
            QMessageBox* messageBox = new QMessageBox();
            messageBox->setText(error);
            messageBox->setWindowTitle("Engine Crashed");
            messageBox->exec();
            qApp->quit();
        });
    } else
        QObject::connect(engine, &MoeEngine::crashed, [=] (QString error) {
            qDebug() << error;
            qApp->quit();
        });
    QObject::connect(engine, SIGNAL(crashed(QString)), qApp, SLOT(quit()), Qt::QueuedConnection);
    engine->startContent(QString(":/data/%1/").arg(parser.value("content", "content-select").toString()));

    return app->exec();
}
