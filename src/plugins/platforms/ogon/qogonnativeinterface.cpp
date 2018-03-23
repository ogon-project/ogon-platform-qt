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

#include "qogonnativeinterface.h"
#include "qogonplatform.h"
#include <qogon/qogon.h>

QOgonNativeInterface::QOgonNativeInterface(QOgonPlatform *platform) :
	mPlatform(platform)
{
}

void *QOgonNativeInterface::nativeResourceForIntegration(const QByteArray &resource) {
	if(resource == QOGON_RESSOURCE_STRING)
		return mPlatform->mWindowManager;
	return 0;
}


void *QOgonNativeInterface::nativeResourceForWindow(const QByteArray &resourceString,
		QWindow * /*window*/)
{
	if(resourceString == QOGON_RESSOURCE_STRING)
		return mPlatform->mWindowManager;
	return 0;
}

void *QOgonNativeInterface::nativeResourceForScreen(const QByteArray &resourceString,
		QScreen * /*screen*/)
{
	if(resourceString == QOGON_RESSOURCE_STRING)
		return mPlatform->mWindowManager;
	return 0;
}
