QT		-= gui

QT       += core script network xml

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
	network/moeresourcerequest.h \
	network/moeurl.h \
	network/transferdelegate.h \
	core/moeengine.h \
	core/moeobject.h \
	core/moeplugin.h \
	core/moecontentplugin.h \
	core/moescriptregisters.h \
	core/qargumentparser.h \
    moe-macros.h

SOURCES += \
	core/moeengine.cpp \
	core/moeobject.cpp \
	core/moescriptregisters.cpp \
	core/qargumentparser.cpp \
	network/transferdelegate.cpp \
	network/moeresourcerequest.cpp

INCLUDEPATH += $$PWD/../extern/GitProjectVersionQt

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/release/ -lModularCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/debug/ -lModularCore
else:unix: LIBS += -L$$OUT_PWD/../extern/ModularCore/ -lModularCore

INCLUDEPATH += $$PWD/../extern/ModularCore
DEPENDPATH += $$PWD/../extern/ModularCore

include(../extern/GitProjectVersionQt/version.pri)
