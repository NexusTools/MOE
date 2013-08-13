QT		-= gui

QT       += core script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MoeGameEngine
TEMPLATE = lib

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/lib
	} else {
		target.path = /usr/lib
	}
	INSTALLS += target
}

HEADERS += \
    moeenginecore.h \
    moe-macros.h

SOURCES += \
    moeenginecore.cpp

INCLUDEPATH += $$PWD/../extern/GitProjectVersionQt

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/release/ -lModularCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/debug/ -lModularCore
else:unix: LIBS += -L$$OUT_PWD/../extern/ModularCore/ -lModularCore

INCLUDEPATH += $$PWD/../extern/ModularCore
DEPENDPATH += $$PWD/../extern/ModularCore

include(../extern/GitProjectVersionQt/version.pri)
