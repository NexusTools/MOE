#include "moe.h"
#include "qargumentparser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QArgumentParser parser(argc, argv);
    QApplication a(argc, argv);

    //MOE w;
    //w.show();
    
    return a.exec();
}
