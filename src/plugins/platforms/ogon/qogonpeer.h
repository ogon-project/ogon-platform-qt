/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * David Fort <contact@hardening-consulting.com>
 * Martin Haimberger <martin.haimberger@thincast.com>
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

#ifndef __QOGON_PEER_H__
#define __QOGON_PEER_H__

#include <QObject>
#include <QRegion>
#include <freerdp/types.h>

class QLocalSocket;
class QOgonWindowManager;
class QOgonKeyboard;
class QOgonTouch;
struct QOgonPeerPriv;
typedef struct _ogon_msg_common ogon_msg_common;
typedef union _ogon_message ogon_message;

/**
 * @brief
 */
class QOgonPeer : public QObject {
	Q_OBJECT

	friend class QOgonWindowManager;
public:
	/**
	 *
	 * @param wm
	 * @param sock
	 */
	QOgonPeer(QOgonWindowManager *wm, QLocalSocket *sock);

	/** destructor */
	~QOgonPeer();

	void resize(const QRect &geometry);

	void addDamage(const QRegion &region);

	bool isAlive() const { return mAlive; }

	/** OgonInterface function
	 * @{ */
	quint32 sbpCall(quint32 typeSbp, const QByteArray &msg);
	/** @} */

public slots:
	void incomingBytes();
	void disconnected();
	void sendMessage(ogon_message *rdpMsg, bool deleteMessage = false);

protected:
	BOOL sendBytes(UINT16 type, ogon_message *rdpMsg, bool deleteMessage = false);
	void treat_message(quint32 msgType, ogon_message *msg);
	void sendSharedFramebuffer();
	void updateMouseButtonsFromFlags(DWORD flags, bool extended);
	void sendSyncReply(int shmId);

protected:
	static quint32 sTagCounter;
	QOgonWindowManager *mWindowManager;
	QOgonKeyboard *mKeyboard;
	QOgonTouch *mTouch;

	QLocalSocket *mSock;
	QRegion mDamagedRegion;
	bool mPendingFrame;
	int mPendingShmId;
	int mPreviousShmId;
	bool mScreenReadySignaled;
	bool mAlive;

	QOgonPeerPriv *priv;

	static int msReadyFrames;
	static int msPendingFrames;
};



#endif /* __QOGON_PEER_H_ */
