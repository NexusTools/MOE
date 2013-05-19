#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T18:17:55
#
#-------------------------------------------------

QT       += core gui network script opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MOEGameEngine
TEMPLATE = app


QMAKE_CXXFLAGS+="-std=c++0x"

SOURCES +=\
    source/debug/crashdialog.cpp \
    source/main.cpp \
    source/gui/moeabstractgraphicssurface.cpp \
    source/core/moeengine.cpp \
    source/gui/moegraphicscontainer.cpp \
    source/gui/moegraphicsimage.cpp \
    source/gui/moegraphicsobject.cpp \
    source/gui/moegraphicssurface.cpp \
    source/gui/moegraphicstext.cpp \
    source/network/moeresourcerequest.cpp \
    source/core/moeobject.cpp \
    source/core/moescriptregisters.cpp \
    source/core/qargumentparser.cpp \
    source/network/transferdelegate.cpp \
    source/gui/widgetsurfacebackend.cpp

HEADERS  += \
    source/gui/moeabstractgraphicssurface.h \
    source/gui/abstractsurfacebackend.h \
    source/debug/crashdialog.h \
    source/core/moeengine.h \
    source/gui/moegraphicscontainer.h \
    source/gui/moegraphicsimage.h \
    source/gui/moegraphicsobject.h \
    source/gui/moegraphicssurface.h \
    source/gui/moegraphicstext.h \
    source/core/moeobject.h \
    source/network/moeresourcerequest.h \
    source/core/moescriptregisters.h \
    source/network/moeurl.h \
    source/core/qargumentparser.h \
    source/gui/qpaintersurfacebackend.h \
    source/gui/renderbuffer.h \
    source/gui/renderinstruction.h \
    source/gui/renderrecorder.h \
    source/network/transferdelegate.h \
    source/gui/widgetsurfacebackend.h

FORMS    += \
    source/debug/crashdialog.ui

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
    loaders/basic.js \
    libraries/standard.js \
    content-select/info.xml

RESOURCES += \
        resources.qrc
