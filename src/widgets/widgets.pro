include(../../global.pri)

CONFIG += plugin
TEMPLATE = lib

QT += gui-private designer widgets

isEmpty(LOCAL_TRANSLATIONS) {
	DEFINES += TRANSLATIONS_DIR=\\\"$$QOGON_TRANSLATION_PATH\\\"
} else {
	DEFINES += TRANSLATIONS_DIR=\\\"$$PWD/.qm/\\\"
}

HEADERS = ../../include/qogon/modeswitcher.h
SOURCES = qogonmodeswitcher.cpp
TRANSLATIONS = qogonwidgets_fr.ts qogonwidgets_en.ts qogonwidgets_de.ts
			
translations.path = $$QOGON_TRANSLATION_PATH
translations.CONFIG = no_check_exist
translations.files = $$OUT_PWD/.qm/qogonwidgets_fr.qm $$OUT_PWD/.qm/qogonwidgets_en.qm $$OUT_PWD/.qm/qogonwidgets_de.qm
			
TARGET = qogonwidgets

target.path = $$QOGON_LIBRARY_PATH
INSTALLS += target translations
