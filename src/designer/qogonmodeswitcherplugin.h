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

#ifndef __QOGON_MODESWITCHERPLUGIN_H___
#define __QOGON_MODESWITCHERPLUGIN_H___

#include <QDesignerCustomWidgetInterface>
#include <QDesignerTaskMenuExtension>

class QIcon;
class QWidget;

/**
* @brief designer interface for the Ogon mode switcher widget
*/
class QOgonModeSwitcherPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
	QOgonModeSwitcherPlugin(QObject *parent = 0);

	/** @overload QDesignerCustomWidgetInterface
	* @{ */
	QString name() const;
	QString group() const;
	QString toolTip() const;
	QString whatsThis() const;
	QString includeFile() const;
	QIcon icon() const;
	bool isContainer() const;
	QWidget *createWidget(QWidget *parent);
	bool isInitialized() const;
	void initialize(QDesignerFormEditorInterface *formEditor);
	QString domXml() const;
	/** @} */

protected:
	bool initialized;
};

#endif /* __QOGON_MODESWITCHERPLUGIN_H___ */
