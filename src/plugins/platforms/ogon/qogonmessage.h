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

#ifndef __QOGON_MESSAGE_H__
#define __QOGON_MESSAGE_H__

#include <QObject>
#include <QTimer>
#include <QKeyEvent>

#include <ogon/backend.h>
#include <ogon-message/ogon-message.h>

/**
 * @brief
 */
class QOgonMessageBox : public QOgonMessageBoxBase {
	Q_OBJECT

public:

	QOgonMessageBox(uint id, uint type, uint style, uint timeout,
			QString para1, QString para2, QString para3,
			QString para4, QString para5);

	virtual ~QOgonMessageBox();

	virtual void sendResponse();
	virtual void showMessagebox();

signals:
	void sendResult(ogon_message *rdpMsg, bool deleteMessage);

};

#endif //__QOGON_MESSAGE_H__
