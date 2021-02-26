/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * Bernhard Miklautz <bernhard.miklautz@thincast.com>
 * David Fort <contact@hardening-consulting.com>
 * Norbert Federa <norbert.federa@thincast.com>
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

#include <QtCore/QtDebug>
#include <QSocketNotifier>

#include "qogonwindowmanager.h"
#include "qogontouch.h"

QOgonTouch::QOgonTouch(QOgonWindowManager *wm) : mWm(wm), mDevice(0), mNotifier(0), mTimestamp(0) {
	mContext = rdpei_server_context_new(WTS_CURRENT_SERVER_HANDLE);
	mContext->onClientReady = onClientReady;
	mContext->onTouchEvent = onTouchEvent;
	mContext->user_data = this;
}

QOgonTouch::~QOgonTouch() {
	if (mContext)
		rdpei_server_context_free(mContext);
	delete mDevice;
}

bool QOgonTouch::init() {
	int fd;
	HANDLE handle;
	if (rdpei_server_init(mContext) != CHANNEL_RC_OK) {
		return false;
	}

	if (rdpei_server_send_sc_ready(mContext, RDPINPUT_PROTOCOL_V101) != CHANNEL_RC_OK) {
		// rdpei_server_close(mContext) ???
		return false;
	}

	handle = rdpei_server_get_event_handle(mContext);
	fd = GetEventFileDescriptor(handle);

	mNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(mNotifier, SIGNAL(activated(int)), this, SLOT(readyRead()));

	return true;
}

void QOgonTouch::readyRead() {
	int ret;

	ret = rdpei_server_handle_messages(mContext);
	switch (ret) {
	case 1:
	case -1:
		break;
	case 0:
		qDebug("%s: disconnected !!!", __FUNCTION__);
		break;
	}
}



void QOgonTouch::disable() {
}

UINT QOgonTouch::onClientReady(RdpeiServerContext *context) {
	if ((context->clientVersion != RDPINPUT_PROTOCOL_V10) && (context->clientVersion != RDPINPUT_PROTOCOL_V101))
		qDebug("strange got an unexpected client version 0x%x", context->clientVersion);

	if (context->protocolFlags & READY_FLAGS_DISABLE_TIMESTAMP_INJECTION)
		qDebug("don't take in account the timestamps\n");

	return CHANNEL_RC_OK;
}

static QTouchDevice::Capabilities scanCapabilitiesInTouchData(const RDPINPUT_TOUCH_EVENT *touchEvent) {
	QTouchDevice::Capabilities ret = 0;

	for (UINT16 i = 0; i < touchEvent->frameCount; i++) {
		RDPINPUT_TOUCH_FRAME *frame = &touchEvent->frames[i];

		for (UINT32 j = 0; j < frame->contactCount; j++) {
			RDPINPUT_CONTACT_DATA *data = &frame->contacts[j];

			if (data->fieldsPresent & CONTACT_DATA_CONTACTRECT_PRESENT)
				ret |= QTouchDevice::Area;

			if (data->fieldsPresent & CONTACT_DATA_PRESSURE_PRESENT)
				ret |= QTouchDevice::Pressure;

			// TODO: Shall we do something from CONTACT_DATA_ORIENTATION_PRESENT ?
		}
	}

	return ret;
}


Qt::TouchPointState QOgonTouch::stateFromTouchData(RDPINPUT_CONTACT_DATA *data) {
	if (data->contactFlags & CONTACT_FLAG_UP)
		return Qt::TouchPointReleased;

	if (data->contactFlags & CONTACT_FLAG_DOWN)
		return Qt::TouchPointPressed;

	return Qt::TouchPointStationary;
}

void QOgonTouch::treatTouchPointsDiff(QOgonTouch::TouchPointList &touchPoints) {
	/* add the contacts that were there the last time and haven't changed since */
	for (int i = 0; i < mPrevTouchPoints.count(); ++i) {
		const QWindowSystemInterface::TouchPoint &prevPoint = mPrevTouchPoints.at(i);
		if (prevPoint.state == Qt::TouchPointReleased)
			continue;

		bool found = false;
		for (TouchPointList::iterator tp = touchPoints.begin(); tp != touchPoints.end(); ++tp) {
			if (tp->id == prevPoint.id) {
				found = true;
				if (tp->normalPosition != prevPoint.normalPosition) {
					tp->state = Qt::TouchPointMoved;
				}
				break;
			}
		}

		if (!found) {
			QWindowSystemInterface::TouchPoint p = prevPoint;
			p.state = Qt::TouchPointStationary;
			touchPoints.append(p);
		}
	}
}


void QOgonTouch::treatTouchEvent(const RDPINPUT_TOUCH_EVENT *touchEvent) {
	if (!mDevice) {
		/* The touch device is created on the fly as we set the capabilities from the
		 * first received touch event */
		mDevice = new QTouchDevice();
		mDevice->setType(QTouchDevice::TouchScreen);
		mDevice->setCapabilities( scanCapabilitiesInTouchData(touchEvent) );
		QWindowSystemInterface::registerTouchDevice(mDevice);
	}


	//qDebug("################## Event encodeTime=%d ===================", touchEvent->encodeTime);
	mTimestamp += touchEvent->encodeTime;
	for (UINT16 i = 0; i < touchEvent->frameCount; i++) {
		RDPINPUT_TOUCH_FRAME *frame = &touchEvent->frames[i];
		QWindow *targetWindow = NULL;
		TouchPointList touchPoints;

		mTimestamp += frame->frameOffset / 1000;
		//qDebug("========== Frame offset=%lld(%lld) ===================", frame->frameOffset, touchEvent->encodeTime + frame->frameOffset);
		for (UINT32 j = 0; j < frame->contactCount; j++) {
			RDPINPUT_CONTACT_DATA *data = &frame->contacts[j];

			QWindowSystemInterface::TouchPoint tp;
			tp.id = data->contactId;
			tp.state = stateFromTouchData(data);
			tp.flags = (QTouchEvent::TouchPoint::InfoFlags)0;

			if (data->fieldsPresent & CONTACT_DATA_PRESSURE_PRESENT)
				tp.pressure = data->pressure / 65000.0;
			else
				tp.pressure = 1.0;

			if (data->fieldsPresent & CONTACT_DATA_CONTACTRECT_PRESENT) {
				tp.area = QRect(
						QPoint(data->x + data->contactRectLeft, data->y + data->contactRectTop),
						QPoint(data->x + data->contactRectRight, data->y + data->contactRectBottom)
				);
			} else {
				tp.area = QRect(QPoint(data->x, data->y), QSize(1, 1));
			}

			//qDebug("tp %d: x=%d y=%d flags=0x%x", data->contactId, data->x, data->y, data->contactFlags);
			tp.normalPosition.setX(data->x);
			tp.normalPosition.setY(data->y);

			touchPoints.append(tp);

			QWindow *window = mWm->getWindowAt(QPoint(data->x, data->y));
			if (window)
				targetWindow = window;
		}

		treatTouchPointsDiff(touchPoints);
		mPrevTouchPoints = touchPoints;

		if (!targetWindow) {
			QOgonAbstractWindow *ogonWindow = mWm->getActiveWindow();
			if (ogonWindow)
				targetWindow = ogonWindow->window();
		}

		if (!targetWindow) {
			qWarning("%s: no window to send touch point events", __FUNCTION__);
			continue;
		}

		QWindowSystemInterface::handleTouchEvent(targetWindow, mTimestamp, mDevice, touchPoints);
	}
}

UINT QOgonTouch::onTouchEvent(RdpeiServerContext *context, const RDPINPUT_TOUCH_EVENT *touchEvent) {
	QOgonTouch *touch = static_cast<QOgonTouch *>(context->user_data);
	touch->treatTouchEvent(touchEvent);
	return CHANNEL_RC_OK;
}

UINT QOgonTouch::onTouchReleased(RdpeiServerContext *context, BYTE contactId) {
	QOgonTouch *touch = static_cast<QOgonTouch *>(context->user_data);
	// TODO
	return CHANNEL_RC_OK;
}

UINT QOgonTouch::onPenEvent(RdpeiServerContext *context, const RDPINPUT_PEN_EVENT *penEvent) {
	QOgonTouch *touch = static_cast<QOgonTouch *>(context->user_data);
	return CHANNEL_RC_OK;
}
