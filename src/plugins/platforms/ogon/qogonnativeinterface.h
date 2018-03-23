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

#ifndef __QOGON_NATIVE_INTERFACE_H__
#define __QOGON_NATIVE_INTERFACE_H__

#include <qpa/qplatformnativeinterface.h>

class QOgonPlatform;

/**
 * @brief
 */
class QOgonNativeInterface : public QPlatformNativeInterface {
public:
	QOgonNativeInterface(QOgonPlatform *platform);

	/** @overload QPlatformNativeInterface
	 * @{ */
	virtual void *nativeResourceForIntegration(const QByteArray &resource);
    virtual void *nativeResourceForWindow(const QByteArray &resourceString,
				  QWindow *window);
    virtual void *nativeResourceForScreen(const QByteArray &resourceString,
                                  QScreen *screen);
	/** @} */

protected:
	QOgonPlatform *mPlatform;
};

#endif // __QOGON_NATIVE_INTERFACE_H__
