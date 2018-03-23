include(../../global.pri)
TEMPLATE = app

QT += widgets gui-private

HEADERS = simple_greeter.h
SOURCES = simple_greeter.cpp
FORMS += greeter.ui

target.path = $$QOGON_BINARY_PATH
INSTALLS += target
