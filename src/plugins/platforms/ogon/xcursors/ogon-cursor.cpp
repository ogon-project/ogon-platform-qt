/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 *
 * Copyright (c) 2013-2018 Thincast Technologies GmbH
 *
 * Authors:
 * David Fort <contact@hardening-consulting.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Thincast Technologies GmbH nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

/**
 * this code is an adaptation of the wayland-cursor.c from wayland source code
 * with the following notice:
 *
 *
 * Copyright © 2012 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "xcursor.h"
#include "ogon-cursor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])


/** @brief a cursor theme */
struct ogon_cursor_theme {
	unsigned int cursor_count;
	struct ogon_cursor **cursors;

	char *name;
	int size;
};

struct cursor {
	struct ogon_cursor cursor;
	uint32_t total_delay; /* length of the animation in ms */
};



static void ogon_cursor_image_destroy(struct ogon_cursor_image *_img)
{
	struct cursor_image *image = (struct cursor_image *) _img;

	free(image);
}

static void ogon_cursor_destroy(struct ogon_cursor *cursor)
{
	unsigned int i;

	for (i = 0; i < cursor->image_count; i++)
		ogon_cursor_image_destroy(cursor->images[i]);

	free(cursor->name);
	free(cursor);
}

#include "cursor-data.h"

static struct ogon_cursor *ogon_cursor_create_from_data(struct cursor_metadata *metadata)
{
	struct cursor *cursor;
	struct ogon_cursor_image *image;
	int size;

	cursor = (struct cursor *)malloc(sizeof *cursor);
	if (!cursor)
		return NULL;

	cursor->cursor.image_count = 1;
	cursor->cursor.images = (struct ogon_cursor_image **)calloc(1, sizeof *cursor->cursor.images);
	if (!cursor->cursor.images)
		goto err_free_cursor;

	cursor->cursor.name = strdup(metadata->name);
	if (!cursor->cursor.name)
		goto err_free_images;
	cursor->total_delay = 0;

	size = metadata->width * metadata->height * 4;
	image = (struct ogon_cursor_image *)malloc(sizeof *image + size);
	if (!image)
		goto err_free_name;

	cursor->cursor.images[0] = image;
	image->width = metadata->width;
	image->height = metadata->height;
	image->hotspot_x = metadata->hotspot_x;
	image->hotspot_y = metadata->hotspot_y;
	image->delay = 0;
	memcpy(image+1, &cursor_data[metadata->offset], size);

	return &cursor->cursor;

err_free_name:
	free(cursor->cursor.name);
err_free_images:
	free(cursor->cursor.images);
err_free_cursor:
	free(cursor);
	return NULL;
}


static void load_default_theme(struct ogon_cursor_theme *theme)
{
	uint32_t i;

	free(theme->name);
	theme->name = strdup("default");

	theme->cursor_count = ARRAY_LENGTH(cursor_metadata);
	theme->cursors = (struct ogon_cursor **)calloc(theme->cursor_count, sizeof(*theme->cursors));

	for (i = 0; i < theme->cursor_count; ++i) {
		theme->cursors[i] =	ogon_cursor_create_from_data(&cursor_metadata[i]);

		if (theme->cursors[i] == NULL)
			break;
	}
	theme->cursor_count = i;
}


static struct ogon_cursor *ogon_cursor_create_from_xcursor_images(XcursorImages *ximages)
{
	struct cursor *cursor;
	struct ogon_cursor_image *ogonCursorImage;
	int i;

	cursor = (struct cursor *)malloc(sizeof *cursor);
	if (!cursor)
		return NULL;

	cursor->cursor.images =	(struct ogon_cursor_image **)calloc(ximages->nimage, sizeof cursor->cursor.images[0]);
	if (!cursor->cursor.images) {
		free(cursor);
		return NULL;
	}

	cursor->cursor.name = strdup(ximages->name);
	cursor->total_delay = 0;

	for (i = 0; i < ximages->nimage; i++) {
		int dataSize = ximages->images[i]->width * ximages->images[i]->height * 4;
		ogonCursorImage = (struct ogon_cursor_image *)malloc(sizeof *ogonCursorImage + dataSize);

		ogonCursorImage->width = ximages->images[i]->width;
		ogonCursorImage->height = ximages->images[i]->height;
		ogonCursorImage->hotspot_x = ximages->images[i]->xhot;
		ogonCursorImage->hotspot_y = ximages->images[i]->yhot;
		ogonCursorImage->delay = ximages->images[i]->delay;

		memcpy(ogonCursorImage+1, ximages->images[i]->pixels, dataSize);

		cursor->total_delay += ogonCursorImage->delay;
		cursor->cursor.images[i] = ogonCursorImage;
	}
	cursor->cursor.image_count = i;

	if (cursor->cursor.image_count == 0) {
		free(cursor->cursor.name);
		free(cursor->cursor.images);
		free(cursor);
		return NULL;
	}

	return &cursor->cursor;
}

static void load_callback(XcursorImages *images, void *data)
{
	struct ogon_cursor_theme *theme = (struct ogon_cursor_theme *)data;
	struct ogon_cursor *cursor;

	if (ogon_cursor_theme_get_cursor(theme, images->name)) {
		XcursorImagesDestroy(images);
		return;
	}

	cursor = ogon_cursor_create_from_xcursor_images(images);

	if (cursor) {
		theme->cursor_count++;
		theme->cursors = (struct ogon_cursor **)realloc(theme->cursors, theme->cursor_count * sizeof theme->cursors[0]);

		theme->cursors[theme->cursor_count - 1] = cursor;
	}

	XcursorImagesDestroy(images);
}

struct ogon_cursor_theme *ogon_cursor_theme_load(const char *name, int size)
{
	struct ogon_cursor_theme *theme;

	theme = (struct ogon_cursor_theme *)malloc(sizeof *theme);
	if (!theme)
		return NULL;

	if (!name)
		name = "default";

	theme->name = strdup(name);
	if (!theme->name)
		goto out_free;
	theme->size = size;
	theme->cursor_count = 0;
	theme->cursors = NULL;

	xcursor_load_theme(name, size, load_callback, theme);

	if (theme->cursor_count == 0)
		load_default_theme(theme);

	return theme;

out_free:
	free(theme);
	return NULL;
}


void ogon_cursor_theme_destroy(struct ogon_cursor_theme *theme)
{
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++)
		ogon_cursor_destroy(theme->cursors[i]);

	free(theme->cursors);
	free(theme);
}

/** Get the cursor for a given name from a cursor theme
 *
 * \param theme The cursor theme
 * \param name Name of the desired cursor
 * \return The theme's cursor of the given name or %NULL if there is no
 * such cursor
 */
struct ogon_cursor *
ogon_cursor_theme_get_cursor(struct ogon_cursor_theme *theme, const char *name)
{
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++) {
		if (strcmp(name, theme->cursors[i]->name) == 0)
			return theme->cursors[i];
	}

	return NULL;
}

/** Find the frame for a given elapsed time in a cursor animation
 *
 * \param cursor The cursor
 * \param time Elapsed time since the beginning of the animation
 *
 * \return The index of the image that should be displayed for the
 * given time in the cursor animation.
 */
int ogon_cursor_frame(struct ogon_cursor *_cursor, uint32_t time)
{
	struct cursor *cursor = (struct cursor *) _cursor;
	uint32_t t;
	int i;

	if (cursor->cursor.image_count == 1)
		return 0;

	i = 0;
	t = time % cursor->total_delay;

	while (t - cursor->cursor.images[i]->delay < t)
		t -= cursor->cursor.images[i++]->delay;

	return i;
}
