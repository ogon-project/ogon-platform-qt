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

#ifndef __QOGONRENDERNODESWINDOW_H__
#define __QOGONRENDERNODESWINDOW_H__

#include <qpa/qplatformwindow.h>

#include <gbm.h>
#include <EGL/egl.h>

#include "qogonwindowmanager.h"


/**
 * @brief
 */
class QOgonRenderNodesWindow : public QOgonAbstractWindow
{
public:
	/**
	 * @param window
	 */
	QOgonRenderNodesWindow(QWindow *window, QOgonWindowManager *wm);

	/** dtor */
	~QOgonRenderNodesWindow();

	/** @return the window's id of this window */
    WId winId() const { return mWinId; }

    /** @return the associated EGL surface */
    EGLSurface surface();

    virtual void setGeometry(const QRect &rect);

    const QImage *getContent();

protected:
    struct gbm_surface *mGbmSurface;
    EGLSurface mSurface;
    QImage mImage;
};


#endif /* __QOGONRENDERNODESWINDOW_H_ */
