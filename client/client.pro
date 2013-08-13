#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T18:17:55
#
#-------------------------------------------------

QT       += core gui network script opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MOEGameLauncher
TEMPLATE = app


QMAKE_CXXFLAGS+="-std=c++0x"

SOURCES += debug/crashdialog.cpp \
    main.cpp \
    gui/moeabstractgraphicssurface.cpp \
    core/moeengine.cpp \
    gui/moegraphicscontainer.cpp \
    gui/moegraphicsimage.cpp \
    gui/moegraphicsobject.cpp \
    gui/moegraphicssurface.cpp \
    gui/moegraphicstext.cpp \
    network/moeresourcerequest.cpp \
    core/moeobject.cpp \
    core/moescriptregisters.cpp \
    core/qargumentparser.cpp \
    network/transferdelegate.cpp \
	gui/widgetsurfacebackend.cpp

HEADERS  += gui/moeabstractgraphicssurface.h \
    gui/abstractsurfacebackend.h \
    debug/crashdialog.h \
    core/moeengine.h \
    gui/moegraphicscontainer.h \
    gui/moegraphicsimage.h \
    gui/moegraphicsobject.h \
    gui/moegraphicssurface.h \
    gui/moegraphicstext.h \
    core/moeobject.h \
    network/moeresourcerequest.h \
    core/moescriptregisters.h \
    network/moeurl.h \
    core/qargumentparser.h \
    gui/qpaintersurfacebackend.h \
    gui/renderbuffer.h \
    gui/renderinstruction.h \
    gui/renderrecorder.h \
    network/transferdelegate.h \
	gui/widgetsurfacebackend.h

FORMS    += debug/crashdialog.ui

include(../data/resources.pri)
