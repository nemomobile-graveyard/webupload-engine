 
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

#ifndef _WEBUPLOAD_AUTH_BASE_H_
#define _WEBUPLOAD_AUTH_BASE_H_

#include <QObject>
#include <QString>
#include <WebUpload/Account>
#include <authsession.h>
#include <signonerror.h>

namespace WebUpload {

    // Forward declaration of private class
    class AuthBasePrivate;

    /*!
      \class  AuthData
      \brief  This struct collects information required from the plugin to
              start the SSO authentication process
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    struct AuthData {
        //! Name of the authentication method to use
        QString methodName; 
        //! Information required for the authentication session
        SignOn::SessionData sessionData;
        //! Mechanism used for the authentication process
        QString mechanism;
    };


    /*!
      \class  AuthBase
      \brief  This class provides the base class to be used by the Upload
              plugins to handle authentication requirements before upload or
              option updates are done. This class abstracts out all the common
              SSO communication. The plugin could derive from this class and
              over-ride all the functions, if it wants to handle the
              authentication process by itself
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class AuthBase : public QObject {
        Q_OBJECT

    public:

        /*! 
         \brief Constructor
         \param parent QObject parent
        */
        AuthBase (QObject * parent);

        //! Destructor
        virtual ~AuthBase ();

        /*!
          \brief Result codes for auth
         */
        enum ResultCode {
            RESULT_SUCCESS, //!< Authorization successfull
            RESULT_UNAUTHORIZED, //!< Invalid username/password
            RESULT_CONNECTION_ERROR, //!< Connection error
            RESULT_CONN_DATE_ERROR, //!< This usually means that date is wrong
            RESULT_CANCELED, //!< Auth process was cancelled
            RESULT_NO_CONNECTION //!< No network connection currently
        };

        /*!
          \brief This function is re-implemented by plugins to handle error
                 responses that cannot be handled in the base class. The plugin
                 should only bother about errors it can handle. Any errors that
                 cannot be handled either in the base class or the plugin will
                 be treated as unknown errors and handled in the base class
                 Default implementation of this function will always return
                 <code>false</code>
          \param err SignOn::Error object with error information
          \return <code>true</code> if the plugin handled the error and emitted
                  the authResult signal, else <code>false</code>
         */
        virtual bool handleError (const SignOn::Error &err);

        /*!
          \brief This function is re-implemented by plugins to extract whatever
                 information is required from the authentication response. 
                 Default implementation of this function will just emit the
                 authResult signal with the RESULT_SUCCESS as argument
          \param sessionData Filled SessionData response from SSO. 
         */
        virtual void handleResponse (const SignOn::SessionData &sessionData);

    public Q_SLOTS:

        /*!
          \brief Start the authorization process. This is not normally
                 implemented by the plugin - it should be implemented by the
                 plugin only if it has some custom way of managing
                 authentication.
                 This function calls the protected functions isAuthReqd() and
                 getAuthData() which should be implemented by the plugin to
                 provide appropriate data to the base class.
          \param account Account used
         */
        virtual void startAuth (WebUpload::Account * account = 0);

        /*!
          \brief Cancel running authorization process. If process is found and
                 canceled, then authResult is emitted. 
                 This should be implemented by the plugin if and only if the
                 plugin implements the start function as well
         */
        virtual void cancel();

        /*!
          \brief Check if authentication process is ongoing
                 This should be implemented by the plugin if and only if the
                 plugin implements the start function as well
          \return <code>true</code> if authentication process is ongoing
         */
        virtual bool isAuthOngoing ();

    Q_SIGNALS:

        /*!
          \brief Signal for authorization response
          \param result Result for authorization (enum AuthBase::ResultCode)
         */
        void authResult (int result);

        /*!
          \brief Signal emitted for unknown errors
          \param errMsg String containing information about the unknown error
         */
        void authUnknownError (const QString &errMsg);

    protected:
       
        /*!
          \brief Check if authentication is required. Plugins would typically
                 over-ride this and return <code>true</code> and
                 <code>false</code> depending on what they do with the
                 authentication response. 
                 Default implementation in this class always returns false.
          \return <code>true</code> if authentication process has to be started
         */        
        virtual bool isAuthRequired ();

        /*!
          \brief Get the authentication information. Plugins need to
                 re-implement this function to provide the correct value.
                 Default implementation of this function will return an
                 instance of the AuthData class with all fields empty
          \return Filled AuthData reference
         */
        virtual AuthData getAuthData ();
        
    private:

        Q_DISABLE_COPY(AuthBase)
        AuthBasePrivate * const d_ptr; //!< Private data

    };
}

#endif
