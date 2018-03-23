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

#include "qogoneglplatformcontext.h"
#include "qogonrendernodeswindow.h"

#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>
#include <errno.h>

#include <QList>
#include <qdebug.h>


struct QOgonEglPlatformContextPriv {
    int drmFd;
    struct gbm_device *gbm;
    QList<QOgonRenderNodesWindow *> dirtySurfaces;
};

static char *open_drm_device(struct udev *udev) {
	struct udev_enumerate *e;
	struct udev_list_entry *entry;
	struct udev_device *device;
	const char *path, *filename;
	char *ret;
	int fd;

	e = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(e, "drm");
	udev_enumerate_add_match_sysname(e, "renderD[0-9]*");

	udev_enumerate_scan_devices(e);
	device = NULL;
	ret = NULL;
	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)) {
		path = udev_list_entry_get_name(entry);
		device = udev_device_new_from_syspath(udev, path);
		if (device) {
			filename = udev_device_get_devnode(device);
			fd = open(filename, O_RDWR | O_CLOEXEC);
			if (fd < 0) {
				if (errno == EPERM)
					qDebug("invalid permission on %s", filename);

				udev_device_unref(device);
				continue;
			}

			close(fd);
			ret = strdup(filename);
			udev_device_unref(device);
			break;
		}
	}

	udev_enumerate_unref(e);
	return ret;
}

bool QOgonEglPlatformContext::haveRenderNodesSupport() {
	if (!ms_renderNodesTested) {
		ms_renderNodesTested = true;

		struct udev *udev = udev_new();
		if (udev) {
			const char *fname = open_drm_device(udev);
			if (fname)
				ms_deviceName = fname;

			udev_unref(udev);
		}
	}

	return ms_deviceName.size();
}

QOgonEglPlatformContext *QOgonEglPlatformContext::createInstance(const QSurfaceFormat &format,
		QPlatformOpenGLContext *share, QOgonWindowManager *wm)
{
	QSurfaceFormat platformFormat;
	EGLDisplay dpy;
	int drmFd;
	struct gbm_device *gbm;
	EGLint major, minor;

	if (!haveRenderNodesSupport()) {
		qWarning("no render nodes support");
		return 0;
	}

	drmFd = open(ms_deviceName.constData(), O_RDWR | O_CLOEXEC);
	if (drmFd < 0) {
		qWarning("unable to open renderNodes device");
		goto error;
	}

	gbm = gbm_create_device(drmFd);
	if (!gbm) {
		qWarning("unable to create gbm device");
		goto error_fd;
	}

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		qFatal("unable to bind GL ES API");
		goto error_gbm;
	}

#ifdef EGL_MESA_platform_gbm
	dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, gbm, NULL);
#else
	dpy = eglGetDisplay(gbm);
#endif
    if (dpy == EGL_NO_DISPLAY) {
    	qWarning("unable to open an EGL display");
    	goto error_gbm;
    }

	if (!eglInitialize(dpy, &major, &minor)) {
		qWarning("error initializing EGL display");
		goto error_egl;
	}

	qDebug("EGL version %d.%d", major, minor);

	if (format.profile() == QSurfaceFormat::NoProfile) {
		platformFormat.setDepthBufferSize(24);
		platformFormat.setStencilBufferSize(8);
        platformFormat.setRedBufferSize(8);
        platformFormat.setGreenBufferSize(8);
        platformFormat.setBlueBufferSize(8);
	} else {
		platformFormat = format;
	}

    return new QOgonEglPlatformContext(platformFormat, share, dpy, drmFd, gbm, wm);

error_egl:
	eglTerminate(dpy);
error_gbm:
	gbm_device_destroy(gbm);
error_fd:
	close(drmFd);
error:
	return 0;
}

QOgonEglPlatformContext::QOgonEglPlatformContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share,
								EGLDisplay display, int fd, struct gbm_device *gbm, QOgonWindowManager *wm) :
				QEGLPlatformContext(format, share, display),
				d(new QOgonEglPlatformContextPriv()),
				mWm(wm)

{
	d->drmFd = fd;
	d->gbm = gbm;
}

struct gbm_device *QOgonEglPlatformContext::gbm() {
	return d->gbm;
}

EGLSurface QOgonEglPlatformContext::eglSurfaceForPlatformSurface(QPlatformSurface *surface) {
	QOgonRenderNodesWindow *win = static_cast<QOgonRenderNodesWindow *>(surface);
	return win->surface();
}

bool QOgonEglPlatformContext::makeCurrent(QPlatformSurface *surface) {
	QOgonRenderNodesWindow *window = static_cast<QOgonRenderNodesWindow *>(surface);

	//qDebug("%s(%d): %p", __FUNCTION__, (int)window->winId(), surface);
	if (!d->dirtySurfaces.contains(window))
		d->dirtySurfaces.push_front(window);

	return QEGLPlatformContext::makeCurrent(surface);
}

void QOgonEglPlatformContext::doneCurrent() {
	//qDebug("%s", __FUNCTION__);
	QEGLPlatformContext::doneCurrent();
}

void QOgonEglPlatformContext::swapBuffers(QPlatformSurface *surface) {
	QOgonRenderNodesWindow *window = static_cast<QOgonRenderNodesWindow *>(surface);
	//qDebug("%s(%d): %p", __FUNCTION__, (int)window->winId(), surface);
	mWm->repaint(window->geometry());
	QEGLPlatformContext::swapBuffers(surface);
}


bool QOgonEglPlatformContext::ms_renderNodesTested = false;
QByteArray QOgonEglPlatformContext::ms_deviceName;
