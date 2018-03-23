/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 * Simple Greeter Example
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

#include <QtWidgets/QApplication>
#include <qpa/qplatformnativeinterface.h>
#include "simple_greeter.h"
#include <qogon/qogon.h>

SimpleGreeterWindow::SimpleGreeterWindow() : mOgon(0) {
	setupUi(this);

	if(QGuiApplication::platformName() != "ogon") {
		qWarning("not running under ogon");
		return;
	}

	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	mOgon = qobject_cast<QOgonInterface *>(
			(QObject *)native->nativeResourceForIntegration(QOGON_RESSOURCE_STRING)
	);
	if(mOgon) {
		QObject *qobj = mOgon->asQObject();
		connect(qobj, SIGNAL(ogonConnectionEstablished()), this, SLOT(connectionEstablished()));
		connect(qobj, SIGNAL(ogonConnectionLost()), this, SLOT(connectionLost()));
		connect(qobj, SIGNAL(ogonPartialCredentials(QString, QString)), this, SLOT(fillCredential(QString, QString)));
		connect(qobj, SIGNAL(ogonLogonResult(quint32)), this, SLOT(on_logonResult(quint32)));
	}
}

SimpleGreeterWindow::~SimpleGreeterWindow() {
}

void SimpleGreeterWindow::connectionEstablished() {
	qDebug("%s: connection established", __func__);
}

void SimpleGreeterWindow::connectionLost() {
	qDebug("%s: connection lost", __func__);
}

void SimpleGreeterWindow::fillCredential(const QString &username, const QString &domain) {
	qDebug("%s: user=%s domain=%s", __func__,
			username.toLatin1().data(),
			domain.toLatin1().data()
	);
	loginEntry->setText(username);
	domainEntry->setText(domain);
}

void SimpleGreeterWindow::on_logonResult(quint32 res) {
	qDebug("%s: logon result=%d", __func__, res);
}

void SimpleGreeterWindow::on_loginButton_pressed() {
	if(!mOgon)
		return;

	QString login = loginEntry->text();
	QString password = passwordEntry->text();
	QString domain = domainEntry->text();

	qDebug("clicked");
}

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	SimpleGreeterWindow tracer;
	tracer.show();
	return app.exec();
}
