
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
 
#ifndef _WEBUPLOAD_UPDATE_SIMPLE_HTTP_H_
#define _WEBUPLOAD_UPDATE_SIMPLE_HTTP_H_

#include <WebUpload/export.h>
#include <WebUpload/UpdateBase>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>


namespace WebUpload {

    class AuthBase;

    /*!
       \class UpdateSimpleHttp
       \brief This is base class for post implementations that only need single
              network request to update single option values from the service.
              generateRequest() is called to create request and start it. Then
              WebUploadUpdateSimpleHttp class will handle connectivity errors.
              And if reponse is receive it's given with handleResponse. It's
              then up to plugin to implement response error handling and emit
              correct signals defined in WebUploadUpdateBase. If you need to do
              multiple HTTP requests to update values then implement your class
              based on UpdateBase.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT UpdateSimpleHttp : public UpdateBase {

        Q_OBJECT

    public:

        /*!
          \brief Constructor
          \param parent QObject parent
         */
        UpdateSimpleHttp (QObject *parent = 0);
        
        virtual ~UpdateSimpleHttp();

        /*!
          \brief Reimplements UpdateBase::getAuthPtr
          \return Return null for no authorization handling. Should be
                  reimplemented by inheriting classes to add authorization
                  handling if needed.
         */
        virtual AuthBase * getAuthPtr (); 
        
    protected:
           
        /*!
          \brief Needed update steps has to be done in implementation of this
                 function. Emit optionDone or optionFailed signal when update
                 is done.
         */        
        virtual void updateOption ();
        
        /*!
          \brief Will try to stop request handled by network access manager
         */
        virtual void optionCancel();
        
        /*!
          \brief Called to generate and start update request by using class
                 variable <code>netAM</code>. Has to be implemented by
                 inheriting class.
          \return Needs to return reply received from netAM when request is
                  started. Used to cancel and follow request.
         */
        virtual QNetworkReply * generateUpdateRequest () = 0;
        
        /*!
          \brief Called to generate and start add option request by using class
                 variable <code>netAM</code>. Default implementation returns
                 <code>null</code>.
          \param valueName Name of value to be added
          \return Needs to return reply received from netAM when request is
                  started. Used to cancel and follow request.
         */
        virtual QNetworkReply * generateAddRequest (const QString &valueName);
        
        /*!
          \brief Called when response is received and common errors are not
                 present. Should be implemented by inheriting class. Must emit
                 signals to WebUploadPostBase level.
          \param response Response that should be handled
         */
        virtual void handleResponse (QNetworkReply * response) = 0;
        
        QNetworkAccessManager * netAM; //!< Manager used for connections
        QNetworkReply * currentReply; //!< Reply received from generateRequest()
        
    protected Q_SLOTS:
        
        /*!
          \brief Slot for QNetworkAccessManager::finished. Will check common
                 errors first and then calls handleResponse with reply if common
                 errors not found.
         */
        virtual void NAMfinished (QNetworkReply * reply);        

        /*!
          \brief Steps needed to add a new value to the option are done in the
                 implementation of this function. Emit optionDone or
                 optionFailed signal when update is done.
         */
        virtual void addOptionValue (const QString &valueName);

    private Q_SLOTS:

        /*!
          \brief Slot for QNetworkAccessManager::sslErrors
         */
        void namSslErrors (QNetworkReply * reply, 
            const QList<QSslError> & errors);
    };
}

#endif
