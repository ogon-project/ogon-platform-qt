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
#include "qogonwindowmanager.h"
#include "qogonscreen.h"

#include <QtGui/QScreen>
#include <QtGui/QImage>

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#else
#include <QtEglSupport/private/qeglconvenience_p.h>
#endif

#include <GL/glext.h>


static QImage::Format imgFormat = QImage::Format_RGBA8888_Premultiplied;

QOgonRenderNodesWindow::QOgonRenderNodesWindow(QWindow *w, QOgonWindowManager *wm) :
	QOgonAbstractWindow(w, wm),
	mGbmSurface(NULL),
	mSurface(EGL_NO_SURFACE),
	mImage(w->size(), imgFormat)
{
	//qDebug("%s: this=%p", __FUNCTION__, this);
	w->setSurfaceType(QSurface::OpenGLSurface);
	wm->addWindow(this);
}

QOgonRenderNodesWindow::~QOgonRenderNodesWindow() {
	QWindow *window = this->window();
	QOgonScreen *screen = static_cast<QOgonScreen *>(window->screen()->handle());
	QOgonEglPlatformContext *context = screen->setAndGetContext();

	mWindowManager->dropWindow(this);

	if (mGbmSurface)
		gbm_surface_destroy(mGbmSurface);

	if (mSurface != EGL_NO_SURFACE)
		eglDestroySurface(context->eglDisplay(), mSurface);
}

EGLSurface QOgonRenderNodesWindow::surface() {
	if (!mSurface)
		this->setGeometry(window()->geometry());

	return mSurface;
}

void QOgonRenderNodesWindow::setGeometry(const QRect &size) {
	qDebug("%s(%d): resizing to %dx%d", __FUNCTION__, (int)mWinId, size.width(), size.height());

	QWindow *window = this->window();
	QOgonScreen *screen = static_cast<QOgonScreen *>(window->screen()->handle());
	QOgonEglPlatformContext *context = screen->setAndGetContext();

	if (mGbmSurface)
		gbm_surface_destroy(mGbmSurface);

	if (mSurface != EGL_NO_SURFACE)
		eglDestroySurface(context->eglDisplay(), mSurface);

	mGbmSurface = gbm_surface_create(context->gbm(), size.width(), size.height(), GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING);
	if (!mGbmSurface) {
		qFatal("%s(%d): unable to create a GBM surface", __FUNCTION__, (int)mWinId);
	}

	QSurfaceFormat platformFormat;
	platformFormat.setDepthBufferSize(24);
	platformFormat.setStencilBufferSize(8);
	platformFormat.setRedBufferSize(8);
	platformFormat.setGreenBufferSize(8);
	platformFormat.setBlueBufferSize(8);
	EGLConfig config = q_configFromGLFormat(context->eglDisplay(), platformFormat);

	mSurface = eglCreateWindowSurface(context->eglDisplay(), config, mGbmSurface, NULL);
	if (mSurface == EGL_NO_SURFACE) {
		qFatal("%s(%d): unable to create an EGL window surface", __FUNCTION__, (int)mWinId);
	}

	mImage = QImage(size.size(), imgFormat);
}

const QImage *QOgonRenderNodesWindow::getContent() {
	QSize size = window()->size();

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, mImage.bits());
	mImage = mImage.mirrored(false, true);

	return &mImage;
}



