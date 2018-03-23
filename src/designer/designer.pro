include(../../global.pri)

CONFIG += plugin
TEMPLATE = lib

QT += gui-private designer widgets

build_all:!build_pass {
	CONFIG -= build_all
	CONFIG += release
}


HEADERS = qogonwidgets.h \
			qogonmodeswitcherplugin.h

SOURCES = qogonwidgets.cpp \
			qogonmodeswitcherplugin.cpp
RESOURCES = qogonwidgets.qrc
LIBS += -L../widgets -lqogonwidgets
			
TARGET = qogonwidgets_designer

target.path = $$QOGON_PLUGIN_PATH/designer
INSTALLS += target
