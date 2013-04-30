#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T18:17:55
#
#-------------------------------------------------

QT       += core gui network script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MOEGameEngine
TEMPLATE = app

SOURCES += main.cpp\
		moe.cpp \
	qargumentparser.cpp \
	moeobject.cpp \
	moegraphicsobject.cpp \
	moegraphicscontainer.cpp \
	moeengine.cpp \
	moeengineview.cpp \
	moegraphicssurface.cpp \
    renderrecorder.cpp \
    transferdelegate.cpp

HEADERS  += moe.h \
	qargumentparser.h \
	moeobject.h \
	moegraphicsobject.h \
	moegraphicscontainer.h \
	moeengine.h \
	moeengineview.h \
	moegraphicssurface.h \
    renderrecorder.h \
    moeresourcerequest.h \
    transferdelegate.h \
    moescriptregisters.h

FORMS    +=

OTHER_FILES += \
	data/content-select/init.js \
    data/shared.js \
    data/rgb.txt

RESOURCES += \
	res.qrc
