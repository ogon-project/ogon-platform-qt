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

#include "qogonplatform.h"
#include "qogonwindowmanager.h"
#include "qogonscreen.h"
#include "qogonpeer.h"
#include "qogonkeyboard.h"


#include <winpr/stream.h>
#include <freerdp/input.h>

#include <ogon/backend.h>
#include <ogon/service.h>

#include <QtGui/qpa/qwindowsysteminterface.h>

#include <QRegion>
#include <QPainter>
#include <QDebug>
#include <QtWidgets/QApplication>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

WId QOgonAbstractWindow::ms_globalWinId = 1; // the id must not be 0


WId QOgonAbstractWindow::getFreeWinId() {
	return ++ms_globalWinId;
}

QOgonAbstractWindow::QOgonAbstractWindow(QWindow *w, QOgonWindowManager *wm) :
		QPlatformWindow(w),
		mWinId(getFreeWinId()),
		mVisible(false),
		mSentInitialResize(false),
		mState(Qt::WindowNoState),
		mWindowManager(wm)
{
}


QOgonAbstractWindow::~QOgonAbstractWindow() {
}

void QOgonAbstractWindow::setWindowState(Qt::WindowState state) {
	qDebug("QOgonWindow::%s(%d, 0x%x)", __func__, (int)mWinId, (int)state);
	if(mState == state)
		return;

	mState = state;
	switch(state) {
	case Qt::WindowActive:
		mWindowManager->setActiveWindow(this);
		break;
	case Qt::WindowMaximized:
	case Qt::WindowFullScreen: {
		QRect r = mWindowManager->geometry();
		setGeometry(r);
		break;
	}
	default:
		qDebug("QOgonWindow::%s: state %d not handled yet", __func__, (int)state);
		break;
	}

	QWindowSystemInterface::handleWindowStateChanged(window(), state);
	QWindowSystemInterface::flushWindowSystemEvents(); // Required for oldState to work on WindowStateChanged
}


void QOgonAbstractWindow::raise() {
    qDebug("QOgonWindow::%s(%d)", __func__, (int)mWinId);
	mWindowManager->raise(this);
}

void QOgonAbstractWindow::lower() {
    qDebug("QOgonWindow::%s(%d)", __func__, (int)mWinId);
	mWindowManager->lower(this);
}

void QOgonAbstractWindow::setVisible(bool visible) {
    qDebug("QOgonWindow::%s(%d,visible=%d)", __func__, (int)mWinId, visible);
	mVisible = visible;

	QPlatformWindow::setVisible(visible);

	if (visible) {
		if (!mSentInitialResize) {
			QWindowSystemInterface::handleGeometryChange(window(), geometry());
			mSentInitialResize = true;
		}

		QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
	}

	if(!visible)
		mWindowManager->recomputeActiveWindow();
	mWindowManager->repaint(geometry());
}

void QOgonAbstractWindow::setGeometry(const QRect &rect) {
	qDebug("QOgonWindow::%s(%d, %d,%d - %dx%d)", __func__, (int)mWinId, rect.left(),
			rect.top(), rect.width(), rect.height());
	QPlatformWindow::setGeometry(rect);

    QWindowSystemInterface::handleGeometryChange(window(), rect);
    QWindowSystemInterface::handleExposeEvent(window(), QRegion(rect));
}

void QOgonAbstractWindow::propagateSizeHints() {
}




QOgonWindowManager::QOgonWindowManager(QOgonPlatform *platform, const QRect &geometry,
		QLocalServer *sock, bool allowReconnection) :
	mPlatform(platform),
	mScreen(new QOgonScreen(this, geometry.width(), geometry.height(), 32)),
	mGeometry(),
	mAllowReconnection(allowReconnection),
	mSock(sock),
	mCurrentPeer(0),
	mDesktopImage(0),
	mActiveWindow(0)
{
	if (mSock) {
		connect(sock, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	}
	resize(geometry);
}

void QOgonWindowManager::setListeningSocket(QLocalServer *sock) {
	if (sock) {
		mSock = sock;
		connect(sock, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	}
}


QOgonWindowManager::~QOgonWindowManager() {
	if (!mSock) {
		return;
	}
	if (mSock->isListening()) {
		disconnect(mSock, SIGNAL(newConnection()));
		mSock->close();
	}

	delete mSock;
}


void QOgonWindowManager::onNewConnection() {
	if (mCurrentPeer) {
		delete mCurrentPeer;
		emit ogonConnectionLost();
		mCurrentPeer = 0;
	}

	if (!mSock) {
		return;
	}
	QLocalSocket *sock = mSock->nextPendingConnection();
	if (!ogon_check_peer_credentials(sock->socketDescriptor())) {
		qDebug("socket connection forbidden");
		sock->close();
	}

	mCurrentPeer = new QOgonPeer(this, sock);

	emit ogonConnectionEstablished();

	if (!mAllowReconnection) {
		disconnect(mSock, SIGNAL(newConnection()));
		mSock->close();
	}
}

bool QOgonWindowManager::resize(const QRect &geometry) {
	if (mGeometry == geometry)
		return false;

	QImage *oldOne = mDesktopImage;

	mDesktopImage = new QImage(geometry.width(), geometry.height(), QImage::Format_ARGB32_Premultiplied);
	mDesktopImage->fill(Qt::black);

	if (oldOne) {
		QPainter painter(mDesktopImage);
		painter.drawImage(QPoint(0, 0), *oldOne);

		delete oldOne;
	}

	if(mCurrentPeer)
		mCurrentPeer->resize(geometry);

	return true;
}


QRect QOgonWindowManager::geometry() const {
	return mScreen->geometry();
}


void QOgonWindowManager::addWindow(QOgonAbstractWindow *window) {
	mWindows.push_front(window);
	if(window->window()->type() != Qt::Desktop)
		mActiveWindow = window;
}

void QOgonWindowManager::dropWindow(QOgonAbstractWindow *window) {
	if(mActiveWindow == window)
		mActiveWindow = 0;
	if(!mWindows.removeAll(window))
		return;
	repaint(window->geometry());
}

void QOgonWindowManager::raise(QOgonAbstractWindow *window) {
	if(!mWindows.removeOne(window))
		return;

	mWindows.push_front(window);
	if(window->isExposed())
		repaint(window->geometry());
}

void QOgonWindowManager::lower(QOgonAbstractWindow *window) {
	if(!mWindows.removeOne(window))
		return;

	mWindows.push_back(window);
	if(window->isExposed())
		repaint(window->geometry());
}

/**
 * copies a rect <srcRect> area from <srcImg> to <destImg> at <dst>
 * @param srcRect source rectangle
 * @param srcImg source image
 * @param dst destination position in destImg
 * @param destImg the destination image
 */
void qimage_bitblt(const QRect &srcRect, const QImage *srcImg, const QPoint &dst, QImage *destImg) {
	QPainter painter(destImg);
	painter.drawImage(dst, *srcImg, srcRect);
}

void qimage_fillrect(const QRect &rect, QImage *dest, quint32 color) {
	QRect screenGeometry = QRect(0, 0, dest->width(), dest->height());;
	// clip to screen size
	QRect toFill = rect.intersected(screenGeometry); 
	QPoint topLeft = toFill.topLeft();
	int offset = toFill.left();
	int stride = dest->bytesPerLine();

	uchar *first = (dest->bits() + ((topLeft.y()) * stride) +
		(topLeft.x() * 4));
	quint32 *firstPoint = (quint32 *)first;

	// prepare first line
	for (int w = 0; w < toFill.width(); w++, firstPoint++)
		*firstPoint = color;

	uchar *ptr = dest->bits() +
		((topLeft.y() + 1) * stride) +
		(topLeft.x() * 4);

	// and copy it
	for (int h = 1; h < toFill.height(); h++, ptr += stride)
		memcpy(ptr, first , toFill.width() * 4);
}

void QOgonWindowManager::repaint(const QRegion &region) {
	QRect screenGeometry = mScreen->geometry();
	QRegion toRepaint = region.intersected(screenGeometry); // clip to screen size

	//qDebug("============repaint");
	foreach(QOgonAbstractWindow *window, mWindows)
	{
		if(toRepaint.isEmpty())
			break;

		if(!window->isVisible())
			continue;

		const QRect &windowRect = window->geometry();

		QRegion inter = toRepaint.intersected(windowRect);
		//qDebug("window%d", (int)window->winId());
		foreach(QRect repaintRect, inter.rects()) {
			QPoint topLeft = windowRect.topLeft();
			QRect localCoord = repaintRect.translated(-topLeft);
			const QImage *windowContent = window->getContent();

			if (windowContent) {
				qimage_bitblt(localCoord, windowContent, repaintRect.topLeft(), mDesktopImage);
			}
		}

		toRepaint -= inter;
	}

	foreach(QRect repaintRect, toRepaint.rects()) {
		qimage_fillrect(repaintRect, mDesktopImage, 0);
	}

	if(mCurrentPeer)
		mCurrentPeer->addDamage( region.intersected(screenGeometry) );
}


void QOgonWindowManager::setBlankCursor() {
	if (mCurrentPeer) {
		ogon_msg_set_system_pointer set_system_pointer;

		set_system_pointer.ptrType = SYSPTR_NULL;

		mCurrentPeer->sendBytes(OGON_SERVER_SET_SYSTEM_POINTER, (ogon_message *)&set_system_pointer);
	}
}

void QOgonWindowManager::setCursor(ogon_msg_set_pointer *msg) {
	if (mCurrentPeer)
		mCurrentPeer->sendBytes(OGON_SERVER_SET_POINTER, (ogon_message *)msg);
}

QWindow *QOgonWindowManager::getWindowAt(const QPoint pos) const {
	foreach(QOgonAbstractWindow *window, mWindows) {
		if(!window->isVisible())
			continue;

		if(window->geometry().contains(pos))
			return window->window();
	}
	return 0;
}

void QOgonWindowManager::setActiveWindow(QOgonAbstractWindow *w) {
	mActiveWindow = w;
}

QObject *QOgonWindowManager::asQObject() {
	return this;
}

QOgonAbstractWindow *QOgonWindowManager::recomputeActiveWindow() {
	foreach(QOgonAbstractWindow *window, mWindows) {
		if(window->isVisible()) {
			mActiveWindow = window;
			return window;
		}
	}
	mActiveWindow = 0;
	return 0;
}


quint32 QOgonWindowManager::sbpCall(quint32 msgType, const QByteArray &request) {
	if (!mCurrentPeer) {
		qWarning("logonUser(): not connected nothing can be done");
		return 0;
	}

	return mCurrentPeer->sbpCall(msgType, request);
}

QString QOgonWindowManager::guessLocaleFromKeyboard() const {
	if (!mCurrentPeer) {
		qWarning("%s(): not connected nothing can be done", __FUNCTION__);
		return QString::null;
	}

	return mCurrentPeer->mKeyboard->computeLocale();
}

void QOgonWindowManager::changeScreenSize(const QSize &newSize) {
	if (!mCurrentPeer) {
		qWarning("%s(): not connected nothing can be done", __FUNCTION__);
		return;
	}

	if(mScreen->geometry().size() != newSize)
		mCurrentPeer->resize( QRect(QPoint(), newSize) );
}

QSize QOgonWindowManager::getScreenSize() const {
	if (!mCurrentPeer) {
		qWarning("%s(): not connected nothing can be done", __FUNCTION__);
		return QSize(0, 0);
	}

	return mScreen->geometry().size();
}

void QOgonWindowManager::notifyPeerClosed() {
	if (!mAllowReconnection) {
		QCoreApplication::quit();
	}
}

void QOgonWindowManager::handleMouseEvent(const QPoint &pos, Qt::MouseButtons buttons,
		int wheelDelta)
{
	QWindow *window = getWindowAt(pos);
	if(!window)
		return;

	Qt::KeyboardModifiers modifiers = Qt::NoModifier;
	QPoint wTopLeft = window->geometry().topLeft();
	QPoint localCoord = pos - wTopLeft;
	QWindowSystemInterface::handleMouseEvent(window, localCoord, pos, buttons, modifiers);

	if(wheelDelta != 0) {
		QPoint angleDelta;
		angleDelta.setY(wheelDelta);
		QWindowSystemInterface::handleWheelEvent(window, localCoord, pos, QPoint(), angleDelta);
	}
}
