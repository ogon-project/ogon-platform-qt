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

#ifndef __QOGON_WINDOWMANAGER_H___
#define __QOGON_WINDOWMANAGER_H___

#include <QList>
#include <QRect>
#include <QRegion>
#include <QObject>

#include <qogon/qogon.h>
#include <qpa/qplatformwindow.h>


class QOgonPlatform;
class QOgonScreen;
class QOgonPeer;
class QLocalServer;
class QOgonWindowManager;
typedef struct _ogon_msg_set_pointer ogon_msg_set_pointer;

/**
 * @brief an abstract class for Ogon windows
 */
class QOgonAbstractWindow : public QPlatformWindow {
	Q_DECLARE_PRIVATE(QPlatformWindow)

public:
	QOgonAbstractWindow(QWindow *w, QOgonWindowManager *wm);

	/** virtual dtor to make gcc happy */
	virtual ~QOgonAbstractWindow();

	/** @return the window Id */
    WId winId() const { return mWinId; }

    bool isVisible() const { return mVisible; }

    /** @overload QPlatformWindow
     * @{*/
    virtual void setWindowState(Qt::WindowState state);
    virtual void raise();
    virtual void lower();
    virtual void setVisible(bool visible);
    virtual void setGeometry(const QRect &rect);
    virtual void propagateSizeHints();
    /** @} */

	/**
	 * method to implement that returns the content of the window
	 * @return a QImage that contains the window content
	 */
	virtual const QImage *getContent() = 0;


protected:
	WId mWinId;
	bool mVisible;
	bool mSentInitialResize;
	Qt::WindowState mState;
	QOgonWindowManager *mWindowManager;

protected:
    static WId getFreeWinId();

    static WId ms_globalWinId;
};

/**
 *	@brief
 */
class QOgonWindowManager : public QObject, public QOgonInterface {
	friend class QOgonPlatform;
	friend class QOgonPeer;
	friend class QOgonScreen;

	Q_OBJECT
	Q_INTERFACES(QOgonInterface)

public:
	/**
	 *
	 * @param platform
	 */
	QOgonWindowManager(QOgonPlatform *platform, const QRect &geometry, QLocalServer *sock,
			bool allowReconnection);

	virtual ~QOgonWindowManager();

	QRect geometry() const;

	const QImage &image() const { return *mDesktopImage; };

	/**
	 *
	 * @param geometry
	 * @return if some resize has been done
	 */
	bool resize(const QRect &geometry);

	void addWindow(QOgonAbstractWindow *window);

	void dropWindow(QOgonAbstractWindow *window);

	void raise(QOgonAbstractWindow *window);

	void lower(QOgonAbstractWindow *window);

	void repaint(const QRegion &region);

	/** sets a null pointer shaper */
	void setBlankCursor();

	void setCursor(ogon_msg_set_pointer *msg);

	/** retrieve the window visible at the given position
	 * @param pos the position
	 * @return the computed window, NULL otherwise
	 */
	QWindow *getWindowAt(const QPoint pos) const;

	void setActiveWindow(QOgonAbstractWindow *w);

	QOgonAbstractWindow *getActiveWindow() const { return mActiveWindow; }

	/** computes the active window according to our internal list
	 * @return the new active window
	 */
	QOgonAbstractWindow *recomputeActiveWindow();

	/** @overload QOgonInterface
	 * @{ */
	virtual QObject *asQObject();
	virtual quint32 sbpCall(quint32 msgType, const QByteArray &request);
	virtual QString guessLocaleFromKeyboard() const;
	virtual QSize getScreenSize() const;
	virtual void changeScreenSize(const QSize &newSize);
	/** @} */

	/** Allows to set the ListeningSocket at a later point
	 */
	void setListeningSocket(QLocalServer *sock);
public slots:
	void onNewConnection();

signals:
	void ogonConnectionEstablished();
	void ogonScreenReady();
	void ogonConnectionLost();
	void ogonPartialCredentials(const QString &login, const QString &domain);
	void ogonSbpReply(quint32 tagId, quint32 sbpType, quint32 status, const QByteArray &reply);

protected:
	void notifyPeerClosed();
	void handleMouseEvent(const QPoint &pos, Qt::MouseButtons buttons, int wheelDelta);

protected:
	QOgonPlatform *mPlatform;
	QOgonScreen *mScreen;
	QRect mGeometry;
	bool mAllowReconnection;
	QLocalServer *mSock;
	QOgonPeer *mCurrentPeer;

	QImage *mDesktopImage;

	typedef QList<QOgonAbstractWindow *> QOgonWindowList;
	QOgonWindowList mWindows;
	QOgonAbstractWindow *mActiveWindow;
};

#endif /* __QOGON_WINDOWMANAGER_H___ */
