/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * David Fort <contact@hardening-consulting.com>
 *
 * This file may be used under the terms of the GNU Affero General
 * Public License version 3 as published by the Free Software Foundation
 * and appearing in the file LICENSE-AGPL included in the distribution
 * of this file.
 *
 * Under the GNU Affero General Public License version 3 section 7 the
 * copyright holders grant the additional permissions set forth in the
 * ogon Core AGPL Exceptions version 1 as published by
 * Thincast Technologies GmbH.
 *
 * Under the GNU Affero General Public License version 3 section 7 the
 * copyright holders grant the additional permissions set forth in the
 * ogon Library AGPL Exceptions version 1 as published by
 * Thincast Technologies GmbH.
 *
 * For more information see the file LICENSE in the distribution of this file.
 */

#include "qogonplatform.h"
#include "qogonnativeinterface.h"
#include "qogonscreen.h"
#include "qogonbackingstore.h"
#include "qogonwindowmanager.h"
#ifdef BUILD_RENDER_NODES
#include "qogoneglplatformcontext.h"
#include "qogonrendernodeswindow.h"
#endif
#include "qogonrasterwindow.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtPlatformSupport/private/qgenericunixthemes_p.h>
#else
#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtThemeSupport/private/qgenericunixthemes_p.h>
#endif

#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextplugin_p.h>
#include <qpa/qplatforminputcontextfactory_p.h>

#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QtCore/QSocketNotifier>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>

#include <winpr/wlog.h>

#ifdef BUILD_VKB
#include <QPluginLoader>
#include <QObject>
#endif

bool geometryFromParams(const QStringList& paramList, QRect *screenSize) {
	QString subVal;
	bool ok;
	bool haveWidth = false, haveHeight = false;

	for (int i = 0; i < paramList.size(); i++) {
		const QString &param = paramList.at(i);
		if(param.startsWith(QLatin1String("width="))) {
			subVal = param.mid(strlen("width="));
			screenSize->setWidth(subVal.toInt(&ok));
			if(!ok)
				qFatal("invalid width");
			haveWidth = true;
		} else if(param.startsWith(QLatin1String("height="))) {
			subVal = param.mid(strlen("height="));
			screenSize->setHeight(subVal.toInt(&ok));
			if(!ok)
				qFatal("invalid height");
			haveHeight = true;
		}
	}
	return haveWidth && haveHeight;
}

bool parseParams(const QStringList& paramList, bool &allowReconnection) {
	allowReconnection = false;
	for (int i = 0; i < paramList.size(); i++) {
		const QString &param = paramList.at(i);
		if (param == "allowReconnection")
			allowReconnection = true;
	}
	return true;
}

QOgonPlatform::QOgonPlatform(const QStringList& paramList)
    : mFontDb(new QGenericUnixFontDatabase()),
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
    mEventDispatcher(createUnixEventDispatcher()),
#endif
	mWindowManager(0),
    mNative(new QOgonNativeInterface(this)),
    mInputContext(0)
#ifdef BUILD_RENDER_NODES
    ,mEglContext(0)
#endif
{
	wLogAppender *wlog_appender;
	wLog *wlog_root;

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
	QGuiApplicationPrivate::instance()->setEventDispatcher(mEventDispatcher);
#endif

	//Disable desktop settings for now (or themes crash)
	QGuiApplicationPrivate::obey_desktop_settings = false;

	if (!geometryFromParams(paramList, &mScreenGeometry))
		mScreenGeometry.setSize( QSize(320, 200) );

	mPath = qgetenv("OGON_PIPE_PATH");
	if (!mPath.size())
		qFatal("no OGON_PIPE_PATH was given");

	if (QFile::exists(mPath))
		QFile::remove(mPath);

	parseParams(paramList, mAllowReconnection);

	WLog_Init();
	wlog_root = WLog_GetRoot();
	if (wlog_root) {
		WLog_SetLogAppenderType(wlog_root, WLOG_APPENDER_CONSOLE);

		wlog_appender = WLog_GetLogAppender(wlog_root);
		if (wlog_appender)
			WLog_ConfigureAppender(wlog_appender, "outputstream", const_cast<char *>("stderr"));

	}

	mWindowManager = new QOgonWindowManager(this, mScreenGeometry, NULL, mAllowReconnection);
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
	screenAdded(mWindowManager->mScreen);
#else
	QWindowSystemInterface::handleScreenAdded(mWindowManager->mScreen);
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
	initialize();
#endif

}

QOgonPlatform::~QOgonPlatform() {
	delete mWindowManager;
	delete mNative;
	delete mFontDb;

	WLog_Uninit();
}

void QOgonPlatform::initialize() {
	QLocalServer *listeningSock = new QLocalServer();
	listeningSock->listen(mPath);
	mWindowManager->setListeningSocket(listeningSock);


#ifdef BUILD_VKB
	QPluginLoader loader("platforminputcontexts/libogon.so");
	if (!loader.load()) {
		qDebug("unable to load ogon input context");
	}

	QObject *obj = loader.instance();
	if (obj) {
		QPlatformInputContextPlugin *plugin = qobject_cast<QPlatformInputContextPlugin *>(obj);
		if (plugin)
			mInputContext = plugin->create("ogon", QStringList());
	}
#endif
	mInputContext = QPlatformInputContextFactory::create();
}

QPlatformWindow *QOgonPlatform::createPlatformWindow(QWindow *window) const {
#ifdef BUILD_RENDER_NODES
	QPlatformWindow *w;
#endif

	switch (window->surfaceType()) {
	case QSurface::RasterSurface:
		return new QOgonRasterWindow(window, mWindowManager);
#ifdef BUILD_RENDER_NODES
	case QSurface::OpenGLSurface:
		w = new QOgonRenderNodesWindow(window, mWindowManager);
		w->requestActivateWindow();
		return w;
#endif
	default:
		qFatal("%s: unhandled window type %d", __FUNCTION__, (int)window->surfaceType());
		break;
	}

	return 0;
}


QPlatformBackingStore *QOgonPlatform::createPlatformBackingStore(QWindow *window) const {
	//qDebug("%s: windowSurface type=%d", __FUNCTION__, (int)window->surfaceType());
    return new QOgonBackingStore(window, (QOgonPlatform *)this);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
QAbstractEventDispatcher *QOgonPlatform::guiThreadEventDispatcher() const {
    return mEventDispatcher;
}
#else
QAbstractEventDispatcher *QOgonPlatform::createEventDispatcher() const {
    return createUnixEventDispatcher();
}
#endif

QPlatformFontDatabase *QOgonPlatform::fontDatabase() const {
    return mFontDb;
}

QStringList QOgonPlatform::themeNames() const {
    return QGenericUnixTheme::themeNames();
}

QPlatformTheme *QOgonPlatform::createPlatformTheme(const QString &name) const {
    return QGenericUnixTheme::createUnixTheme(name);
}

QPlatformNativeInterface *QOgonPlatform::nativeInterface() const {
	return mNative;
}

QPlatformInputContext *QOgonPlatform::inputContext() const {
	return mInputContext;
}

#ifdef BUILD_RENDER_NODES
QPlatformOpenGLContext *QOgonPlatform::createPlatformOpenGLContext(QOpenGLContext *context) const {
	QOgonPlatform *that = const_cast<QOgonPlatform *>(this);
	if (!mEglContext) {
		that->mEglContext = QOgonEglPlatformContext::createInstance(context->format(), context->handle(), mWindowManager);
	}
	return mEglContext;
}

#if QT_VERSION > QT_VERSION_CHECK(5, 3, 0)
QOpenGLContext::OpenGLModuleType QOgonPlatform::openGLModuleType() {
	return QOpenGLContext::LibGLES;
}
#endif
#endif

bool QOgonPlatform::hasCapability(
		QPlatformIntegration::Capability cap) const {
	switch (cap) {
	case ThreadedOpenGL:
		return false;

#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
	case RasterGLSurface:
		return false;
#endif
	case OpenGL:
#ifdef BUILD_RENDER_NODES
		return QOgonEglPlatformContext::haveRenderNodesSupport();
#else
		return false;
#endif

	case ThreadedPixmaps:
		return true;
	default:
		return QPlatformIntegration::hasCapability(cap);
	}
}

