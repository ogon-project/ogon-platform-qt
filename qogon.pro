TEMPLATE=subdirs
SUBDIRS = src 

examples.depends = src

include(global.pri)
message("QOGON_LIBRARY_PATH: $$QOGON_LIBRARY_PATH")
message("QOGON_PLUGIN_PATH: $$QOGON_PLUGIN_PATH")
message("QOGON_HEADER_PATH: $$QOGON_HEADER_PATH")
message("QOGON_BINARY_PATH: $$QOGON_BINARY_PATH")
message("QOGON_TRANSLATION_PATH: $$QOGON_TRANSLATION_PATH")

public_headers.path = $$QOGON_HEADER_PATH/qogon
public_headers.files = include/qogon/qogon.h \
						include/qogon/modeswitcher.h

INCLUDE_POSTFIX=$$replace(QOGON_HEADER_PATH, $$QOGON_PREFIX , "")

pkgconfig.path  = $${QOGON_LIBRARY_PATH}/pkgconfig
pkgconfig.target = qogon.pc
pkgconfig.files = $$OUT_PWD/qogon.pc
pkgconfig.commands = sed \'s,@prefix@,$${QOGON_PREFIX},;s,@include_postfix@,$${INCLUDE_POSTFIX},;s,@version@,$${QOGON_VERSION},;s,@libdir_name@,$${libdir_name},\' $$PWD/qogon.pc.in > qogon.pc
pkgconfig.CONFIG = no_check_exist

cmakeVersion.path = $${QOGON_LIBRARY_PATH}/cmake/qogon
cmakeVersion.target = qogonConfigVersion.cmake
cmakeVersion.files = $$OUT_PWD/qogonConfigVersion.cmake
cmakeVersion.commands = sed \'s,@version@,$${QOGON_VERSION},\' $$PWD/qogonConfigVersion.cmake.in > qogonConfigVersion.cmake
cmakeVersion.CONFIG = no_check_exist

cmakeConfig.path = $${QOGON_LIBRARY_PATH}/cmake/qogon
cmakeConfig.target = qogonConfig.cmake
cmakeConfig.files = $$OUT_PWD/qogonConfig.cmake
cmakeConfig.commands = sed \'s,@header_path@,$${QOGON_HEADER_PATH},\' $$PWD/qogonConfig.cmake.in > qogonConfig.cmake
cmakeConfig.CONFIG = no_check_exist

INSTALLS += public_headers pkgconfig cmakeVersion cmakeConfig
