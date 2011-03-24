
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
 
#ifndef _WEBUPLOAD_POST_SIMPLE_HTTP_H_
#define _WEBUPLOAD_POST_SIMPLE_HTTP_H_

#include <WebUpload/export.h>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <WebUpload/PostBase>
#include <WebUpload/Media>

#include <QVariantList>

namespace WebUpload {
    /*!
       \class PostSimpleHttp
       \brief This is base class for post implementations that only need single
              network request to upload a file to service. generateRequest() is
              called to create request and start it. Then this class will handle
              connectivity errors. And if reponse is receive it's given with
              handleResponse. It's then up to plugin to implement response error
              handling and emit correct signals defined in WebUploadPostBase.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT PostSimpleHttp : public PostBase {

        Q_OBJECT

    public:
        /*!
          \brief Constructor
          \param parent QObject parent
         */
        PostSimpleHttp (QObject *parent = 0);
        
        virtual ~PostSimpleHttp ();
        
    Q_SIGNALS:
        
        /*!
          \brief Signal emitted by the plugin implementation when the upload
                 request has more than one network requests associated with it,
                 and the next request has to be sent
          \param media Pointer to the media instance whose upload request is in
                 process
          \param options A QVariantList filled with values that would be
                 required by the next upload request. 
         */
        void sendNextNetworkRequest (WebUpload::Media *media, 
            QVariantList options);

    protected:

        /*!
          \brief Will try to stop upload by calling abort to reply received.
         */
        virtual void stopMediaUpload();

    protected Q_SLOTS:

        /*!
          \brief Slot for QNetworkAccessManager::finished. Will check common
                 errors first and then calls handleResponse with reply if common
                 errors not found.
         */        
        virtual void namFinished (QNetworkReply * reply);
        
        /*!
          \brief Slot for QNetworkReply::uploadProgress. Will calculate
                 progress and then emit mediaProgress with float value.
         */        
        virtual void nrUpProgress (qint64 bytesSent, qint64 bytesTotal);
        
        /*!
          \brief Implements uploadMedia by calling generateRequest() and then
                 handleResponse() functions.
          \param media Media to be uploaded
         */
        virtual void uploadMedia (WebUpload::Media * media);              
        
    protected:
        
        /*!
          \brief Called to generate and start request by using class variable
                 netAM. Should be implemented by inheriting class.
          \param media Media that should be uploaded to service
          \return Reply class generated. If null then assumed that request can't
                  be generated.
         */
        virtual QNetworkReply * generateRequest (WebUpload::Media * media) = 0;

        /*!
          \brief Called to generate and start request by using class variable
                 netAM. Should be implemented by inheriting class. Default
                 implementation is an empty function, provided so that services
                 that don't need this feature need not implement this function
          \param media Media that should be uploaded to service
          \param options A QVariantList filled with values that would be
                 required by this next upload request.
          \return Reply class generated. If null then assumed that request can't
                  be generated.
         */
        virtual QNetworkReply * generateNextRequest (WebUpload::Media * media,
            QVariantList options);

        
        /*!
          \brief Called when response is received and common errors are not
                 present. Should be implemented by inheriting class. Must emit
                 signals to WebUploadPostBase level.
          \param response Response to handle
         */
        virtual void handleResponse (QNetworkReply * response) = 0;
        
        QNetworkAccessManager * netAM; //!< Manager used for connections
        QNetworkReply * currentReply; //!< Reply received from generateRequest()

        bool uploadStopped; //!< Has upload stopped been called?

    private Q_SLOTS:

        /*!
          \brief Slot for QNetworkAccessManager::sslErrors
         */
        void namSslErrors (QNetworkReply * reply, 
            const QList<QSslError> & errors);

        /*!
          \brief Slot for signal sendNextNetworkRequest
          \param options A QVariantList filled with values that would be
                 required by the next upload request.
         */
        void nextNetworkRequest (WebUpload::Media * media, 
            QVariantList options);

    private:

        //! Pointer to the current media being uploaded. This is being stored
        // to support uploads that require multiple network requests
        WebUpload::Media *m_currMedia;
    };
}

#endif
