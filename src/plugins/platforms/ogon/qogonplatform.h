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

#ifndef __QPLATFORMINTEGRATION_OGON_H__
#define __QPLATFORMINTEGRATION_OGON_H__

#include <qpa/qplatformintegration.h>


class QAbstractEventDispatcher;
class QPlatformTheme;
class QOgonScreen;
class QOgonWindowManager;
class QOgonBackingStore;
class QOgonNativeInterface;
class QPlatformInputContext;
class QOgonEglPlatformContext;

/**
 *	@brief
 */
class QOgonPlatform : public QPlatformIntegration
{
	friend class QOgonBackingStore;
	friend class QOgonNativeInterface;
	friend class QOgonScreen;
public:
	/**
	 * @param paramList
	 */
	QOgonPlatform(const QStringList& paramList);

	/** dtor */
    virtual ~QOgonPlatform();

    /** @overload QPlatformIntegration
     * @{ */
    virtual bool hasCapability(QPlatformIntegration::Capability cap) const;
    virtual QPlatformWindow *createPlatformWindow(QWindow *window) const;
    virtual QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const;
    virtual QPlatformFontDatabase *fontDatabase() const;
    virtual QStringList themeNames() const;
    virtual QPlatformTheme *createPlatformTheme(const QString &name) const;
    virtual QPlatformNativeInterface *nativeInterface() const;
    virtual QPlatformInputContext *inputContext() const;
#ifdef BUILD_RENDER_NODES
    virtual QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;
#if QT_VERSION > 0x050300
    virtual QOpenGLContext::OpenGLModuleType openGLModuleType();
#endif
#endif
#if QT_VERSION < 0x050200
    virtual QAbstractEventDispatcher *guiThreadEventDispatcher() const;
#else
    virtual QAbstractEventDispatcher *createEventDispatcher() const;
#endif
    void initialize();
    /** @} */

protected:
    QPlatformFontDatabase *mFontDb;
#if QT_VERSION < 0x050200
    QAbstractEventDispatcher *mEventDispatcher;
#endif
    QOgonWindowManager *mWindowManager;
    QOgonNativeInterface *mNative;
    QPlatformInputContext *mInputContext;
#ifdef BUILD_RENDER_NODES
    QOgonEglPlatformContext *mEglContext;
#endif
    QString mPath;
    QRect mScreenGeometry;
    bool mAllowReconnection;
};


#endif
