#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T18:17:55
#
#-------------------------------------------------

QT       += core gui network script opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MoeGameLauncher
TEMPLATE = app

QMAKE_CXXFLAGS+="-std=c++0x"

SOURCES += debug/crashdialog.cpp \
    main.cpp \
	gui/moeabstractgraphicssurface.cpp \
    gui/moegraphicscontainer.cpp \
    gui/moegraphicsimage.cpp \
    gui/moegraphicsobject.cpp \
    gui/moegraphicssurface.cpp \
	gui/moegraphicstext.cpp \
	gui/widgetsurfacebackend.cpp \
	moeclientengine.cpp \
	clientscriptconverters.cpp \
    opengl/moeglgraphicsview.cpp \
    opengl/moeglslshaderprogram.cpp \
    opengl/moeglvertexmodel.cpp \
    opengl/moeglcubemodel.cpp \
    opengl/moeglscene.cpp \
    gui/qpaintersurfacebackend.cpp

HEADERS  += gui/moeabstractgraphicssurface.h \
    gui/abstractsurfacebackend.h \
	debug/crashdialog.h \
    gui/moegraphicscontainer.h \
    gui/moegraphicsimage.h \
    gui/moegraphicsobject.h \
    gui/moegraphicssurface.h \
	gui/moegraphicstext.h \
    gui/qpaintersurfacebackend.h \
    gui/renderbuffer.h \
    gui/renderinstruction.h \
	gui/renderrecorder.h \
	gui/widgetsurfacebackend.h \
	moeclientengine.h \
    opengl/moeglgraphicsview.h \
    opengl/moeglvertexmodel.h \
    opengl/moeglslshaderprogram.h \
    opengl/moeglcubemodel.h \
    opengl/moeglscene.h \
    opengl/gltypes.h

FORMS    += debug/crashdialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -lMoeGameEngine0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -lMoeGameEngine0
else:unix: LIBS += -L$$OUT_PWD/../lib/ -lMoeGameEngine

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/release/ -lModularCore0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../extern/ModularCore/debug/ -lModularCore0
else:unix: LIBS += -L$$OUT_PWD/../extern/ModularCore/ -lModularCore

INCLUDEPATH += $$PWD/../extern/ModularCore
DEPENDPATH += $$PWD/../extern/ModularCore

include(../extern/GitProjectVersionQt/version.pri)
include(../data/resources.pri)
