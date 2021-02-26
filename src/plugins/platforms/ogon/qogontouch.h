/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * Bernhard Miklautz <bernhard.miklautz@thincast.com>
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

#ifndef __QOGON_TOUCH_H__
#define __QOGON_TOUCH_H__


#include <freerdp/server/rdpei.h>

#include <QtGui/qpa/qwindowsysteminterface.h>

class QSocketNotifier;
class QOgonWindowManager;
class QTouchDevice;

/** @brief */
class QOgonTouch : public QObject {
	Q_OBJECT
public:
	QOgonTouch(QOgonWindowManager *wm);
	virtual ~QOgonTouch();

	bool init();

	void disable();
protected:
	typedef QList<QWindowSystemInterface::TouchPoint> TouchPointList;

	Qt::TouchPointState stateFromTouchData(RDPINPUT_CONTACT_DATA *data);

	void treatTouchEvent(const RDPINPUT_TOUCH_EVENT *touchEvent);

	void treatTouchPointsDiff(TouchPointList &touchPoints);

protected slots:
	void readyRead();


protected:
	QOgonWindowManager *mWm;
	QTouchDevice *mDevice;
	RdpeiServerContext *mContext;
	QSocketNotifier *mNotifier;
	quint32 mTimestamp;

	TouchPointList mPrevTouchPoints;

	static UINT onClientReady(RdpeiServerContext *context);
	static UINT onTouchEvent(RdpeiServerContext *context, const RDPINPUT_TOUCH_EVENT *touchEvent);
	static UINT onTouchReleased(RdpeiServerContext *context, BYTE contactId);
	static UINT onPenEvent(RdpeiServerContext *context, const RDPINPUT_PEN_EVENT *penEvent);
};

#endif /* __QOGON_TOUCH_H__ */
