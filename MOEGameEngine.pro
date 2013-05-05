#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T18:17:55
#
#-------------------------------------------------

QT       += core gui network script opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MOEGameEngine
TEMPLATE = app


QMAKE_CXXFLAGS+="-std=c++0x"

SOURCES += main.cpp\
	qargumentparser.cpp \
	moeobject.cpp \
	moegraphicsobject.cpp \
	moegraphicscontainer.cpp \
	moeengine.cpp \
	moegraphicssurface.cpp \
    transferdelegate.cpp \
    moegraphicstext.cpp \
    moescriptregisters.cpp \
    moegraphicsimage.cpp \
    moeabstractgraphicssurface.cpp \
    widgetsurfacebackend.cpp \
    crashdialog.cpp

HEADERS  += \
	qargumentparser.h \
	moeobject.h \
	moegraphicsobject.h \
	moegraphicscontainer.h \
	moeengine.h \
	moegraphicssurface.h \
    renderrecorder.h \
    moeresourcerequest.h \
    transferdelegate.h \
    moescriptregisters.h \
    moegraphicstext.h \
    moegraphicsimage.h \
    renderbuffer.h \
    moeabstractgraphicssurface.h \
    renderinstruction.h \
    abstractsurfacebackend.h \
    qpaintersurfacebackend.h \
    widgetsurfacebackend.h \
    crashdialog.h

FORMS    += \
    crashdialog.ui

OTHER_FILES += \
    data/shared.js \
    data/rgb.txt \
    data/prototype.js \
    data/content-select/init.js \
    data/examples/image-viewer/init.js \
    data/examples/crash/init.js

RESOURCES += \
	res.qrc
