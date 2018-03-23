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

#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "qogonmodeswitcherplugin.h"

#include <qogon/modeswitcher.h>


QOgonModeSwitcherPlugin::QOgonModeSwitcherPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

QString QOgonModeSwitcherPlugin::name() const {
	return "QOgonModeSwitcher";
}

QString QOgonModeSwitcherPlugin::group() const {
	return "ogon widgets";
}

QString QOgonModeSwitcherPlugin::toolTip() const {
	return tr("this widget allows to switch the resolution");
}

QString QOgonModeSwitcherPlugin::whatsThis() const {
	return "";
}

QString QOgonModeSwitcherPlugin::includeFile() const {
	return "qogon/modeswitcher.h";
}

QIcon QOgonModeSwitcherPlugin::icon() const {
	return QIcon(":/images/modeswitcher.png");
}

bool QOgonModeSwitcherPlugin::isContainer() const {
	return false;
}

QWidget *QOgonModeSwitcherPlugin::createWidget(QWidget *parent) {
	QOgonModeSwitcher *switcher = new QOgonModeSwitcher(parent);
	return switcher;
}

bool QOgonModeSwitcherPlugin::isInitialized() const {
	return initialized;
}

void QOgonModeSwitcherPlugin::initialize(QDesignerFormEditorInterface *formEditor) {
	Q_UNUSED(formEditor)

	if (initialized)
		return;

	initialized = true;
}

QString QOgonModeSwitcherPlugin::domXml() const
{
	return QLatin1String("\
		<ui language=\"c++\">\
		<widget class=\"QOgonModeSwitcher\" name=\"modeSwitcher\"/>\
		</ui>");
}
