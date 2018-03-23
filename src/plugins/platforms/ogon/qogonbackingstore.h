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

#ifndef __QFREERSPBACKINGSTORE_H__
#define __QFREERSPBACKINGSTORE_H__

#include <qpa/qplatformbackingstore.h>
#include <QtGui/QImage>


class QOgonPlatform;

/**
 *	@brief
 */
class QOgonBackingStore : public QObject, public QPlatformBackingStore
{
    Q_OBJECT
public:
    /**
     * @param window
     * @param platform
     */
    QOgonBackingStore(QWindow *window, QOgonPlatform *platform);

    /** */
    ~QOgonBackingStore();

    /** @overload QPlatformBackingStore
     * @{*/
    QPaintDevice *paintDevice();
    void flush(QWindow *window, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size, const QRegion &staticContents);
    void beginPaint(const QRegion &region);
    /** @} */

#if 0
    bool scroll(const QRegion &area, int dx, int dy);
#endif

protected:
    QImage mImage;
    QRegion mDirtyRegion;
    QOgonPlatform *mPlatform;
};

#endif
