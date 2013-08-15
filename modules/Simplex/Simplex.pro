#-------------------------------------------------
#
# Project created by QtCreator 2013-08-15T07:44:14
#
#-------------------------------------------------

QT       -= gui

TARGET = Simplex
TEMPLATE = lib

DEFINES += SIMPLEX_LIBRARY

SOURCES += simplex.cpp

HEADERS += simplex.h\
        simplex_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../lib/release/ -lMoeGameEngine0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../lib/debug/ -lMoeGameEngine0
else:unix: LIBS += -L$$OUT_PWD/../../lib/ -lMoeGameEngine

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../extern/ModularCore/release/ -lModularCore0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../extern/ModularCore/debug/ -lModularCore0
else:unix: LIBS += -L$$OUT_PWD/../../extern/ModularCore/ -lModularCore

INCLUDEPATH += $$PWD/../../extern/ModularCore
DEPENDPATH += $$PWD/../../extern/ModularCore

include($$PWD/../../extern/GitProjectVersionQt/version.pri)
