
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
        if (!QFile::exists (originalFilePath)) {
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

void PostSimpleHttp::nrUpProgress (qint64 bytesSent, qint64 bytesTotal) {
    
    if (bytesTotal > 0) {
        float progressAmt = ((float)bytesSent)/((float)bytesTotal);
        DBG_STREAM << "progress:" << progressAmt;
        Q_EMIT (mediaProgress(progressAmt));
    } else {
        DBG_STREAM << "undefined progress";
    }
}
