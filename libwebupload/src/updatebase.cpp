 
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

#include "WebUpload/UpdateBase"
#include "updatebaseprivate.h"
#include "WebUpload/AuthBase"
#include <QDebug>
#include <QListIterator>
#include "WebUpload/PostOption"

using namespace WebUpload;

typedef UpdateBasePrivate MyPrivate;

UpdateBase::UpdateBase (QObject *parent) : UpdateInterface(parent),
    d_ptr (new MyPrivate(this)) {
    
    connect (d_ptr, SIGNAL (done()), this, SIGNAL (done()));
    connect (d_ptr, SIGNAL (canceled()), this, SIGNAL (canceled()));
    connect (d_ptr, SIGNAL (error(WebUpload::Error::Code, QStringList)), this, 
        SIGNAL (error(WebUpload::Error::Code, QStringList)));
    connect (d_ptr, SIGNAL (error(WebUpload::Error, QStringList)), this,
        SIGNAL (error(WebUpload::Error, QStringList)));

    connect (d_ptr, SIGNAL (updateAuthDone()), this, SLOT (updateOption()));
    connect (d_ptr, SIGNAL (addAuthDone(QString)), this, 
        SLOT (addOptionValue(QString)));

    // Connect internal signals
    connect (this, SIGNAL (optionDone()), d_ptr, SLOT (optionDoneSlot()));
    connect (this, SIGNAL (optionAdded(bool)), d_ptr,
        SLOT (optionAddedSlot(bool)));
    connect (this, SIGNAL (optionFailed(WebUpload::Error::Code)), d_ptr,
        SLOT (optionFailedSlot(WebUpload::Error::Code)));
    connect (this, SIGNAL (optionFailed(WebUpload::Error)), d_ptr,
        SLOT (optionFailedSlot(WebUpload::Error)));
    connect (this, SIGNAL (reAuth()), d_ptr, SLOT (reAuthSlot()), 
        Qt::QueuedConnection);
    connect (this, SIGNAL (forceReAuthorization()), this, SLOT (forceReAuth()));
}

UpdateBase::~UpdateBase() {
    delete d_ptr;
}

void UpdateBase::updateAll (WebUpload::Account * account) {

    if (!(account->service()->hasUpdatablePostOptions())) {
        qDebug() << "No updatable options found but update called";
        Q_EMIT (done());
        return;
    }

    d_ptr->updateAll (account);
}

void UpdateBase::updateAllForceReAuth (WebUpload::Account * account) {

    Q_EMIT (forceReAuthorization());
    updateAll (account);
}
    
void UpdateBase::update (WebUpload::Account * account,
    WebUpload::ServiceOption * option) {
        
    // Nothing to do with options that aren't updatable
    if (!(option != 0 && option->isUpdatable())) {
        Q_EMIT (done());
        return;
    }    
    
    d_ptr->update (account, option, true);
}

void UpdateBase::updateForceReAuth (WebUpload::Account * account,
    WebUpload::ServiceOption * option) {

    Q_EMIT (forceReAuthorization());
    update (account, option);
}

void UpdateBase::addValue (WebUpload::Account * account,
    WebUpload::ServiceOption * option, const QString & valueName) {

    // Nothing to do with options that aren't updatable
    if (!(option != 0 && option->isChangeable())) {
        Q_EMIT (done());
        return;
    }    
    
    d_ptr->addValue (account, option, valueName);
}

void UpdateBase::forceReAuth() {
    // Nothing to be done by default
}

void UpdateBase::cancel() {
    if (d_ptr->state == MyPrivate::STATE_AUTH) {
        qDebug() << "Cancelling auth request";
        d_ptr->cancelAuth ();
    } else if (d_ptr->state == MyPrivate::STATE_UPDATE) {
        qDebug() << "Cancelling option update";
        d_ptr->state = MyPrivate::STATE_CANCEL;
        optionCancel();
    } else {
        qDebug() << "Nothing to cancel?";
    }
}

WebUpload::Account * UpdateBase::account () {
    return d_ptr->accountWas;
}

bool UpdateBase::isUpdate () const {
    return d_ptr->isUpdate;
}

WebUpload::ServiceOption * UpdateBase::currentOption () {
    return d_ptr->currentOption;
}

// -- Private class functions ---

UpdateBasePrivate::UpdateBasePrivate(UpdateBase * parent) : QObject (parent),
    accountWas (0), currentOption(0), isUpdate (false), state (STATE_IDLE),
    singleTask (false), optionIter(QList<PostOption *> ()), 
    publicObject (parent), authPtr (0) {
        
}

UpdateBasePrivate::~UpdateBasePrivate() {
    // authPtr belongs to the derived class, whose destructor would have been
    // called before this, and so, authPtr should have been destroyed by now
    authPtr = 0;
    reset ();
}

void UpdateBasePrivate::reset () {
    accountWas = 0;
    state = STATE_IDLE;

    // Reset to iterator to empty list
    optionIter = QList<PostOption *> ();

    currentOption = 0;

    if (authPtr != 0) {
        authPtr->disconnect (this);
        authPtr = 0;
    }

}

void UpdateBasePrivate::updateAll (WebUpload::Account * account) {

    qDebug() << "In UpdateBasePrivate::updateAll";
    accountWas = account;
    
    optionIter = account->service()->postOptions();
    
    if (optionIter.hasNext() == false) {
        qWarning() << "Failed to receive options";
        reset ();
        Q_EMIT (done());
        return;
    }
    
    while (optionIter.hasNext()) {
        ServiceOption * option = 
            qobject_cast<ServiceOption *> (optionIter.next());
        if (option != 0 && option->isUpdatable()) {
            update (accountWas, option, false);
            return;
        }
    }
    
    qWarning() << "No updatable options found?";
    reset ();
    Q_EMIT (done());
    
    return;
}

void UpdateBasePrivate::update (WebUpload::Account * account,
    WebUpload::ServiceOption * option, bool singleTask) {
        
    if (singleTask == true) {
        accountWas = account;
        this->singleTask = true;
    } else {
        this->singleTask = false;
    }

    currentOption = option;
    state = STATE_AUTH;
    isUpdate = true;
    startAuthentication (publicObject->getAuthPtr());
}

void UpdateBasePrivate::addValue (WebUpload::Account * account,
    WebUpload::ServiceOption * option, const QString & valueName) {

    accountWas = account;
    singleTask = true;
    currentOption = option;
    this->valueName = valueName;
    state = STATE_AUTH;
    isUpdate = false;
    startAuthentication (publicObject->getAuthPtr());
}

void UpdateBasePrivate::cancelAuth () {
    if ((state == STATE_AUTH) && (authPtr != 0)) {
        state = STATE_CANCEL;
        authPtr->cancel();
    } else {
        qWarning() << "Either state is wrong or authPtr is null. State is "
            << state;
    }
}

void UpdateBasePrivate::startAuthentication (AuthBase *authP) {
    qDebug() << "Starting UpdateBasePrivate::startAuthentication";

    if (authP != 0) {
        if (authPtr != 0) {
            authPtr->disconnect (this);
        }

        authPtr = authP;

        connect (authPtr, SIGNAL (authResult(int)), this,
            SLOT (authResultSlot(int)));
        connect (authPtr, SIGNAL (authUnknownError(QString)), this,
            SLOT (authUnknownErrorSlot(QString)));
    }

    if (authPtr == 0) {
        QStringList failedIds;
        // Default error would be connect failure error
        Q_EMIT (error (WebUpload::Error::CODE_CONNECT_FAILURE, failedIds));
        return;
    }

    if ((accountWas == 0) || (accountWas->accountsObject() == 0)) {
        QStringList failedIds;
        Q_EMIT (error (WebUpload::Error::CODE_ACCOUNT_REMOVED, failedIds));
        return;
    } else if (accountWas->isValid() == false) {
        QStringList failedIds;
        Q_EMIT (error (WebUpload::Error::CODE_ACCOUNT_DISABLED, failedIds));
        return;
    }

    authPtr->startAuth (accountWas);
}

void UpdateBasePrivate::authResultSlot (int err) {

    // Handle user cancels
    if (state == STATE_CANCEL)  {
        reset ();
        Q_EMIT (canceled());
        return;
    }

    Q_ASSERT (state == STATE_AUTH);

    if (err == WebUpload::AuthBase::RESULT_SUCCESS) {
        qDebug() << "Authentication successful";

        state = STATE_UPDATE;
        if (isUpdate == true) {
            qDebug() << "Item" << currentOption->id() << "update...";    
            Q_EMIT (updateAuthDone());
        } else {
            qDebug() << "Add" << valueName << "to" << currentOption->id();
            Q_EMIT (addAuthDone(valueName));
        }

        return;
    } 

    qDebug() << "Media authentication failed with code " << err;
    reset ();

    QStringList failedIds;
    switch (err) {
        case WebUpload::AuthBase::RESULT_NO_CONNECTION:
            Q_EMIT (error(WebUpload::Error::CODE_NO_CONNECTION, failedIds));
            break;

        case WebUpload::AuthBase::RESULT_UNAUTHORIZED:
        // If cancel is not started by us, but by user -> same as unauth
        case WebUpload::AuthBase::RESULT_CANCELED:        
            Q_EMIT (error(WebUpload::Error::CODE_AUTH_FAILED, failedIds));
            break;

        case WebUpload::AuthBase::RESULT_CONN_DATE_ERROR:
            Q_EMIT (error(WebUpload::Error::CODE_INV_DATE_TIME, failedIds));
            break;

        case WebUpload::AuthBase::RESULT_CONNECTION_ERROR:
        default:
            // Using connection failure as default error
            Q_EMIT (error(WebUpload::Error::CODE_CONNECT_FAILURE, failedIds));
    }

    return;
}


void UpdateBasePrivate::authUnknownErrorSlot (const QString &errMsg) {
    Q_UNUSED (errMsg);
    qDebug() << "enter UpdateBasePrivate::authUnknownErrorSlot";
    reset ();
    qDebug() << "After reset";
    QStringList failedIds;
    Q_EMIT (error(WebUpload::Error::CODE_CONNECT_FAILURE, failedIds));
    qDebug() << "exit UpdateBasePrivate::authUnknownErrorSlot after emitting error";
}


void UpdateBasePrivate::optionAddedSlot (bool needsUpdate) {
    // New value added, now do option update as well
    if (needsUpdate) {
        update (accountWas, currentOption, true);
        return;
    } else {
        reset ();
        Q_EMIT (done());    
    }
}

void UpdateBasePrivate::optionDoneSlot () {

    if (!isUpdate) {
        qWarning() << "OptionDoneSlot called while not in update mode";
        // Direct to correct slot
        optionAddedSlot (true);
        return;

    } else if (!singleTask) {
        while (optionIter.hasNext()) {
            ServiceOption * option = 
                qobject_cast<ServiceOption *> (optionIter.next());

            if (option != 0 && option->isUpdatable()) {
                update (accountWas, option, false);
                return;
            }
        }
    }

    reset ();
    Q_EMIT (done());
}

void UpdateBasePrivate::optionFailedSlot (WebUpload::Error::Code errCode) {
        
    //TODO: If not so fatal error, we could continue
    if (state == STATE_CANCEL) {
        Q_EMIT (canceled());
    } else {
        QStringList failed;
        Q_EMIT (error (errCode, failed));
    }

    reset (); 
}

void UpdateBasePrivate::optionFailedSlot (WebUpload::Error optionError) {

    if (state == STATE_CANCEL) {
        Q_EMIT (canceled());
    } else {
        QStringList failed;
        Q_EMIT (error (optionError, failed));
    }

    reset ();
}

void UpdateBasePrivate::reAuthSlot () {
    state = STATE_AUTH;
    startAuthentication(publicObject->getAuthPtr());
}
