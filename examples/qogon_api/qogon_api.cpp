/**
 * ogon - Free Remote Desktop Services
 * Qt Platform Abstraction Layer (QPA)
 * ogon API example
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
#include "qogon_api.h"
#include <qogon/qogon.h>

OgonTracer::OgonTracer() : doRun(false), mOgon(0) {
	if(QGuiApplication::platformName() != "ogon") {
		qWarning("not running under ogon");
		return;
	}

	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	QObject *qobj1 = (QObject *)native->nativeResourceForIntegration(QOGON_RESSOURCE_STRING);
	mOgon = qobject_cast<QOgonInterface *>(qobj1);
	if(mOgon) {
		QObject *qobj = mOgon->asQObject();
		connect(qobj, SIGNAL(ogonConnectionEstablished()), this, SLOT(connectionEstablished()));
		connect(qobj, SIGNAL(ogonConnectionLost()), this, SLOT(connectionLost()));
		doRun = true;
	}
}

OgonTracer::~OgonTracer() {
	if(!mOgon)
		return;

	QObject *qobj = (QObject *)mOgon;
	disconnect(qobj, SIGNAL(ogonConnectionEstablished()));
	disconnect(qobj, SIGNAL(ogonConnectionLost()));
}

void OgonTracer::connectionEstablished() {
	qDebug("%s: connection established", __func__);
}

void OgonTracer::connectionLost() {
	qDebug("%s: connection lost", __func__);
}

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	OgonTracer tracer;
	if(tracer.doRun)
		app.exec();
}
