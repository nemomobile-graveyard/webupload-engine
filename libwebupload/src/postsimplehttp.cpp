
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
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
 
#include "WebUpload/PostSimpleHttp"
#include <QDebug>
#include <QNetworkConfiguration>
#include <QFile>

#define DBG_PREFIX "PostSHttp:"
#define DBG_STREAM qDebug() << DBG_PREFIX
#define WARN_STREAM qWarning() << DBG_PREFIX
#define CRIT_STREAM qCritical() << DBG_PREFIX

using namespace WebUpload;

PostSimpleHttp::PostSimpleHttp (QObject * parent) : PostBase (parent),
    netAM (new QNetworkAccessManager(this)), currentReply (0) {
   
    // Connect signals
    connect (netAM, SIGNAL(finished(QNetworkReply*)), this,
        SLOT(namFinished(QNetworkReply*)));     
    connect (netAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
        SLOT(namSslErrors(QNetworkReply*,QList<QSslError>)));     
}
        
PostSimpleHttp::~PostSimpleHttp() {
    netAM->disconnect (this);
    delete netAM;
}

void PostSimpleHttp::stopMediaUpload () {
    if (currentReply != 0 && currentReply->isRunning()) {
        currentReply->abort();
    } else {
        WARN_STREAM << "stopMediaUpload ignored, currentReply undefined";
    }
}

void PostSimpleHttp::uploadMedia (Media * media) {

    DBG_STREAM << "Calling generateRequest";
    currentReply = 0;
    
    if (media->type() == Media::TYPE_FILE) {        
        QString originalFilePath = media->srcFilePath ();
        QString copyFilePath = media->copyFilePath ();
        if (!QFile::exists (originalFilePath) ||
            !QFile::exists (copyFilePath)) {

            Q_EMIT (mediaError(WebUpload::Error::missingFiles()));
            return;
        }
    }

    currentReply = generateRequest (media);

    if (currentReply == 0) {
        WARN_STREAM << "Generate request returned null";
        // Letting this stay as custom error, since this should not normally
        // happen
        Q_EMIT (mediaError(WebUpload::Error::custom ("System Failure",
            "Failed to create request")));
    } else {
        // Connect progress signal
        QObject::connect (currentReply, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(nrUpProgress(qint64,qint64)));   
    }
}

void PostSimpleHttp::namFinished (QNetworkReply * reply) {
    
    if (currentReply != reply) {
        CRIT_STREAM << "Reply mismatch" << currentReply << reply;
        reply->deleteLater();
        return;
    }
    
    currentReply = 0;

    QNetworkReply::NetworkError replyError = reply->error();
    reply->disconnect (this);
    
    switch (replyError) {
        case QNetworkReply::OperationCanceledError:
            DBG_STREAM << "NAM finished, operation cancelled";
            Q_EMIT (stopped ());
            break;
            
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
        case QNetworkReply::SslHandshakeFailedError:
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
            WARN_STREAM << "NAM finished with an connection error" <<
                replyError << reply->errorString() << " and http code " <<
                httpCode;
            Q_EMIT (mediaError(WebUpload::Error::connectFailure())); 
            break;
        }
            
        default: 
            DBG_STREAM << "Calling handleResponse" << replyError; 
            handleResponse (reply);
            break;
    }
    
    reply->deleteLater ();
}

void PostSimpleHttp::namSslErrors (QNetworkReply * reply, 
    const QList<QSslError> & errors) {
    
    if (currentReply != reply) {
        CRIT_STREAM << "Reply mismatch" << currentReply << reply;
        reply->deleteLater();
        return;
    }
    
    const QSslError::SslError errorEnum = QSslError::NoError; 
    int i = 0;
    for (i = 0; errorEnum == QSslError::NoError && i < errors.size (); ++i) {
        errorEnum = errors.at (i).error ();
    }

    if (firstError == QSslError::UnableToGetLocalIssuerCertificate) {
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

    if (errorEnum == QSslError::NoError) {
        qWarning() << "QNetworkAccessManager::sslErrors signal emitted with "
            "no ssl errors - just marking transfer as failed";
        Q_EMIT (mediaError(WebUpload::Error::transferFailed())); 
        return;
    }

    WebUpload::Error error = WebUpload::Error::connectFailure();
    switch (errorEnum) {
        case QSslError::CertificateNotYetValid:
            //% "Check device time and date."
            error->setDescription (qtTrId ("qtn_tui_invalid_device_time"));
            break;

        default:
            //% "Secure connection failed"
            error->setDescription (qtTrId ("qtn_tui_ssl_connection_failed"));
            break;
    }

    Q_EMIT (mediaError(error)); 
}


void PostSimpleHttp::nrUpProgress (qint64 bytesSent, qint64 bytesTotal) {
    
    if (bytesTotal > 0) {
        float progressAmt = ((float)bytesSent)/((float)bytesTotal);
        DBG_STREAM << "progress:" << progressAmt;
        Q_EMIT (mediaProgress(progressAmt));
    } else {
        DBG_STREAM << "undefined progress";
    }
}
