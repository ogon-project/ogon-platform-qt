TARGET=ogon

TEMPLATE = lib
CONFIG += plugin

include(../../../../global.pri)

QMAKE_RPATHDIR += $$ADDITIONAL_RPATHS

unix {
    CONFIG += link_pkgconfig
    
    PKGCONFIG += xkbcommon glib-2.0 ogon-backend1 winpr2 fontconfig freerdp-server2 ogon-message1

	SOURCES += qogonshmframebuffer.cpp
	HEADERS += qogonshmframebuffer.h

	packagesExist(xcursor) {
		SOURCES += xcursors/xcursor.cpp xcursors/ogon-cursor.cpp xcursors/qogonxcursor.cpp
		HEADERS += xcursors/ogon-cursor.h xcursors/qogonxcursor.h xcursors/xcursor.h
		DEFINES += BUILD_XCURSORS
	}

	greaterThan(QT_MINOR_VERSION, 1) {
		packagesExist("libudev \'gbm >= 0.9.0\' egl"){
			PKGCONFIG += libudev gbm egl
			QT += opengl

			SOURCES += qogoneglplatformcontext.cpp \
						qogonrendernodeswindow.cpp
			HEADERS += qogoneglplatformcontext.h \
						qogonrendernodeswindow.h
			DEFINES += BUILD_RENDER_NODES
		}
	}
}

CONFIG += link_pkgconfig qpa/genericunixfontdatabase 


QT += core-private gui-private network widgets
lessThan(QT_MINOR_VERSION, 8) {
QT += platformsupport-private
} else {
QT += fontdatabase_support_private eventdispatcher_support_private  theme_support_private egl_support_private
}



OTHER_FILES = qogon.json

SOURCES += main.cpp 				\
		qogonplatform.cpp		\
		qogonnativeinterface.cpp	\
		qogonbackingstore.cpp	\
		qogonscreen.cpp			\
		qogonrasterwindow.cpp	\
		qogonwindowmanager.cpp	\
		qogonkeyboard.cpp		\
		qogontouch.cpp			\
		qogonmessage.cpp			\
		qogonpeer.cpp

HEADERS += qogonplatform.h 		\
		qogonnativeinterface.h	\
		qogonbackingstore.h 		\
		qogonscreen.h 			\
		qogonrasterwindow.h 		\
		qogonwindowmanager.h 	\
		qogonkeyboard.h			\
		qogontouch.h				\
		qogonmessage.h				\
		qogonpeer.h

target.path = $$QOGON_PLUGIN_PATH/platforms

INSTALLS += target
