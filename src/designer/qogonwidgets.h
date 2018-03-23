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

#ifndef __QOGON_WIDGETS_H___
#define __QOGON_WIDGETS_H___

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

/**
 * @brief
 */
class QOgonWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")

public:
	/**
	 * @param parent
	 */
	QOgonWidgets(QObject *parent = 0);

	/** @overload QDesignerCustomWidgetCollectionInterface
	 * @{ */
	virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;
	/** @} */

private:
	QList<QDesignerCustomWidgetInterface*> widgets;
};


#endif /* __QOGON_WIDGETS_H___ */
