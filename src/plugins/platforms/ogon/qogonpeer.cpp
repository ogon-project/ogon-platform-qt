/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * David Fort <contact@hardening-consulting.com>
 * Martin Haimberger <martin.haimberger@thincast.com>
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

#include <QByteArray>
#include <QtNetwork/QLocalSocket>
#include <QtGui/QScreen>

#include <freerdp/locale/keyboard.h>

#include <ogon/backend.h>
#include <ogon/version.h>
#include "qogonmessage.h"

#include "qogonpeer.h"
#include "qogonkeyboard.h"
#include "qogontouch.h"
#include "qogonwindowmanager.h"
#include "qogonscreen.h"

#ifdef Q_OS_UNIX
#include "qogonshmframebuffer.h"
#include <unistd.h>
#include <sys/types.h>
#else
#warning "WIN32 not handled yet"
#endif

#ifdef BUILD_XCURSORS
#include "xcursors/qogonxcursor.h"
#endif

#define OGON_COMMON_LENGTH 6

/**
 * @brief a utility class to map a wStream on a QByteArray
 */
class WStreamByteArray {
public:
	WStreamByteArray(quint32 sizeTip = 0) {
		if (!sizeTip)
			sizeTip = 0xffff;
		mByteArray.resize(sizeTip);
		mStream = Stream_New((BYTE *)mByteArray.data(), sizeTip);

	}
	~WStreamByteArray() {
		Stream_Free(mStream, FALSE);
	}

	wStream *stream() const { return mStream; }

	QByteArray &seal() {
		mByteArray.resize( Stream_GetPosition(mStream) );
		return mByteArray;
	}

protected:
	QByteArray mByteArray;
	wStream *mStream;
};

/**
 * @brief private data of a Ogon peer
 */
struct QOgonPeerPriv {
	/** @brief state of the module protocol automata */
	enum AutomataState {
		STREAM_WAITING_COMMON_HEADER,
		STREAM_WAITING_DATA
	};

	QOgonPeerPriv() :
			stream_in(Stream_New(0, 0xffff)),
			currentMessageType(0),
			expected_bytes(OGON_COMMON_LENGTH),
			state(STREAM_WAITING_COMMON_HEADER),
			buttons_state(Qt::NoButton),
#ifdef Q_OS_UNIX
			shm(new QOgonShmFramebuffer())
#endif
	{
		memset(&rds_fb_info, 0, sizeof(rds_fb_info));
		rds_fb_info.bitsPerPixel = 32;
		rds_fb_info.bytesPerPixel = 4;
#ifdef Q_OS_UNIX
		rds_fb_info.userId = (UINT32)getuid();
#endif
		rds_fb_info.multiseatCapable = FALSE;

		memset(&rds_set_system_pointer, 0, sizeof(rds_set_system_pointer));
		rds_set_system_pointer.ptrType = SYSPTR_DEFAULT;

		memset(&rds_sync_reply, 0, sizeof(rds_sync_reply));

	}

	~QOgonPeerPriv() {
		Stream_Free(stream_in, TRUE),
		delete shm;
	}

	wStream *stream_in;
	UINT16 currentMessageType;
	qint64 expected_bytes;
	AutomataState state;
	Qt::MouseButtons buttons_state;
	QPoint lastMousePos;

	ogon_msg_framebuffer_info rds_fb_info;
	ogon_msg_framebuffer_sync_reply rds_sync_reply;
	ogon_msg_set_system_pointer rds_set_system_pointer;

#ifdef Q_OS_UNIX
	QOgonShmFramebuffer *shm;
#else
#warning "WIN32 not handled yet"
#endif
};


quint32 QOgonPeer::sTagCounter = 0;

QOgonPeer::QOgonPeer(QOgonWindowManager *wm, QLocalSocket *sock) :
	mWindowManager(wm),
	mKeyboard(new QOgonKeyboard(wm)),
	mTouch(new QOgonTouch(wm)),
	mSock(sock),
	mPendingFrame(false),
	mPendingShmId(-1),
	mPreviousShmId(-1),
	mScreenReadySignaled(false),
	mAlive(true),
	priv(new QOgonPeerPriv())
{
	ogon_msg_version version;
	version.versionMajor = OGON_PROTOCOL_VERSION_MAJOR;
	version.versionMinor = OGON_PROTOCOL_VERSION_MINOR;

	QByteArray backendCookie = qgetenv("OGON_BACKEND_COOKIE");
	if (!backendCookie.isNull()) {
		version.cookie = strdup(backendCookie.constData());
		if (!version.cookie)
			qFatal("unable to duplicate backend cookie");
	} else {
		version.cookie = NULL;
	}

	WStreamByteArray msg;
	if (!ogon_message_send(msg.stream(), OGON_SERVER_VERSION_REPLY, (ogon_message *)&version)) {
		qFatal("failed to write version message to stream");
	}

	mSock->write(msg.seal());

	free(version.cookie);

	connect(mSock, SIGNAL(readyRead()), this, SLOT(incomingBytes()));
	connect(mSock, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

QOgonPeer::~QOgonPeer() {
	disconnect(mSock, SIGNAL(readyRead()));
	delete priv;

	mSock->close();
	delete mSock;

	delete mKeyboard;
}

void QOgonPeer::resize(const QRect &geometry) {
	qDebug("QOgonPeer::resize(%d,%d - %dx%d)", geometry.left(), geometry.top(),
			geometry.width(), geometry.height());

	if(priv->shm->geometry() != geometry) {
		priv->shm->resize(geometry);

		ogon_msg_framebuffer_info *fbInfoMsg = &priv->rds_fb_info;
		fbInfoMsg->width = geometry.width();
		fbInfoMsg->height = geometry.height();
		fbInfoMsg->scanline = priv->shm->scanline();

		sendSharedFramebuffer();
		mPendingFrame = false;
		mPreviousShmId = mPendingShmId;
		mPendingShmId = -1;
		mWindowManager->mScreen->setGeometry(geometry);
	}

}

void QOgonPeer::sendSharedFramebuffer() {
	WStreamByteArray msg;
	if (!ogon_message_send(msg.stream(), OGON_SERVER_FRAMEBUFFER_INFO, (ogon_message *)&priv->rds_fb_info)) {
		qFatal("failed to write framebuffer info message to stream");
	}
	mSock->write(msg.seal());
}

void QOgonPeer::incomingBytes() {
	quint64 ret;

	while(priv->expected_bytes <= mSock->bytesAvailable()) {
		ret = mSock->read((char *)Stream_Pointer(priv->stream_in), priv->expected_bytes);
		if(ret <= 0) {
			qWarning("Ogon connection closed");
			return;
		}

		Stream_Seek(priv->stream_in, ret);
		priv->expected_bytes -= ret;

		if (priv->expected_bytes)
			continue;

		if (priv->state == QOgonPeerPriv::STREAM_WAITING_COMMON_HEADER) {
			UINT32 len;

			Stream_SetPosition(priv->stream_in, 0);
			ogon_read_message_header(priv->stream_in, &priv->currentMessageType, &len);
			priv->expected_bytes = len;

			Stream_SetPosition(priv->stream_in, 0);
			if (priv->expected_bytes) {
				Stream_EnsureRemainingCapacity(priv->stream_in, len);
				priv->state = QOgonPeerPriv::STREAM_WAITING_DATA;
				continue;
			}
		}

		ogon_message clientMessage;

		Stream_SealLength(priv->stream_in);
		Stream_SetPosition(priv->stream_in, 0);

		if (!ogon_message_read(priv->stream_in, priv->currentMessageType, &clientMessage)) {
			qWarning("%s: invalid message type %d", __FUNCTION__, priv->currentMessageType);
			return;
		}

		treat_message(priv->currentMessageType, &clientMessage);

		ogon_message_free(priv->currentMessageType, &clientMessage, true);
		Stream_SetPosition(priv->stream_in, 0);
		priv->expected_bytes = OGON_COMMON_LENGTH;
		priv->state = QOgonPeerPriv::STREAM_WAITING_COMMON_HEADER;
	}
}

void QOgonPeer::disconnected() {
	mAlive = false;
	mKeyboard->resetKeyboard();
	mTouch->disable();

	mWindowManager->notifyPeerClosed();
}

void QOgonPeer::updateMouseButtonsFromFlags(DWORD flags, bool extended) {
	Qt::MouseButtons buttons = Qt::NoButton;

	if(!extended) {
		if (flags & PTR_FLAGS_BUTTON1)
			buttons |= Qt::LeftButton;
		else if (flags & PTR_FLAGS_BUTTON2)
			buttons |= Qt::RightButton;
		else if (flags & PTR_FLAGS_BUTTON3)
			buttons |= Qt::MiddleButton;
	} else {
		qWarning("extended mouse button not handled");
	}

	if (flags & PTR_FLAGS_DOWN)
		priv->buttons_state |= buttons;
	else
		priv->buttons_state &= (~buttons);
}


quint32 convertStatusCode(SBP_REPLY_STATUS status) {
	switch(status) {
	case SBP_REPLY_SUCCESS:
		return SBPCALL_SUCCESS;
	case SBP_REPLY_TRANSPORT_ERROR:
		return SBPCALL_TRANSPORT;
	case SBP_REPLY_TIMEOUT:
		return SBPCALL_TIMEOUT;
	case SBP_REPLY_NOT_FOUND:
		return SBPCALL_NOT_FOUND;
	default:
		return SBPCALL_UNKNOWN_ERROR;
	}
}

int QOgonPeer::msReadyFrames = 0;
int QOgonPeer::msPendingFrames = 0;

void QOgonPeer::treat_message(UINT32 msgType, ogon_message *msg) {
	QString user;
	QString domain;
	QRect peerGeometry;
	QPoint pos;
	QRegion extraDamage;
	int wheelDelta = 0;
	QOgonCursor *cursor;
	QOgonMessageBox *msgbox = 0;

	//qDebug("message of type %d", msgType);
	switch(msgType) {
	case OGON_CLIENT_CAPABILITIES:
		qDebug("%s: layout=0x%x type=0x%x subtype=0x%x", __FUNCTION__, msg->capabilities.keyboardLayout,
				msg->capabilities.keyboardType,
				msg->capabilities.keyboardSubType);
		mKeyboard->reconfigureKeyboard(msg->capabilities.keyboardLayout, msg->capabilities.keyboardType);
		if (!mTouch->init()) {
			qDebug("multitouch channel not available");
		}

		peerGeometry.setRect(0, 0, msg->capabilities.desktopWidth, msg->capabilities.desktopHeight);
		mDamagedRegion = peerGeometry;
		priv->shm->resize(peerGeometry);
		priv->rds_fb_info.width = peerGeometry.width();
		priv->rds_fb_info.height = peerGeometry.height();
		priv->rds_fb_info.scanline = priv->shm->scanline();
		sendSharedFramebuffer();
		if (peerGeometry != mWindowManager->geometry()) {
			qDebug("resizing screen from %dx%d to %dx%d", mWindowManager->geometry().width(),
					mWindowManager->geometry().height(), peerGeometry.width(), peerGeometry.height());
			mWindowManager->mScreen->setGeometry(peerGeometry);
		}
#ifdef BUILD_XCURSORS
		cursor = (QOgonCursor *)mWindowManager->mScreen->cursor();
		if (cursor)
			cursor->restoreLastCursor();
#endif
		break;

	case OGON_CLIENT_MOUSE_EVENT:
		updateMouseButtonsFromFlags(msg->mouse.flags, false);
		pos = QPoint(msg->mouse.x, msg->mouse.y);
		if (msg->mouse.flags & PTR_FLAGS_WHEEL) {
			// wheel event have x and y set to 0 and we need to know to which window
			// send the erdpvent, so use the last mouse position
			pos = priv->lastMousePos;

			wheelDelta = (10 * (msg->mouse.flags & 0xff)) / 120;
			if (msg->mouse.flags & PTR_FLAGS_WHEEL_NEGATIVE)
				wheelDelta = -wheelDelta;
		} else {
			priv->lastMousePos = pos;
		}

		mWindowManager->handleMouseEvent(pos, priv->buttons_state, wheelDelta);
		break;

	case OGON_CLIENT_EXTENDED_MOUSE_EVENT:
		updateMouseButtonsFromFlags(msg->extendedMouse.flags, true);
		pos = QPoint(msg->extendedMouse.x, msg->extendedMouse.y);
		mWindowManager->handleMouseEvent(pos, priv->buttons_state, wheelDelta);
		break;

	case OGON_CLIENT_SCANCODE_KEYBOARD_EVENT:
		mKeyboard->updateStateFromScancode(msg->scancodeKeyboard.keyboardType, msg->scancodeKeyboard.flags,
											msg->scancodeKeyboard.code);
		break;

	case OGON_CLIENT_SYNCHRONIZE_KEYBOARD_EVENT:
		mKeyboard->resetKeyboard();
		mKeyboard->updateModifiersState(msg->synchronizeKeyboard.flags & KBD_SYNC_CAPS_LOCK,
				msg->synchronizeKeyboard.flags & KBD_SYNC_NUM_LOCK,
				msg->synchronizeKeyboard.flags & KBD_SYNC_SCROLL_LOCK,
				msg->synchronizeKeyboard.flags & KBD_SYNC_KANA_LOCK
		);
		break;

	case OGON_CLIENT_UNICODE_KEYBOARD_EVENT:
		mKeyboard->updateStateFromUnicode(msg->unicodeKeyboard.flags, msg->unicodeKeyboard.code);
		break;

	case OGON_CLIENT_FRAMEBUFFER_SYNC_REQUEST:
		if (!mScreenReadySignaled) {
			mScreenReadySignaled = true;
			emit mWindowManager->ogonScreenReady();
		}

		mPendingFrame = true;
		mPendingShmId = msg->framebufferSyncRequest.bufferId;
		if (!mDamagedRegion.isEmpty()) {
			msReadyFrames++;
			//qWarning("%s: sending ready frame (ready=%d pending=%d)", __FUNCTION__, msReadyFrames, msPendingFrames);
			if (mPendingShmId != mPreviousShmId)
				sendSyncReply(mPendingShmId);
			else
				qWarning("ignoring sync request for previous shmId");
		}
		break;

	case OGON_CLIENT_IMMEDIATE_SYNC_REQUEST:
		mPendingFrame = true;
		mPendingShmId = msg->framebufferSyncRequest.bufferId;
		if (mPendingShmId != mPreviousShmId)
			sendSyncReply(mPendingShmId);
		else
			qWarning("ignoring immediate sync request for previous shmId");
		break;

	case OGON_CLIENT_SBP_REPLY: {
		QByteArray payload(msg->sbpReply.data, msg->sbpReply.dataLen);
		emit mWindowManager->ogonSbpReply(msg->sbpReply.tag, msg->sbpReply.sbpType,
				convertStatusCode(msg->sbpReply.status),
				payload
		);
		break;
	}
	case OGON_CLIENT_SEAT_NEW:
	case OGON_CLIENT_SEAT_REMOVED:
		/* We don't do anything of this right now */
		break;
	case OGON_CLIENT_VERSION: {
		if (msg->version.versionMajor != OGON_PROTOCOL_VERSION_MAJOR) {
			qWarning("%s: Version info mismatch, version %d received, but mismatches with version %d",
				__FUNCTION__, msg->version.versionMajor, OGON_PROTOCOL_VERSION_MAJOR);
			disconnected();
		}

		if (!msg->version.cookie) {
			qWarning("%s: Ogon doesn't send any cookie", __FUNCTION__);
			disconnected();
		}

		QByteArray expected = qgetenv("OGON_COOKIE");
		if (!expected.length()) {
			qWarning("%s: strange the sessionManager haven't set a cookie to check", __FUNCTION__);
		} else if (expected != msg->version.cookie) {
			qWarning("%s: wrong Ogon cookie %s (expecting %s)", __FUNCTION__, msg->version.cookie,
					expected.constData());
			disconnected();
		}

		break;
	}

	case OGON_CLIENT_MESSAGE:
		msgbox = new QOgonMessageBox(msg->message.message_id, msg->message.message_type,
			msg->message.style, msg->message.timeout,
			msg->message.parameter1_len ? msg->message.parameter1 : "",
			msg->message.parameter2_len ? msg->message.parameter2 : "",
			msg->message.parameter3_len ? msg->message.parameter3 : "",
			msg->message.parameter4_len ? msg->message.parameter4 : "",
			msg->message.parameter5_len ? msg->message.parameter5 : "");
		connect(msgbox, SIGNAL(sendResult(ogon_message *, bool)), this, SLOT(sendMessage(ogon_message *, bool)));
		msgbox->start();
		break;

	default:
		qWarning("not handled yet, %d", msgType);
		break;
	}
}


void QOgonPeer::sendSyncReply(int shmId) {
	WStreamByteArray msg;

	if (!priv->shm->updateDamage(mWindowManager->image(), mDamagedRegion, shmId))
		qFatal("%s: unable to updateDamage", __FUNCTION__);

	priv->rds_sync_reply.bufferId = shmId;
	if (!ogon_message_send(msg.stream(), OGON_SERVER_FRAMEBUFFER_SYNC_REPLY, (ogon_message *)&priv->rds_sync_reply)) {
		qWarning("%s: error writing SyncReply", __FUNCTION__);
		return;
	}

	mSock->write(msg.seal());

	mPendingFrame = false;
	mDamagedRegion = QRegion();
}


void QOgonPeer::addDamage(const QRegion &region) {
	mDamagedRegion += region;

	if (!mPendingFrame || mDamagedRegion.isEmpty())
		return;

	msPendingFrames++;
	//qDebug("%s: sending pending frame ready=%d pending=%d", __FUNCTION__, msReadyFrames, msPendingFrames);
	if (!priv->shm->updateDamage(mWindowManager->image(), mDamagedRegion, mPendingShmId)) {
		qWarning("%s: unable to updateDamage", __FUNCTION__);
	}
	sendSyncReply(mPendingShmId);
}

void QOgonPeer::sendMessage(ogon_message *rdpMsg, bool deleteMessage) {
	if (!sendBytes(OGON_SERVER_MESSAGE_REPLY, rdpMsg, deleteMessage)) {
		qWarning("%s: unable to send message reply", __FUNCTION__);
	}
}

BOOL QOgonPeer::sendBytes(UINT16 type, ogon_message *rdpMsg, bool deleteMessage) {
	WStreamByteArray msg;

	if (!ogon_message_send(msg.stream(), type, rdpMsg)) {
		qWarning("%s: error writing message type %d", __FUNCTION__, type);
		if (deleteMessage) {
			delete rdpMsg;
		}
		return FALSE;
	}

	mSock->write(msg.seal());
	if (deleteMessage) {
		delete rdpMsg;
	}
	return TRUE;
}


quint32 QOgonPeer::sbpCall(quint32 typeSbp, const QByteArray &msg) {
	ogon_msg_sbp_request sbp;
	sbp.tag = ++sTagCounter;
	sbp.sbpType = typeSbp;
	sbp.dataLen = msg.size();
	sbp.data = (char *)msg.data();

	WStreamByteArray streamBa;

	if (!ogon_message_send(streamBa.stream(), OGON_SERVER_SBP_REQUEST, (ogon_message *)&sbp)) {
		qFatal("failed to write SBP request to stream");
	}

	mSock->write(streamBa.seal());
	return sbp.tag;
}

