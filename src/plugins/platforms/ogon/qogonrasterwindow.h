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

#ifndef __QOGON_WINDOW_H__
#define __QOGON_WINDOW_H__

#include "qogonwindowmanager.h"

class QOgonPlatform;
class QOgonBackingStore;

/**
 * @brief
 */
class QOgonRasterWindow : public QOgonAbstractWindow
{

public:
	/**
	 * @param window
	 * @param platform
	 */
    QOgonRasterWindow(QWindow *window, QOgonWindowManager *wm);

    /** dtor */
    virtual ~QOgonRasterWindow();

    void setBackingStore(QOgonBackingStore *b);

    /** @overload QOgonAbstractWindow
     * @{ */
    virtual const QImage *getContent();
    /** @} */

protected:
    QOgonBackingStore *mBackingStore;
    //QPlatformScreen *mScreen;
};

#endif
