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

#include "qogonscreen.h"
#include "qogonplatform.h"
#ifdef BUILD_XCURSORS
#include "xcursors/qogonxcursor.h"
#endif
#include "qogonwindowmanager.h"

#ifdef BUILD_RENDER_NODES
#include "qogoneglplatformcontext.h"
#endif

#include <QtCore/QtDebug>
#include <QDebug>

#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

QOgonScreen::QOgonScreen(QOgonWindowManager *windowManager, int width, int height, int /*depth*/) :
	mWindowManager(windowManager)
#ifdef BUILD_RENDER_NODES
	, mGlContext(0)
#endif
#ifdef BUILD_XCURSORS
    , mCursor(new QOgonCursor(windowManager))
#endif

{
	qDebug("QOgonScreen::%s(%d x %d)", __func__, width, height);
	mGeometry = QRect(0, 0, width, height);
}

QOgonScreen::~QOgonScreen() {
	qDebug("QOgonScreen::%s", __FUNCTION__);
}

QRect QOgonScreen::geometry() const {
	return mGeometry;
}

int QOgonScreen::depth() const {
	return 32;
}

QImage::Format QOgonScreen::format() const {
	return QImage::Format_ARGB32_Premultiplied;
}

qreal QOgonScreen::refreshRate() const {
	return 60000.0 / 1000.f;
}

QPlatformCursor *QOgonScreen::cursor() const {
#ifdef BUILD_XCURSORS
	return mCursor;
#else
	return 0;
#endif

}

#ifdef BUILD_RENDER_NODES
QOgonEglPlatformContext *QOgonScreen::setAndGetContext() {
	return mWindowManager->mPlatform->mEglContext;
}
#endif

void QOgonScreen::setGeometry(const QRect &geometry) {
	qDebug("QOgonScreen::%s(%d,%d, %dx%d)", __func__, geometry.left(), geometry.top(),
		geometry.width(), geometry.height());
	if(geometry == mGeometry)
		return;

	mGeometry = geometry;

#if QT_VERSION < 0x050400
	QWindowSystemInterface::handleScreenGeometryChange(screen(), mGeometry);
	QWindowSystemInterface::handleScreenAvailableGeometryChange(screen(), mGeometry);
#else
	QWindowSystemInterface::handleScreenGeometryChange(screen(), mGeometry, mGeometry);
#endif

	mWindowManager->resize(geometry);
	resizeMaximizedWindows();
}


QT_END_NAMESPACE
