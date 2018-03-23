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

#ifndef __QOGON_XCURSOR_H__
#define __QOGON_XCURSOR_H__

#include <QString>
#include <QMultiMap>
#include <qpa/qplatformcursor.h>

class QOgonWindowManager;
struct ogon_cursor_theme;
struct ogon_cursor;

/**
 * @brief
 */
class QOgonCursor : public QPlatformCursor {
public:
	QOgonCursor(QOgonWindowManager *platform);

	~QOgonCursor();

#ifndef QT_NO_CURSOR
    virtual void changeCursor(QCursor *windowCursor, QWindow *window);
#endif

    void restoreLastCursor();

protected:
    /** @brief */
    enum OgonCursor {
		ArrowCursor = Qt::ArrowCursor,
		UpArrowCursor,
		CrossCursor,
		WaitCursor,
		IBeamCursor,
		SizeVerCursor,
		SizeHorCursor,
		SizeBDiagCursor,
		SizeFDiagCursor,
		SizeAllCursor,
		BlankCursor,
		SplitVCursor,
		SplitHCursor,
		PointingHandCursor,
		ForbiddenCursor,
		WhatsThisCursor,
		BusyCursor,
		OpenHandCursor,
		ClosedHandCursor,
		DragCopyCursor,
		DragMoveCursor,
		DragLinkCursor,
		ResizeNorthCursor = Qt::CustomCursor + 1,
		ResizeSouthCursor,
		ResizeEastCursor,
		ResizeWestCursor,
		ResizeNorthWestCursor,
		ResizeSouthEastCursor,
		ResizeNorthEastCursor,
		ResizeSouthWestCursor
	};

protected:
    void initCursorMap();
    struct ogon_cursor *requestCursor(OgonCursor shape);

protected:
    QOgonWindowManager *mWindowManager;
    QMultiMap<OgonCursor, QByteArray> mCursorNamesMap;
    QMap<OgonCursor, struct ogon_cursor *> mCursors;
    struct ogon_cursor_theme *mCursorTheme;
    QCursor mLastCursor;
};


#endif // __QOGON_XCURSOR_H__
