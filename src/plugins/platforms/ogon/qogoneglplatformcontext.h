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

#ifndef __QOGONEGLPLATFORMCONTEXT_H__
#define __QOGONEGLPLATFORMCONTEXT_H__

#include <qpa/qplatformopenglcontext.h>
#include <gbm.h>
#include <EGL/egl.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#include <QtPlatformSupport/private/qeglplatformcontext_p.h>
#else
#include <QtEglSupport/private/qeglplatformcontext_p.h>
#endif

struct QOgonEglPlatformContextPriv;
class QOgonWindowManager;

/**
 *	@brief
 */
class QOgonEglPlatformContext : public QEGLPlatformContext {
public:
	/** Constructor
	 *
	 * @param format surface format
	 * @param share a shared instance
	 * @param display the EGL display
	 * @param fd a file opened on the render nodes device
	 * @param gbm a gbm device struct opened on the render nodes device
	 */
	QOgonEglPlatformContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, EGLDisplay display,
			int fd, struct gbm_device *gbm, QOgonWindowManager *wm);

	/** @return the underlying gbm device */
	struct gbm_device *gbm();

    virtual bool makeCurrent(QPlatformSurface *surface);
    virtual void doneCurrent();
	virtual void swapBuffers(QPlatformSurface *surface);
public:
	/** @return if render nodes are supported */
	static bool haveRenderNodesSupport();

	/**
	 *  retrieve an instance of context
	 * @param format
	 * @param share
	 * @return
	 */
	static QOgonEglPlatformContext *createInstance(const QSurfaceFormat &format,
					QPlatformOpenGLContext *share, QOgonWindowManager *wm);

protected:
    virtual EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface);

protected:
    QOgonEglPlatformContextPriv *d;
    QOgonWindowManager *mWm;

protected:
    static bool ms_renderNodesTested;
    static QByteArray ms_deviceName;
};


#endif /* __QOGONEGLPLATFORMCONTEXT_H__ */
