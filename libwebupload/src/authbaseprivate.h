
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

#ifndef _AUTH_BASE_PRIVATE_H_
#define _AUTH_BASE_PRIVATE_H_

#include <QObject>
#include <WebUpload/Account>
#include <identity.h>
#include <authsession.h>

namespace WebUpload {

    class AuthBase;

    /*!
      \class AuthBasePrivate
      \brief Private class for WebUpload::AuthBase
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class AuthBasePrivate : public QObject {

        Q_OBJECT
    
    public:
        /*!
          \brief Create new private object for AuthBase
          \param parent If not valid pointer to object all functionality
                        will not work.
         */
        AuthBasePrivate(AuthBase * parent);
        ~AuthBasePrivate ();

        /*!
          \brief Destroy the identity and session information
         */
        void clearAuthInformation ();
    
    public Q_SLOTS:

        /*!
          \brief Slot to get response from Signon for the authentication
                 request
          \param sessionData: Filled sessionData instance, which should contain
                     the authentication token
         */
        void sessionResponse (const SignOn::SessionData &sessionData);

        /*!
          \brief Slot to get error from Signon for the authentication request
          \param err: SignOn::Error object with error information
         */
        void sessionError(const SignOn::Error &err);

    Q_SIGNALS:

        /*!
          \brief Signal for authorization response. Connects to corresponding
                 signal in AuthBase
          \param code Result for authorization (enum AuthBase::ResultCode)
         */
        void authResult (int result);

        /*!
          \brief Signal emitted for unknown errors. Connects to corresponding
                 signal in AuthBase
          \param errMsg String containing information about the unknown error
         */
        void authUnknownError (const QString &errMsg);

    public:
        SignOn::Identity *identity; //!< Identity associated with the account
        SignOn::AuthSessionP session; //!< Session for the account
        WebUpload::Account * account; //!< Account owned

    private:
        AuthBase * const authBaseObject; //!< Public parent object
    };
}

#endif
