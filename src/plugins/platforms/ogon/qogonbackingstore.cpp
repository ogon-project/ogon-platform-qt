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

#include "qogonrasterwindow.h"
#include "qogonbackingstore.h"
#include "qogonplatform.h"
#include "qogonwindowmanager.h"

#include <qpa/qplatformwindow.h>
#include <QtCore/QtDebug>

QT_BEGIN_NAMESPACE

QOgonBackingStore::QOgonBackingStore(QWindow *window, QOgonPlatform *platform)
    : QPlatformBackingStore(window),
      mPlatform(platform)
{
	QOgonRasterWindow *ogonWindow = (QOgonRasterWindow *)window->handle();

	ogonWindow->setBackingStore(this);
}

QOgonBackingStore::~QOgonBackingStore() {
}

QPaintDevice *QOgonBackingStore::paintDevice() {
	return &mImage;
}

void QOgonBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
	Q_UNUSED(window);
	Q_UNUSED(offset);
	QPoint winTopLeft = window->geometry().topLeft();

	foreach (const QRect &rect, (region & mDirtyRegion).rects()) {
		QRect globalRect = rect.translated(winTopLeft);
		mPlatform->mWindowManager->repaint(globalRect);
	}
	mDirtyRegion -= region;
}

void QOgonBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
	Q_UNUSED(staticContents);
	//qDebug("QOgonBackingStore::%s(%d, %dx%d)", __func__, (int)window()->winId(),
	//	size.width(), size.height());

	if (mImage.size() != size)
		mImage = QImage(size, QImage::Format_ARGB32_Premultiplied);
	mDirtyRegion = QRegion();
}


#if 0
extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);
bool QOgonBackingStore::scroll(const QRegion &area, int dx, int dy)
{
	qDebug() << "QOgonBackingStore::" << __func__ << "()";
	const QPoint offset(dx, dy);
	foreach (const QRect &rect, area.rects()) {
		QMetaObject::invokeMethod(mHtmlService, "scroll",
		Q_ARG(int, static_cast<int>(window()->winId())),
		Q_ARG(int, rect.x()),
		Q_ARG(int, rect.y()),
		Q_ARG(int, rect.width()),
		Q_ARG(int, rect.height()),
		Q_ARG(int, dx),
		Q_ARG(int, dy));*/
		qt_scrollRectInImage(mImage, rect, offset);
	}
	return true;
}
#endif


void QOgonBackingStore::beginPaint(const QRegion &region)
{
    mDirtyRegion += region;
    QPlatformBackingStore::beginPaint(region);
}

QT_END_NAMESPACE
