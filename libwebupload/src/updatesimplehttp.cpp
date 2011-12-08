
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#include "WebUpload/UpdateSimpleHttp"
#include "WebUpload/AuthBase"
#include <QNetworkProxy>
#include <QDebug>
#include <QNetworkConfiguration>

#include <QSslError>
#include <QSslCertificate>
#include <QDBusInterface>
#include <QDBusReply>

#define DBG_PREFIX "UpdateSHttp:"
#define DBG_STREAM qDebug() << DBG_PREFIX
#define WARN_STREAM qWarning() << DBG_PREFIX
#define CRIT_STREAM qCritical() << DBG_PREFIX

using namespace WebUpload;

UpdateSimpleHttp::UpdateSimpleHttp (QObject * parent) : UpdateBase (parent),
    netAM (new QNetworkAccessManager(this)), currentReply (0) {
    
    // Connect signals
    connect (netAM, SIGNAL(finished(QNetworkReply*)), this,
        SLOT(NAMfinished(QNetworkReply*)));
    connect (netAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
        SLOT(namSslErrors(QNetworkReply*,QList<QSslError>)));     
}
        
UpdateSimpleHttp::~UpdateSimpleHttp() {
    netAM->disconnect (this);
    delete netAM;
}

void UpdateSimpleHttp::optionCancel () {
    if (currentReply != 0 && currentReply->isRunning()) {
        currentReply->abort();
    } else {
        WARN_STREAM << "optionCancel ignored, currentReply undefined";    
    }
}

void UpdateSimpleHttp::updateOption () {
    
    currentReply = 0;
    currentReply = generateUpdateRequest (); 
    
    if (currentReply == 0) {
        WARN_STREAM << "Generate request returned null";
        // Default error is connection failure
        Q_EMIT (optionFailed(WebUpload::Error::CODE_CONNECT_FAILURE));
    }       
}

AuthBase *  UpdateSimpleHttp::getAuthPtr () {
    return 0;
}

void UpdateSimpleHttp::addOptionValue (const QString & valueName) { 

    currentReply = generateAddRequest (valueName);
    
    if (currentReply == 0) {
        WARN_STREAM << "Generate request returned null";
        // Default error is connection failure
        Q_EMIT (optionFailed(WebUpload::Error::CODE_CONNECT_FAILURE));
    }       
}

void UpdateSimpleHttp::NAMfinished (QNetworkReply * reply) {
    
    if (currentReply != reply) {
        CRIT_STREAM << "Reply mismatch" << currentReply << reply;
        reply->deleteLater();
        return;
    }

    currentReply = 0;

    QNetworkReply::NetworkError replyError = reply->error();
    switch (replyError) {
    
        case QNetworkReply::OperationCanceledError:
            DBG_STREAM << "NAM finished, operation cancelled";
            Q_EMIT (canceled ());
            break;
            
        case QNetworkReply::SslHandshakeFailedError:
            // Error will be handled in namSslErrors slot
            break;

        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::UnknownNetworkError:
        case QNetworkReply::UnknownProxyError:
        {
            int httpCode = reply->attribute (
                QNetworkRequest::HttpStatusCodeAttribute).toInt();
            WARN_STREAM << "NAM finished with an connection error" << replyError
                << reply->errorString() << " and http code " << httpCode;
            Q_EMIT (optionFailed(WebUpload::Error::CODE_CONNECT_FAILURE)); 
            break;
        }
            
        default:
            DBG_STREAM << "NAM finished, calling handleResponse" << replyError;   
            handleResponse (reply);
            break;
    }
    
    reply->disconnect (this);    
    reply->deleteLater ();
    DBG_STREAM << "Nam finished: done";    
}

void UpdateSimpleHttp::namSslErrors (QNetworkReply * reply, 
    const QList<QSslError> & errors) {
    
    if (currentReply != reply) {
        CRIT_STREAM << "Reply mismatch" << currentReply << reply;
        reply->deleteLater();
        return;
    }
    
    QSslError::SslError errorEnum = QSslError::NoError; 
    int i = 0;
    for (i = 0; errorEnum == QSslError::NoError && i < errors.size (); ++i) {
        errorEnum = errors.at (i).error ();
    }

    if (errorEnum == QSslError::UnableToGetLocalIssuerCertificate) {
        QSslCertificate cert = errors.at (i).certificate ();
        if (!cert.isNull ()) {
            QByteArray certDer = cert.toDer ();
            QDBusInterface iface ("com.nokia.certman", "/com/nokia/certman");
            if (iface.isValid ()) {
                QDBusReply<bool> result = 
                    iface.call ("CheckCertificate", certDer);
                if (result) {
                    reply->ignoreSslErrors ();
                    return;
                }
            }
        }
    }

    currentReply = 0;
    reply->deleteLater ();

    if (errorEnum == QSslError::CertificateNotYetValid) {
        Q_EMIT (optionFailed(WebUpload::Error::CODE_INV_DATE_TIME)); 
    } else {
        //% "Secure connection failed"
        WebUpload::Error error =
            WebUpload::Error::custom("", qtTrId ("qtn_tui_ssl_connection_failed"));

        Q_EMIT (optionFailed(error));
    }
}

QNetworkReply * UpdateSimpleHttp::generateAddRequest (
    const QString &valueName) {
    
    Q_UNUSED (valueName)
    
    return 0;     
}
