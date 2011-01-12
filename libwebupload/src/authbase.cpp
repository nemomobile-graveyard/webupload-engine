
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

#include "WebUpload/authbase.h"
#include "authbaseprivate.h"
#include <QDebug>

using namespace WebUpload;

AuthBase::AuthBase (QObject *parent) : QObject (parent), 
    d_ptr (new AuthBasePrivate(this)) {

    connect (d_ptr, SIGNAL (authResult(int)), this, SIGNAL (authResult(int)));
    connect (d_ptr, SIGNAL (authUnknownError(QString)), this, 
        SIGNAL (authUnknownError(QString)));
}

AuthBase::~AuthBase () {
}

void AuthBase::startAuth (WebUpload::Account * account) {
    
    // Check if there is an authentication request in progress. If so, there is
    // no need to do anything - that request's signals should suffice
    if (d_ptr->identity != 0) {
        qDebug() << "identity != 0, another request in progress";
        return;
    }

    // If authorization is not required, then result is success
    if (isAuthRequired() == false) {
        Q_EMIT (authResult(RESULT_SUCCESS));
        return;
    }

    AuthData authData = getAuthData();
    if (authData.methodName.isEmpty() || authData.mechanism.isEmpty()) {
        qWarning() << 
            "Plugin did not provide either method name or mechanism";
        Q_EMIT (authResult(RESULT_UNAUTHORIZED));
        return;
    }

    // If account does not exist, we cannot get credentials, hence unauthorized
    // error
    if ((account == 0) || (account->accountsObject() == 0)) {
        qWarning() << "Either account is null object or the accounts object" 
            << " is a null object";
        Q_EMIT (authResult(RESULT_UNAUTHORIZED));
        return;
    }

    d_ptr->identity = SignOn::Identity::existingIdentity (
        account->accountsObject()->credentialsId(), this);
    if (d_ptr->identity == 0) {
        qWarning() << "Auth request cannot be done without identity";
        Q_EMIT (authResult(RESULT_UNAUTHORIZED));
        return;
    }

    d_ptr->session = d_ptr->identity->createSession (authData.methodName);
    if (d_ptr->session == 0) {
        d_ptr->clearAuthInformation();
        QString errMsg = 
            "Could not create session from identity, using method name ";
        errMsg.append (authData.methodName);
        Q_EMIT (authUnknownError(errMsg));
        return;
    }

    d_ptr->account = account;
    connect (d_ptr->session, SIGNAL (response(SignOn::SessionData)), d_ptr, 
        SLOT (sessionResponse(SignOn::SessionData)));
    connect (d_ptr->session, SIGNAL (error(SignOn::Error)), d_ptr, 
        SLOT (sessionError(SignOn::Error)));

    d_ptr->session->process (authData.sessionData, authData.mechanism);
}


void AuthBase::cancel () {
    if (isAuthOngoing ()) {
        qDebug() << "Cancel auth request";
        d_ptr->session->cancel ();
    }
}

bool AuthBase::isAuthOngoing () {
    // If identity has been allocated, then authentication request is in
    // process
    return (d_ptr->identity != 0);
}


bool AuthBase::isAuthReqd () {
    qCritical() << "Deprecated" << __FUNCTION__ << "called";
    return isAuthRequired ();
}

bool AuthBase::isAuthRequired () {
    // Default implementation - no authentication is required
    return false;
}

AuthData AuthBase::getAuthData () {
    // Default implementation - return without filling anything
    AuthData data;
    return data;
}

bool AuthBase::handleError (const SignOn::Error & err) {
    Q_UNUSED (err);
    // Default implementation - error is not handled
    return false;
}

void AuthBase::handleResponse (const SignOn::SessionData & sessionData) {
    Q_UNUSED (sessionData);
    // Default implementation - emit success, and ignore any information that
    // might be there in sessionData
    Q_EMIT (authResult(RESULT_SUCCESS));
    return;
}


/*******************************************************************************
 * Definition of functions for AuthBasePrivate
 ******************************************************************************/

AuthBasePrivate::AuthBasePrivate (AuthBase * parent) : QObject (parent),
    identity (0), account (0), authBaseObject (parent) {

}

AuthBasePrivate::~AuthBasePrivate () {
    clearAuthInformation();
}


void AuthBasePrivate::clearAuthInformation () {
    if (identity != 0 ) {
        if (session != 0) {
            identity->destroySession (session);
        }

        identity->deleteLater ();
        identity = 0;
    }
}

void AuthBasePrivate::sessionResponse (const SignOn::SessionData &sessionData) {
    clearAuthInformation ();
    authBaseObject->handleResponse (sessionData);
}

void AuthBasePrivate::sessionError (const SignOn::Error & err) {
    int code = err.type ();
    QString message = err.message ();

    qWarning () << "Authorization failed with code " << code << 
        " and message " << message;

    clearAuthInformation ();

    switch (code) {
        case SignOn::Error::NoConnection:
            Q_EMIT (authResult (AuthBase::RESULT_NO_CONNECTION));
            break;

        case SignOn::Error::ServiceNotAvailable:
        case SignOn::Error::Network:
            Q_EMIT (authResult (AuthBase::RESULT_CONNECTION_ERROR));
            break;

        case SignOn::Error::IdentityNotFound:
        case SignOn::Error::InvalidCredentials:
        case SignOn::Error::NotAuthorized:
        case SignOn::Error::UserInteraction:
            Q_EMIT (authResult (AuthBase::RESULT_UNAUTHORIZED));
            break;

        case SignOn::Error::Ssl:
            Q_EMIT (authResult (AuthBase::RESULT_CONN_DATE_ERROR));
            break;

        case SignOn::Error::IdentityOperationCanceled:
        case SignOn::Error::SessionCanceled:
            Q_EMIT (authResult (AuthBase::RESULT_CANCELED));
            break;

        default:
            if (authBaseObject->handleError (err) == false) {
                QString errMsg("SSO error !! code = %1, message = %2");
                errMsg = errMsg.arg(QString::number(code)).arg(message);
                Q_EMIT (authUnknownError (errMsg));
            }
            break;
    }
} 