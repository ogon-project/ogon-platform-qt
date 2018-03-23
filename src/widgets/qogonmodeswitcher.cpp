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

#include <QtWidgets/QApplication>
#include <qpa/qplatformnativeinterface.h>

#include <qogon/qogon.h>
#include <qogon/modeswitcher.h>

QOgonModeSwitcher::QOgonModeSwitcher(QWidget *parent) :
	QComboBox(parent),
	mOgon(0)
{
	if(QGuiApplication::platformName() != "ogon") {
		setDisabled(true);
		addItem( tr("not under Ogon"), "");
		setCurrentIndex(0);
		return;
	}

	const QSize modeSizes[] = {
		QSize(640, 480),
		QSize(800, 600),
		QSize(1024, 768),
		QSize(1280, 1024),
		QSize(1600, 1200),

		QSize(0, 0),	 // /!\ last one to 0 as a marker
	};

	QList<QSize> standardModes;
	for(int i = 0; modeSizes[i].width(); i++)
		standardModes.push_back( modeSizes[i] );

	updateModes(standardModes, 0);

	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	mOgon = qobject_cast<QOgonInterface *>(
			(QObject *)native->nativeResourceForIntegration(QOGON_RESSOURCE_STRING)
	);
	if(!mOgon)
		qFatal("unable to retrieve QOgon interface, even if we're using it as platform");

	connect(mOgon->asQObject(), SIGNAL(ogonScreenReady()), this, SLOT(onScreenReady()));
	connect(this, SIGNAL(currentIndexChanged(int)),
			this, SLOT(on_currentIndexChanged(int)) );
	connect(this, SIGNAL(editTextChanged(const QString)),
			this, SLOT(on_editTextChanged(const QString)) );
}

QSize QOgonModeSwitcher::parseMode(const QString &mode, bool *ok) {
	int pos = mode.indexOf('x');
	if (pos < 0) {
		qWarning("%s: invalid mode %s", __FUNCTION__, mode.toLatin1().data());
		if(ok)
			*ok = false;
		return QSize();
	}

	int width = mode.left(pos).toInt(ok, 10);
	if(!ok || width < 0 || width > 20000) {
		qWarning("%s: invalid width in mode %s", __FUNCTION__, mode.toLatin1().data());
		if(ok)
			*ok = false;
		return QSize();
	}

	int height = mode.mid(pos+1).toInt(ok, 10);
	if(!ok || height < 0 || height > 20000) {
		qWarning("%s: invalid height in mode %s", __FUNCTION__, mode.toLatin1().data());
		if(ok)
			*ok = false;
		return QSize();
	}

	return QSize(width, height);
}

void QOgonModeSwitcher::updateModes(const QList<QSize> &modes, int selected) {
	foreach(QSize mode, modes) {
		int index = mModes.indexOf(mode);
		if (index >= 0)
			continue;

		QString shownValue = QString("%1 x %2").arg(mode.width()).arg(mode.height());
		addItem(shownValue, mode);
		mModes.push_back(mode);
		//qDebug("%s: adding %dx%d", __FUNCTION__, mode.width(), mode.height());
	}

	// TODO: drop modes that have been removed

	if(mModes.size() < selected || selected < 0)
		selected = 0;
	setCurrentIndex(selected);
}

void QOgonModeSwitcher::on_currentIndexChanged(int newIndex) {
	QSize newMode = mModes.at(newIndex);
	qDebug("%s: adding %dx%d", __FUNCTION__, newMode.width(), newMode.height());
	if(!mOgon) {
		qWarning("%s: should ask Ogon to change to mode %dx%d", __FUNCTION__, newMode.width(),
				newMode.height());
		return;
	}

	if(mOgon->getScreenSize() != newMode) {
		mOgon->changeScreenSize(newMode);
	}
}

void QOgonModeSwitcher::on_editTextChanged(const QString &text) {
	bool ok;
	QSize modeSize = parseMode(text, &ok);
	if (!ok) {
		qDebug("%s: invalid mode", __FUNCTION__);
		return;
	}

	int index = mModes.indexOf(modeSize, 0);
	if(index >= 0) {
		qDebug("%s: mode already in the list", __FUNCTION__);
		return;
	}

	addItem(QString("%1 x %2").arg(modeSize.width(), modeSize.height()), modeSize);
	index = mModes.indexOf(modeSize, 0);
	on_currentIndexChanged(index);
}


void QOgonModeSwitcher::onScreenReady() {
	QSize sz = mOgon->getScreenSize();
	int index = mModes.indexOf(sz, 0);

	qDebug("%s: found at index %d", __FUNCTION__, index);
	if(index < 0) {
		qDebug("%s: %dx%d not in the supported modes, adding it", __FUNCTION__,
				sz.width(), sz.height());
		QString newMode = QString("%1 x %2").arg(sz.width()).arg(sz.height());
		insertItem(0, newMode, sz);
		index = 0;
	}

	if(currentIndex() != index)
		setCurrentIndex(index);
}
