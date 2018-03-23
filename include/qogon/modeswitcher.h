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

#ifndef __QOGON_MODESWITCHER_H___
#define __QOGON_MODESWITCHER_H___

#include <QtWidgets/QComboBox>

struct QOgonInterface;
/**
 * @brief this widget allows to switch the resolution when running under Ogon
 */
class QOgonModeSwitcher : public QComboBox {
	Q_OBJECT
public:
	/**
	 * @param parent
	 */
	QOgonModeSwitcher(QWidget *parent = 0);

	/**
	 *
	 * @param modes
	 * @param selected
	 */
	void updateModes(const QList<QSize> &modes, int selected);

protected slots:
	void on_currentIndexChanged(int newIndex);
	void on_editTextChanged(const QString &text);
	void onScreenReady();

protected:
	QSize parseMode(const QString &str, bool *ok);

protected:
	QList<QSize> mModes;
	QOgonInterface *mOgon;
};

#endif /* __QOGONMODESWITCHER_H___ */
