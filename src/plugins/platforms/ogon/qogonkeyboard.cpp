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

#include "qogonkeyboard.h"
#include "qogonplatform.h"
#include "qogonwindowmanager.h"
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <private/qguiapplication_p.h>

#include <freerdp/input.h>
#include <freerdp/locale/keyboard.h>

#include <ogon/backend.h>

#ifndef NO_XKB_SUPPORT
#include <X11/keysym.h>
#include <xkbcommon/xkbcommon.h>


struct rdp_to_xkb_keyboard_layout {
	UINT32 rdpLayoutCode;
	const char *xkbLayout;
	const char *xkbVariant;
	const char *localeName;
};

/* table reversed from
 	 https://github.com/awakecoding/FreeRDP/blob/master/libfreerdp/locale/xkb_layout_ids.c#L811 */
static
struct rdp_to_xkb_keyboard_layout rdp_keyboards[] = {
		{KBD_ARABIC_101, "ara", 0, 0},
		{KBD_BULGARIAN, "bg", 0, 0},
		{KBD_CHINESE_TRADITIONAL_US, 0, 0, 0},
		{KBD_CZECH, "cz", "bksl", 0},
		{KBD_CZECH_QWERTY, "cz", "qwerty", 0},
		{KBD_DANISH, "dk", 0, 0},
		{KBD_GERMAN, "de", 0, "de_DE"},
		{KBD_GERMAN_NEO, "de", "neo", "de_DE"},
		{KBD_GERMAN_IBM, "de", "qwerty", "de_DE"},
		{KBD_GREEK, "gr", 0, 0},
		{KBD_GREEK_220, "gr", "simple", 0},
		{KBD_GREEK_319, "gr", "extended", 0},
		{KBD_GREEK_POLYTONIC, "gr", "polytonic", 0},
		{KBD_US, "us", 0, "en_US"},
		{KBD_SPANISH, "es", 0, 0},
		{KBD_SPANISH_VARIATION, "es", "nodeadkeys", 0},
		{KBD_FINNISH, "fi", 0, 0},
		{KBD_FRENCH, "fr", "oss_latin9", "fr_FR"},
		{KBD_HEBREW, "il", 0, 0},
		{KBD_HUNGARIAN, "hu", 0, 0},
		{KBD_HUNGARIAN_101_KEY, "hu", "standard", 0},
		{KBD_ICELANDIC, "is", 0, 0},
		{KBD_ITALIAN, "it", 0, "it_IT"},
		{KBD_ITALIAN_142, "it", "nodeadkeys", "it_IT"},
		{KBD_JAPANESE, "jp", 0, 0},
		{KBD_JAPANESE_INPUT_SYSTEM_MS_IME2002, "jp", "kana", 0},
		{KBD_KOREAN, "kr", 0, 0},
		{KBD_KOREAN_INPUT_SYSTEM_IME_2000, "kr", "kr104", 0},
		{KBD_DUTCH, "nl", 0, 0},
		{KBD_NORWEGIAN, "no", 0, 0},
		{KBD_POLISH_PROGRAMMERS, "pl", 0, 0},
		{KBD_POLISH_214, "pl", "qwertz", 0},
//		{KBD_PORTUGUESE_BRAZILIAN_ABN0416, 0, 0, 0},
		{KBD_ROMANIAN, "ro", 0, 0},
		{KBD_RUSSIAN, "ru", 0, 0},
		{KBD_RUSSIAN_TYPEWRITER, "ru", "typewriter", 0},
		{KBD_CROATIAN, "hr", 0, 0},
		{KBD_SLOVAK, "sk", 0, 0},
		{KBD_SLOVAK_QWERTY, "sk", "qwerty", 0},
		{KBD_ALBANIAN, 0, 0, 0},
		{KBD_SWEDISH, "se", 0, 0},
		{KBD_THAI_KEDMANEE, "th", 0, 0},
		{KBD_THAI_KEDMANEE_NON_SHIFTLOCK, "th", "tis", 0},
		{KBD_TURKISH_Q, "tr", 0, 0},
		{KBD_TURKISH_F, "tr", "f", 0},
		{KBD_URDU, "in", "urd-phonetic3", 0},
		{KBD_UKRAINIAN, "ua", 0, 0},
		{KBD_BELARUSIAN, "by", 0, 0},
		{KBD_SLOVENIAN, "si", 0, 0},
		{KBD_ESTONIAN, "ee", 0, 0},
		{KBD_LATVIAN, "lv", 0, 0},
		{KBD_LITHUANIAN_IBM, "lt", "ibm", 0},
		{KBD_FARSI, "af", 0, 0},
		{KBD_VIETNAMESE, "vn", 0, 0},
		{KBD_ARMENIAN_EASTERN, "am", 0, 0},
		{KBD_AZERI_LATIN, 0, 0, 0},
		{KBD_FYRO_MACEDONIAN, "mk", 0, 0},
		{KBD_GEORGIAN, "ge", 0, 0},
		{KBD_FAEROESE, 0, 0, 0},
		{KBD_DEVANAGARI_INSCRIPT, 0, 0, 0},
		{KBD_MALTESE_47_KEY, 0, 0, 0},
		{KBD_NORWEGIAN_WITH_SAMI, "no", "smi", 0},
		{KBD_KAZAKH, "kz", 0, 0},
		{KBD_KYRGYZ_CYRILLIC, "kg", "phonetic", 0},
		{KBD_TATAR, "ru", "tt", 0},
		{KBD_BENGALI, "bd", 0, 0},
		{KBD_BENGALI_INSCRIPT, "bd", "probhat", 0},
		{KBD_PUNJABI, 0, 0, 0},
		{KBD_GUJARATI, "in", "guj", 0},
		{KBD_TAMIL, "in", "tam", 0},
		{KBD_TELUGU, "in", "tel", 0},
		{KBD_KANNADA, "in", "kan", 0},
		{KBD_MALAYALAM, "in", "mal", 0},
		{KBD_HINDI_TRADITIONAL, "in", 0, 0},
		{KBD_MARATHI, 0, 0, 0},
		{KBD_MONGOLIAN_CYRILLIC, "mn", 0, 0},
		{KBD_UNITED_KINGDOM_EXTENDED, "gb", "intl", 0},
		{KBD_SYRIAC, "syc", 0, 0},
		{KBD_SYRIAC_PHONETIC, "syc", "syc_phonetic", 0},
		{KBD_NEPALI, "np", 0, 0},
		{KBD_PASHTO, "af", "ps", 0},
		{KBD_DIVEHI_PHONETIC, 0, 0, 0},
		{KBD_LUXEMBOURGISH, 0, 0, 0},
		{KBD_MAORI, "mao", 0, 0},
		{KBD_CHINESE_SIMPLIFIED_US, 0, 0, 0},
		{KBD_SWISS_GERMAN, "ch", "de_nodeadkeys", 0},
		{KBD_UNITED_KINGDOM, "gb", 0, 0},
		{KBD_LATIN_AMERICAN, "latam", 0, 0},
		{KBD_BELGIAN_FRENCH, "be", 0, 0},
		{KBD_BELGIAN_PERIOD, "be", "oss_sundeadkeys", 0},
		{KBD_PORTUGUESE, "pt", 0, 0},
		{KBD_SERBIAN_LATIN, "rs", 0, 0},
		{KBD_AZERI_CYRILLIC, "az", "cyrillic", 0},
		{KBD_SWEDISH_WITH_SAMI, "se", "smi", 0},
		{KBD_UZBEK_CYRILLIC, "af", "uz", 0},
		{KBD_INUKTITUT_LATIN, "ca", "ike", 0},
		{KBD_CANADIAN_FRENCH_LEGACY, "ca", "fr-legacy", "fr_FR"}, // for now
		{KBD_SERBIAN_CYRILLIC, "rs", 0, 0},
		{KBD_CANADIAN_FRENCH, "ca", "fr-legacy", "fr_FR"},
		{KBD_SWISS_FRENCH, "ch", "fr", 0},
		{KBD_BOSNIAN, "ba", 0, 0},
		{KBD_IRISH, 0, 0, 0},
		{KBD_BOSNIAN_CYRILLIC, "ba", "us", 0},
		{KBD_UNITED_STATES_DVORAK, "us", "dvorak", 0},
		{KBD_PORTUGUESE_BRAZILIAN_ABNT2, "br", "nativo", 0},
		{KBD_CANADIAN_MULTILINGUAL_STANDARD, "ca", "multix", 0},
		{KBD_GAELIC, "ie", "CloGaelach", 0},

		{0x00000000, 0, 0, 0},
};

//
// these part was adapted from QtWayland, the original code can be retrieved
// from the git repository https://qt.gitorious.org/qt/qtwayland, files are:
//	* src/plugins/platforms/wayland_common/qwaylandinputdevice.cpp
//	* src/plugins/platforms/wayland_common/qwaylandkey.cpp
//
static Qt::KeyboardModifiers translateModifiers(xkb_state *state)
{
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    xkb_state_component cstate = xkb_state_component(XKB_STATE_DEPRESSED | XKB_STATE_LATCHED);

    if (xkb_state_mod_name_is_active(state, "Shift", cstate))
        ret |= Qt::ShiftModifier;
    if (xkb_state_mod_name_is_active(state, "Control", cstate))
        ret |= Qt::ControlModifier;
    if (xkb_state_mod_name_is_active(state, "Alt", cstate))
        ret |= Qt::AltModifier;
    if (xkb_state_mod_name_is_active(state, "Mod1", cstate))
        ret |= Qt::AltModifier;
    if (xkb_state_mod_name_is_active(state, "Mod4", cstate))
        ret |= Qt::MetaModifier;
    if (xkb_state_mod_name_is_active(state, "AltGr", cstate))
        ret |= Qt::GroupSwitchModifier;

    return ret;
}

static const uint32_t KeyTbl[] = {
    XK_Escape,                  Qt::Key_Escape,
    XK_Tab,                     Qt::Key_Tab,
    XK_ISO_Left_Tab,            Qt::Key_Backtab,
    XK_BackSpace,               Qt::Key_Backspace,
    XK_Return,                  Qt::Key_Return,
    XK_Insert,                  Qt::Key_Insert,
    XK_Delete,                  Qt::Key_Delete,
    XK_Clear,                   Qt::Key_Delete,
    XK_Pause,                   Qt::Key_Pause,
    XK_Print,                   Qt::Key_Print,

    XK_Home,                    Qt::Key_Home,
    XK_End,                     Qt::Key_End,
    XK_Left,                    Qt::Key_Left,
    XK_Up,                      Qt::Key_Up,
    XK_Right,                   Qt::Key_Right,
    XK_Down,                    Qt::Key_Down,
    XK_Prior,                   Qt::Key_PageUp,
    XK_Next,                    Qt::Key_PageDown,

    XK_Shift_L,                 Qt::Key_Shift,
    XK_Shift_R,                 Qt::Key_Shift,
    XK_Shift_Lock,              Qt::Key_Shift,
    XK_Control_L,               Qt::Key_Control,
    XK_Control_R,               Qt::Key_Control,
    XK_Meta_L,                  Qt::Key_Meta,
    XK_Meta_R,                  Qt::Key_Meta,
    XK_Alt_L,                   Qt::Key_Alt,
    XK_Alt_R,                   Qt::Key_Alt,
    XK_Caps_Lock,               Qt::Key_CapsLock,
    XK_Num_Lock,                Qt::Key_NumLock,
    XK_Scroll_Lock,             Qt::Key_ScrollLock,
    XK_Super_L,                 Qt::Key_Super_L,
    XK_Super_R,                 Qt::Key_Super_R,
    XK_Menu,                    Qt::Key_Menu,
    XK_Hyper_L,                 Qt::Key_Hyper_L,
    XK_Hyper_R,                 Qt::Key_Hyper_R,
    XK_Help,                    Qt::Key_Help,

    XK_KP_Space,                Qt::Key_Space,
    XK_KP_Tab,                  Qt::Key_Tab,
    XK_KP_Enter,                Qt::Key_Enter,
    XK_KP_Home,                 Qt::Key_Home,
    XK_KP_Left,                 Qt::Key_Left,
    XK_KP_Up,                   Qt::Key_Up,
    XK_KP_Right,                Qt::Key_Right,
    XK_KP_Down,                 Qt::Key_Down,
    XK_KP_Prior,                Qt::Key_PageUp,
    XK_KP_Next,                 Qt::Key_PageDown,
    XK_KP_End,                  Qt::Key_End,
    XK_KP_Begin,                Qt::Key_Clear,
    XK_KP_Insert,               Qt::Key_Insert,
    XK_KP_Delete,               Qt::Key_Delete,
    XK_KP_Equal,                Qt::Key_Equal,
    XK_KP_Multiply,             Qt::Key_Asterisk,
    XK_KP_Add,                  Qt::Key_Plus,
    XK_KP_Separator,            Qt::Key_Comma,
    XK_KP_Subtract,             Qt::Key_Minus,
    XK_KP_Decimal,              Qt::Key_Period,
    XK_KP_Divide,               Qt::Key_Slash,

    XK_ISO_Level3_Shift,        Qt::Key_AltGr,
    XK_Multi_key,               Qt::Key_Multi_key,
    XK_Codeinput,               Qt::Key_Codeinput,
    XK_SingleCandidate,         Qt::Key_SingleCandidate,
    XK_MultipleCandidate,       Qt::Key_MultipleCandidate,
    XK_PreviousCandidate,       Qt::Key_PreviousCandidate,

    XK_Mode_switch,             Qt::Key_Mode_switch,
    XK_script_switch,           Qt::Key_Mode_switch,

	 // dead keys
	XK_dead_grave, 				Qt::Key_Dead_Grave,
	XK_dead_acute, 				Qt::Key_Dead_Acute,
	XK_dead_circumflex, 		Qt::Key_Dead_Circumflex,
	XK_dead_tilde, 				Qt::Key_Dead_Tilde,
	XK_dead_macron, 			Qt::Key_Dead_Macron,
	XK_dead_breve, 				Qt::Key_Dead_Breve,
	XK_dead_abovedot, 			Qt::Key_Dead_Abovedot,
	XK_dead_diaeresis, 			Qt::Key_Dead_Diaeresis,
	XK_dead_abovering, 			Qt::Key_Dead_Abovering,
	XK_dead_doubleacute, 		Qt::Key_Dead_Doubleacute,
	XK_dead_caron, 				Qt::Key_Dead_Caron,
	XK_dead_cedilla, 			Qt::Key_Dead_Cedilla,
	XK_dead_ogonek, 			Qt::Key_Dead_Ogonek,
	XK_dead_iota, 				Qt::Key_Dead_Iota,
	XK_dead_voiced_sound, 		Qt::Key_Dead_Voiced_Sound,
	XK_dead_semivoiced_sound, 	Qt::Key_Dead_Semivoiced_Sound,
	XK_dead_belowdot, 			Qt::Key_Dead_Belowdot,
	XK_dead_hook, 				Qt::Key_Dead_Hook,
	XK_dead_horn, 				Qt::Key_Dead_Horn,

    0,                          0
};

static int keysymToQtKey(xkb_keysym_t key)
{
    int code = 0;
    int i = 0;
    while (KeyTbl[i]) {
        if (key == KeyTbl[i]) {
            code = (int)KeyTbl[i+1];
            break;
        }
        i += 2;
    }

    return code;
}

static int keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers &modifiers, const QString &text)
{
    int code = 0;

    if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F35) {
        code =  Qt::Key_F1 + (int(keysym) - XKB_KEY_F1);
    } else if (keysym >= XKB_KEY_KP_Space && keysym <= XKB_KEY_KP_9) {
        if (keysym >= XKB_KEY_KP_0) {
            // numeric keypad keys
            code = Qt::Key_0 + ((int)keysym - XKB_KEY_KP_0);
        } else {
            code = keysymToQtKey(keysym);
        }
        modifiers |= Qt::KeypadModifier;
    } else if (text.length() == 1 &&
               text.unicode()->unicode() > 0x1f &&
			   text.unicode()->unicode() != 0x7f &&
			!(keysym >= XKB_KEY_dead_grave && keysym <= XKB_KEY_dead_currency)) {
        code = text.unicode()->toUpper().unicode();
    } else {
        // any other keys
        code = keysymToQtKey(keysym);
    }

    return code;
}

#endif


QOgonKeyboard::QOgonKeyboard(QOgonWindowManager *windowManager) :
	mWindowManager(windowManager),
	mKeytime(0),
	mLayout(0xffffffff),
	mKeyboardType(0xffffffff)
#ifndef NO_XKB_SUPPORT
	,mXkbContext(0)
	,mXkbKeymap(0)
	,mXkbState(0)
	,mCapsLockModIndex(0)
	,mNumLockModIndex(0)
	,mScrollLockModIndex(0)
#endif
{
}

bool QOgonKeyboard::reconfigureKeyboard(quint32 layout, quint32 type) {
	if(layout == mLayout && type == mKeyboardType)
		return true;

#ifndef NO_XKB_SUPPORT
	struct xkb_rule_names xkbRuleNames;

	if(mXkbContext) {
		if(mXkbState) {
			xkb_state_unref(mXkbState);
			mXkbState = 0;
		}
		if(mXkbKeymap) {
			xkb_keymap_unref(mXkbKeymap);
			mXkbKeymap = 0;
		}
		xkb_context_unref(mXkbContext);
		mXkbContext = 0;
	}

	memset(&xkbRuleNames, 0, sizeof(xkbRuleNames));
	xkbRuleNames.rules = "evdev";

	switch(type) {
	case 7:
		xkbRuleNames.model = "jp106";
		break;
	default:
		xkbRuleNames.model = "pc105";
		break;
	}

	for(int i = 0; rdp_keyboards[i].rdpLayoutCode; i++) {
		if(rdp_keyboards[i].rdpLayoutCode == layout) {
			xkbRuleNames.layout = rdp_keyboards[i].xkbLayout;
			xkbRuleNames.variant = rdp_keyboards[i].xkbVariant;
			if(rdp_keyboards[i].localeName)
				mLocale = rdp_keyboards[i].localeName;
			else
				mLocale = QString::null;
			break;
		}
	}

	if(!xkbRuleNames.layout) {
		qWarning("%s: don't have a rule to match keyboard layout 0x%x, defaulting to us",
				__FUNCTION__, layout);
		xkbRuleNames.layout = "us";
		layout = KBD_US;
	}

	mXkbContext = xkb_context_new((xkb_context_flags)0);
	if(!mXkbContext) {
		qWarning("%s: unable to create a xkb_context", __FUNCTION__);
		return false;
	}

	mXkbKeymap = xkb_keymap_new_from_names(mXkbContext, &xkbRuleNames, (xkb_keymap_compile_flags)0);
	if(mXkbKeymap) {
		mXkbState = xkb_state_new(mXkbKeymap);
		if(!mXkbState) {
			qWarning("%s: unable to create a keymap state", __FUNCTION__);
			return false;
		}

		mCapsLockModIndex = xkb_map_mod_get_index(mXkbKeymap, XKB_MOD_NAME_CAPS);
		mNumLockModIndex = xkb_map_mod_get_index(mXkbKeymap, "Mod2");
		mScrollLockModIndex = xkb_map_mod_get_index(mXkbKeymap, "ScrollLock");
	} else {
		qWarning("%s: unable to load keymap %s", __FUNCTION__, xkbRuleNames.layout);
	}
#endif

	mLayout = layout;
	mKeyboardType = type;
	return true;
}


void QOgonKeyboard::updateModifiersState(bool capsLock, bool numLock, bool scrollLock, bool kanaLock) {
#ifndef NO_XKB_SUPPORT
	Q_UNUSED(kanaLock);

	uint32_t mods_depressed, mods_latched, mods_locked, group;
	int numMask, capsMask, scrollMask;

	mods_depressed = xkb_state_serialize_mods(mXkbState, xkb_state_component(XKB_STATE_DEPRESSED));
	mods_latched = xkb_state_serialize_mods(mXkbState, xkb_state_component(XKB_STATE_LATCHED));
	mods_locked = xkb_state_serialize_mods(mXkbState, xkb_state_component(XKB_STATE_LOCKED));
	group = xkb_state_serialize_group(mXkbState, xkb_state_component(XKB_STATE_EFFECTIVE));

	numMask = (1 << mNumLockModIndex);
	capsMask = (1 << mCapsLockModIndex);
	scrollMask = (1 << mScrollLockModIndex);

	mods_locked = capsLock ? (mods_locked | capsMask) : (mods_locked & ~capsMask);
	mods_locked = numLock ? (mods_locked | numMask) : (mods_locked & ~numLock);
	mods_locked = scrollLock ? (mods_locked | scrollMask) : (mods_locked & ~scrollMask);

	xkb_state_update_mask(mXkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);
#else
	Q_UNUSED(capsLock);
	Q_UNUSED(numLock);
	Q_UNUSED(scrollLock);
	Q_UNUSED(kanaLock);
#endif
}

void QOgonKeyboard::updateStateFromScancode(quint32 keyboardType, quint32 flags, quint32 scancode) {
	quint32 evdevcode = ogon_rdp_scancode_to_evdev_code(flags, scancode, keyboardType);
	quint32 xkbkeycode = evdevcode + 8; /* see minimum in xkb keycodes/evdev file */
#if 0
	qWarning("%s: flags: 0x%04X scancode: 0x%04X evdevcode: 0x%02X (%03d) [%s]", __func__,
		flags, scancode, evdevcode, evdevcode, ogon_evdev_keyname(evdevcode));
#endif
	updateStateFromXkbKeycode(flags, xkbkeycode);
}

void QOgonKeyboard::updateStateFromXkbKeycode(quint32 flags, quint32 keycode) {
	bool isDown = (flags & KBD_FLAGS_DOWN);

#ifndef NO_XKB_SUPPORT
	bool wasDown = mDownKeys.contains(keycode);
	const xkb_keysym_t *syms = 0;
	uint32_t numSyms = xkb_key_get_syms(mXkbState, keycode, &syms);

	if (numSyms != 1) {
		qWarning("keycode=0x%x produces no symbol", keycode);
		return;
	}

	if (isDown && !wasDown) {
		xkb_state_update_key(mXkbState, keycode, XKB_KEY_DOWN);
		mDownKeys.insert(keycode);
	}

	if (!isDown && wasDown) {
		xkb_state_update_key(mXkbState, keycode, XKB_KEY_UP);
		mDownKeys.remove(keycode);
	}

	//qWarning("%s: numSyms:%d vkCode=0x%x xkbKeycode=0x%x", __func__, numSyms, vk_code, keycode);
	QPlatformInputContext *inputContext = QGuiApplicationPrivate::platformIntegration()->inputContext();

	xkb_keysym_t xsym = syms[0];
	Qt::KeyboardModifiers modifiers = translateModifiers(mXkbState);
	QEvent::Type type = isDown ? QEvent::KeyPress : QEvent::KeyRelease;

	uint utf32 = xkb_keysym_to_utf32(xsym);
	QString text = QString::fromUcs4(&utf32, 1);

	uint32_t qtsym = keysymToQtKey(xsym, modifiers, text);
	//qDebug("qtsym=0x%x", qtsym);
	if (!qtsym)
		return;


	bool filtered = false;
	if (inputContext) {
		QKeyEvent event(type, qtsym, modifiers, keycode, xsym, 0/*nativeModifiers*/,
				text, 0/*isAutoRepeat*/, text.size());
		filtered = inputContext->filterEvent(&event);
		//qDebug("0x%x have inputContext, filtered=%d", text[0].toLatin1(), filtered);
	}

	if (!filtered) {
		QOgonAbstractWindow *focusWindow = mWindowManager->getActiveWindow();
		if(!focusWindow) {
			qWarning("%s: no windows has the focus", __func__);
			return;
		}

		QWindowSystemInterface::handleExtendedKeyEvent(focusWindow->window(),
				++mKeytime, type, qtsym, modifiers, keycode, 0, 0,
				text
		);
	}
#endif
}

void QOgonKeyboard::updateStateFromUnicode(quint32 flags, quint32 ucode) {
	QOgonAbstractWindow *focusWindow = mWindowManager->getActiveWindow();
	QEvent::Type type = (flags & KBD_FLAGS_DOWN) ? QEvent::KeyPress : QEvent::KeyRelease;
	QString text = QString::fromUcs4(&ucode, 1);

	if(!focusWindow) {
		qWarning("%s: no windows has the focus", __func__);
		return;
	}
	QWindowSystemInterface::handleExtendedKeyEvent(focusWindow->window(),
		++mKeytime, type, 0, Qt::NoModifier, 0, 0, 0, text
	);
}

void QOgonKeyboard::resetKeyboard() {
	QOgonAbstractWindow *focusWindow = mWindowManager->getActiveWindow();
	if(!focusWindow)
	    return;

	foreach (quint32 keycode, mDownKeys) {
		const xkb_keysym_t *syms = 0;
		uint32_t numSyms = xkb_key_get_syms(mXkbState, keycode, &syms);
		xkb_state_update_key(mXkbState, keycode, XKB_KEY_UP);

		if (numSyms != 1)
			continue;

		Qt::KeyboardModifiers modifiers = translateModifiers(mXkbState);

        xkb_keysym_t xsym = syms[0];
        uint utf32 = xkb_keysym_to_utf32(xsym);
        QString text = QString::fromUcs4(&utf32, 1);

        uint32_t qtsym = keysymToQtKey(xsym, modifiers, text);

		QWindowSystemInterface::handleExtendedKeyEvent(focusWindow->window(),
				++mKeytime, QEvent::KeyRelease, qtsym, modifiers, keycode, 0, 0,
				text
		);
	}

	mDownKeys.clear();
}

QString QOgonKeyboard::computeLocale() const {
	return mLocale;
}
