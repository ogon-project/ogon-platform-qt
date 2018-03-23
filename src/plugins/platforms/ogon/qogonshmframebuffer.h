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

#ifndef __QOGON_SHM_FRAMEBUFFER_H__
#define __QOGON_SHM_FRAMEBUFFER_H__

#include <QRegion>
#include <QImage>

/**
 * @brief a shared framebuffer based on a SysV SHM
 */
class QOgonShmFramebuffer {
public:
	QOgonShmFramebuffer();

	~QOgonShmFramebuffer();

	void resize(const QRect &geometry);

	bool updateDamage(const QImage &src, QRegion &r, int shmId);

	/** @return the current geometry */
	QRect geometry() const { return mGeometry; }

	/** @return the scanline value */
	int scanline() const { return mScanline; }

protected:
	void updateFramebuffer(const QImage &src, const QRect &r);

	void *mDmgBuf;
	QRect mGeometry;
	int mScanline;
};


#endif /* __QOGON_SHM_FRAMEBUFFER_H__ */
