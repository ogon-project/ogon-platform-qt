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

#include "qogonshmframebuffer.h"

#include <ogon/dmgbuf.h>

QOgonShmFramebuffer::QOgonShmFramebuffer() : mDmgBuf(0), mScanline(0) {
}

QOgonShmFramebuffer::~QOgonShmFramebuffer() {
	if (mDmgBuf)
		ogon_dmgbuf_free(mDmgBuf);
}

void QOgonShmFramebuffer::resize(const QRect &geometry) {
	mGeometry = geometry;
	mScanline = geometry.width() * 4;

	int mod = (mScanline % 16);
	if (mod)
		mScanline += 16 - mod;
}

bool QOgonShmFramebuffer::updateDamage(const QImage &src, QRegion &r, int shmId) {
	if (mDmgBuf) {
		if (ogon_dmgbuf_get_id(mDmgBuf) != shmId) {
			ogon_dmgbuf_free(mDmgBuf);
			mDmgBuf = 0;
			r = mGeometry;
		}
	}

	r = r.intersected( mGeometry );
	if (!mDmgBuf) {
		mDmgBuf = ogon_dmgbuf_connect(shmId);
		if (!mDmgBuf) {
			qFatal("%s: unable to bind shmId=%d", __FUNCTION__, shmId);
			return false;
		}
	}

	int nbRects = r.rectCount();
	RDP_RECT *rdpRect = ogon_dmgbuf_get_rects(mDmgBuf, NULL);

	if (nbRects > (int)ogon_dmgbuf_get_max_rects(mDmgBuf)) {
		/* too many rectangles, let's send the bouding rectangle */
		QRect extents = r.boundingRect();

		ogon_dmgbuf_set_num_rects(mDmgBuf, 1);
		rdpRect->x = extents.left();
		rdpRect->y = extents.top();
		rdpRect->width = extents.width();
		rdpRect->height = extents.height();
		updateFramebuffer(src, extents);
		return true;
	}

	ogon_dmgbuf_set_num_rects(mDmgBuf, nbRects);
	foreach(QRect rect, r.rects()) {
		rdpRect->x = rect.left();
		rdpRect->y = rect.top();
		rdpRect->width = rect.width();
		rdpRect->height = rect.height();
		rdpRect++;

		updateFramebuffer(src, rect);
	}

	return true;
}

void QOgonShmFramebuffer::updateFramebuffer(const QImage &img, const QRect &r) {
	const uchar *src = img.bits() +
			(r.top() * img.bytesPerLine()) +
			(r.left() * 4);

	uchar *dst = ogon_dmgbuf_get_data(mDmgBuf) +
			(r.top() * mScanline) +
			(r.left() * 4);

	for(int y = r.top(); y <= r.bottom(); y++) {
		memcpy(dst, src, r.width() * 4);

		src += img.bytesPerLine();
		dst += mScanline;
	}
}
