include(../../global.pri)

QMAKE_RPATHDIR += $$ADDITIONAL_RPATHS

TEMPLATE = app

QT += widgets gui-private

HEADERS = mode_switcher.h 
SOURCES = mode_switcher.cpp 
LIBS += -L../../src/widgets -lqogonwidgets

target.path = $${QOGON_BINARY_PATH}

INSTALLS += target
