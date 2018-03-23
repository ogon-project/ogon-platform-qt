/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
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

#ifndef __QOGON_KEYBOARD_H__
#define __QOGON_KEYBOARD_H__

#include <QString>
#include <QSet>

#ifndef NO_XKB_SUPPORT
#include <xkbcommon/xkbcommon.h>
#endif

class QOgonWindowManager;

/** @brief */
class QOgonKeyboard {
public:
	/**
	 * @param windowManager
	 */
	QOgonKeyboard(QOgonWindowManager *windowManager);

	/**
	 * @param keyboardLayout
	 * @param keyboardType
	 * @return
	 */
	bool reconfigureKeyboard(quint32 keyboardLayout, quint32 keyboardType);

	/**
	 *
	 * @param capsLock
	 * @param numLock
	 * @param scrollLock
	 * @param kanaLock
	 */
	void updateModifiersState(bool capsLock, bool numLock, bool scrollLock, bool kanaLock);

	/**
	 * @param keyboardType
	 * @param flags
	 * @param scancode
	 */
	void updateStateFromScancode(quint32 keyboardType, quint32 flags, quint32 scancode);

	/**
	 * @param flags
	 * @param keycode
	 */
	void updateStateFromXkbKeycode(quint32 flags, quint32 keycode);

	/**
	 * @param flags
	 * @param ucode
	 */
	void updateStateFromUnicode(quint32 flags, quint32 ucode);

	/** Resets the keyboard state, all the down keys are set up again */
	void resetKeyboard();

	/** tries to compute a locale name from the keyboard layout
	 * @return
	 */
	QString computeLocale() const;

protected:
	QOgonWindowManager *mWindowManager;
	quint32 mKeytime;
	quint32 mLayout;
	quint32 mKeyboardType;
	QString mLocale;
	QSet<quint32> mDownKeys;

#ifndef NO_XKB_SUPPORT
	struct xkb_context *mXkbContext;
	struct xkb_keymap *mXkbKeymap;
	struct xkb_state *mXkbState;

	xkb_mod_index_t mCapsLockModIndex;
	xkb_mod_index_t mNumLockModIndex;
	xkb_mod_index_t mScrollLockModIndex;
#endif
};


#endif /* __QOGON_KEYBOARD_H__ */
