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

#ifndef __QOGON_SCREEN_H__
#define __QOGON_SCREEN_H__

#include <qpa/qplatformscreen.h>


class QOgonPlatform;
class QOgonWindowManager;
class QOgonCursor;
class QOgonEglPlatformContext;

/**
 * @brief
 */
class QOgonScreen : public QObject, public QPlatformScreen
{
	friend class QOgonWindowManager;
	friend class QOgonPlatform;

    Q_OBJECT
public:
    /**
     * @param platform
     * @param width
     * @param height
     * @param depth
     */
    QOgonScreen(QOgonWindowManager *windowManager, int width, int height, int depth);
    ~QOgonScreen();

    virtual QRect geometry() const;
    virtual int depth() const;
    virtual QImage::Format format() const;
    virtual qreal refreshRate() const;
    virtual QPlatformCursor *cursor() const;

#ifdef BUILD_RENDER_NODES
    QOgonEglPlatformContext *setAndGetContext();
#endif

public slots:
    void setGeometry(const QRect &geometry);

protected:
    QRect mGeometry;

    QOgonWindowManager *mWindowManager;
    //QOgonCursor *mCursor;
#ifdef BUILD_RENDER_NODES
    QOgonEglPlatformContext *mGlContext;
#endif
#ifdef BUILD_XCURSORS
    QOgonCursor *mCursor;
#endif

};

#endif
