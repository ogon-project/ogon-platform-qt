INCLUDEPATH += $$PWD/include

isEmpty(QMAKE_LRELEASE) {
   win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
   else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
   unix {
       !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
   } else {
       !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
   }
}

updateqm.input = TRANSLATIONS
updateqm.output = .qm/${QMAKE_FILE_BASE}.qm
updateqm.commands = mkdir -p .qm && $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm .qm/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps

QMAKE_EXTRA_COMPILERS += updateqm

QOGON_VERSION = 1.0.0
QOGON_PREFIX=$$[QT_INSTALL_PREFIX]
QOGON_LIBRARY_PATH=$$[QT_INSTALL_LIBS]
QOGON_PLUGIN_PATH=$$[QT_INSTALL_PLUGINS]
QOGON_HEADER_PATH=$$[QT_INSTALL_HEADERS]
QOGON_BINARY_PATH=/usr/bin
QOGON_TRANSLATION_PATH=$$[QT_INSTALL_TRANSLATIONS]

!isEmpty(PREFIX):!equals(PREFIX, "/usr"):!equals(PREFIX, "/usr/") {
	QOGON_PREFIX=$$clean_path($$PREFIX)
	QOGON_LIBRARY_PATH = $${QOGON_PREFIX}/lib
	QOGON_PLUGIN_PATH = $${QOGON_LIBRARY_PATH}/qt$${QT_MAJOR_VERSION}/plugins
	QOGON_HEADER_PATH = $${QOGON_PREFIX}/include/qt$${QT_MAJOR_VERSION}
	QOGON_BINARY_PATH = $${QOGON_PREFIX}/bin
	QOGON_TRANSLATION_PATH = $${QOGON_PREFIX}/share/qt$${QT_MAJOR_VERSION}/translations
}
