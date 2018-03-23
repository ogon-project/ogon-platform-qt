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

#include "qogonscreen.h"
#include "qogonbackingstore.h"
#include "qogonplatform.h"
#include "qogonwindowmanager.h"

#include <QtCore/QtDebug>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformwindow_p.h>

QT_BEGIN_NAMESPACE


QOgonRasterWindow::QOgonRasterWindow(QWindow *window, QOgonWindowManager *wm) :
	QOgonAbstractWindow(window, wm),
    mBackingStore(0)
{
	qDebug("%s ctor(%d, type=0x%x)", __FUNCTION__, (int)mWinId, window->type());
	wm->addWindow(this);
}

QOgonRasterWindow::~QOgonRasterWindow() {
	qDebug("QOgonWindow::%s(%d)", __func__, (int)mWinId);
	mWindowManager->dropWindow(this);
}


void QOgonRasterWindow::setBackingStore(QOgonBackingStore *b) {
	mBackingStore = b;
}

const QImage *QOgonRasterWindow::getContent() {
	if(!mBackingStore)
		return 0;
	return (const QImage *)mBackingStore->paintDevice();
}



QT_END_NAMESPACE
