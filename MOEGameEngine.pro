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

SOURCES += source/main.cpp\
        source/qargumentparser.cpp \
        source/moeobject.cpp \
        source/moegraphicsobject.cpp \
        source/moegraphicscontainer.cpp \
        source/moeengine.cpp \
        source/moegraphicssurface.cpp \
    source/transferdelegate.cpp \
    source/moegraphicstext.cpp \
    source/moescriptregisters.cpp \
    source/moegraphicsimage.cpp \
    source/moeabstractgraphicssurface.cpp \
    source/widgetsurfacebackend.cpp \
    source/crashdialog.cpp

HEADERS  += \
        source/qargumentparser.h \
        source/moeobject.h \
        source/moegraphicsobject.h \
        source/moegraphicscontainer.h \
        source/moeengine.h \
        source/moegraphicssurface.h \
    source/renderrecorder.h \
    source/moeresourcerequest.h \
    source/transferdelegate.h \
    source/moescriptregisters.h \
    source/moegraphicstext.h \
    source/moegraphicsimage.h \
    source/renderbuffer.h \
    source/moeabstractgraphicssurface.h \
    source/renderinstruction.h \
    source/abstractsurfacebackend.h \
    source/qpaintersurfacebackend.h \
    source/widgetsurfacebackend.h \
    source/crashdialog.h \
    source/moeurl.h

FORMS    += \
    source/crashdialog.ui

OTHER_FILES += \
    libraries/shared.js \
    resources/rgb.txt \
    libraries/prototype.js \
    content-select/init.js \
    examples/image-viewer/init.js \
    examples/crash/init.js \
    examples/blockcity/info.xml \
    examples/blockcity/init.js \
    examples/crash/info.xml \
    examples/image-viewer/info.xml \
    loaders/standard.js \
    loaders/basic.js

RESOURCES += \
        resources.qrc
