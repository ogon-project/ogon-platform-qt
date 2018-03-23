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

#ifndef __QOGON_H___
#define __QOGON_H___

#include <QString>

#define QOGON_RESSOURCE_STRING "ogon API"

/** @brief the result of an SBP call */
enum {
	SBPCALL_SUCCESS = 0,
	SBPCALL_NOT_FOUND = 1,
	SBPCALL_TRANSPORT = 2,
	SBPCALL_TIMEOUT = 3,
	SBPCALL_UNKNOWN_ERROR = 200,
};

/**
 *	@brief interface that application may use to communicate with the QOgon backend
 */
struct QOgonInterface {
	/** dtor */
	virtual ~QOgonInterface() { }

	/** @return the interface as a QObject */
	virtual QObject *asQObject() = 0;

	/** performs an SBP call with the given protobuf content
	 * @param msgType the kind of message that is sent
	 * @param request the message protobuf encoded
	 * @return an id identifying the request message
	 */
	virtual quint32 sbpCall(quint32 msgType, const QByteArray &request) = 0;

	/** tries to guess a locale from the keyboard layout, this may be useful for
	 * localization of the user interface according to the remote locale
	 *
	 * @return the computed locale or an empty string if none were found pertinent
	 */
	virtual QString guessLocaleFromKeyboard() const = 0;

	/** @return the size of the screen from the RDP point of view */
	virtual QSize getScreenSize() const = 0;

	/** instructs Ogon to resize the remotePeer
	 * @param newSize the new size to set
	 */
	virtual void changeScreenSize(const QSize &newSize) = 0;

	/** signals
	 * @{ */
signals:
	virtual void ogonConnectionEstablished() = 0;
	virtual void ogonScreenReady() = 0;
	virtual void ogonConnectionLost() = 0;
	virtual void ogonPartialCredentials(const QString &login, const QString &domain) = 0;
	virtual void ogonSbpReply(quint32 tagId, quint32 sbpType, quint32 status,
			const QByteArray &reply) = 0;
	/** @} */
};

#define QOgonInterface_iid "com.ogon.QOgonInterface"

Q_DECLARE_INTERFACE(QOgonInterface, QOgonInterface_iid)


#endif // __QOGON_H___
