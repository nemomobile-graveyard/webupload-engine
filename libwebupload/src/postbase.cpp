 
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

#include "WebUpload/PostBase"
#include "postbaseprivate.h"
#include "WebUpload/Entry"
#include "WebUpload/Media"
#include <QDebug>

#define DBG_PREFIX "PostBase:"
#define DBG_STREAM qDebug() << DBG_PREFIX
#define WARN_STREAM qWarning() << DBG_PREFIX
#define CRIT_STREAM qCritical() << DBG_PREFIX

using namespace WebUpload;

typedef PostBasePrivate MyPrivate;

PostBase::PostBase (QObject *parent) : PostInterface (parent),
    d_ptr (new MyPrivate(this)) {

    connect (this, SIGNAL (mediaProgress(float)), this,
        SLOT (mediaProgressSlot(float)));
    connect (d_ptr, SIGNAL (progress(float)), this, SIGNAL (progress(float)));

    connect (d_ptr, SIGNAL (mediaStarted(WebUpload::Media*)), this, 
        SIGNAL (mediaStarted(WebUpload::Media*)));
    connect (d_ptr, SIGNAL (error(WebUpload::Error)),
        this, SIGNAL (error(WebUpload::Error)));
#ifdef WARNINGS_ENABLED
    connect (d_ptr, SIGNAL (warning(WebUpload::Error)),
             this, SIGNAL(warning(WebUpload::Error)));
#endif
    connect (d_ptr, SIGNAL (stopped()), this, SIGNAL (stopped()));
    connect (d_ptr, SIGNAL (done()), this, SIGNAL (done()));
    connect (d_ptr, SIGNAL (nowUploadMedia(WebUpload::Media*)), this, 
        SLOT (uploadMedia(WebUpload::Media*)), Qt::QueuedConnection);

    connect (this, SIGNAL (mediaDone(QString)), d_ptr, 
        SLOT (mediaDoneSlot(QString)), Qt::QueuedConnection);
    connect (this, SIGNAL (mediaError(WebUpload::Error)), d_ptr,
        SLOT (mediaErrorSlot(WebUpload::Error)), Qt::QueuedConnection);

#ifdef LIBWEBUPLOAD_EXPERIENTAL
#ifdef WARNINGS_ENABLED
    connect (this, SIGNAL (mediaWarning(WebUpload::Error)), d_ptr,
        SLOT (mediaWarningSlot(WebUpload::Error)), Qt::QueuedConnection);
#endif
#endif

    connect (this, SIGNAL (reAuth()), d_ptr, SLOT (reAuthSlot()), 
        Qt::QueuedConnection);
    connect (this, SIGNAL (mediaStopped()), d_ptr, SLOT (mediaStoppedSlot()));

    connect (this, SIGNAL (errorFixFailed(WebUpload::Error)), this, 
        SIGNAL (error(WebUpload::Error)), Qt::QueuedConnection);
    connect (this, SIGNAL (errorFixed()), this, SLOT (errorFixedSlot()),
        Qt::QueuedConnection);
}

PostBase::~PostBase() {
    delete d_ptr;
}

void PostBase::upload (WebUpload::Entry * entry, WebUpload::Error error) {

    Q_ASSERT (entry != 0);
    Q_ASSERT (d_ptr->setEntry (entry));
    
    if (error.code () != WebUpload::Error::CODE_NO_ERROR) {
        DBG_STREAM << "fix error";    
        d_ptr->state = MyPrivate::STATE_FIX_ERROR_PENDING;
        fixError (entry, error);
    } else if (d_ptr->media == 0) {
        /* There are no more media to be sent */
        d_ptr->reset ();
        DBG_STREAM << "done";        
        Q_EMIT (done());
    } else {
        //Notify Media Started
        Q_EMIT (mediaStarted (d_ptr->media));
        DBG_STREAM << "start authentication";
        d_ptr->startAuthentication (getAuthPtr());
    }

    return;
}

unsigned int PostBase::unsentCount () const {
    if (d_ptr->entry == 0) 
        return 0;
    else 
        return d_ptr->entry->mediaCount() - d_ptr->entry->mediaSentCount ();
}

void PostBase::stop () {
    if (d_ptr->state == MyPrivate::STATE_UPLOAD_PENDING) {
        d_ptr->state = MyPrivate::STATE_CANCEL_PENDING;
        DBG_STREAM << "stop media upload";
        stopMediaUpload ();

        // If the media is still in active state, move it to paused state.
        // Otherwise it might be in completed or canceled state, in which case
        // we do not need to bother.
        if (d_ptr->media->isActive ()) {
            d_ptr->media->setPaused ();
        }
    } else {
        DBG_STREAM << "stop media";
        d_ptr->stop ();
    }
}

void PostBase::fixError (WebUpload::Entry * entry, WebUpload::Error error) {

    Q_UNUSED (entry);

    if (canRetry (error)) {
        Q_EMIT (errorFixed ());
    } else {
        // Since the plugin is not handling the error, there is nothing more
        // that can be done
        Q_EMIT (errorFixFailed (error));
    }
}

bool PostBase::canRetry (WebUpload::Error error) {

    if (error.repairable() == false) 
        return false;

    WebUpload::Error::Code code = error.code();
    if ((code == WebUpload::Error::CODE_AUTH_FAILED) ||
        (code == WebUpload::Error::CODE_CONNECT_FAILURE) || 
        (code == WebUpload::Error::CODE_UPLOAD_LIMIT_EXCEEDED) ||
        (code == WebUpload::Error::CODE_INV_DATE_TIME) || 
        (code == WebUpload::Error::CODE_ACCOUNT_DISABLED)) {
        
        return true;
    } 

    return false;
}


void PostBase::mediaProgressSlot (float uploaded) {
    if (uploaded > 1.0) {
        uploaded = 1.0;
    } else if (uploaded < 0.0) {
        uploaded = 0.0;
    }

    if (d_ptr->totalSize > 0 && uploaded > 0.0) {
        float totalDone = d_ptr->ofItemDone + 
            uploaded*((float)(d_ptr->media->fileSize()))/
            ((float)d_ptr->totalSize);
        
        if (totalDone > 1.0) {
            totalDone = 1.0;
        }

        Q_EMIT (progress (totalDone));        
    }
}

void PostBase::errorFixedSlot () {
    d_ptr->errorFixed (getAuthPtr ());
}

// --- Private class functions ----

PostBasePrivate::PostBasePrivate(PostBase * parent) : QObject (parent), 
    state (STATE_IDLE), publicObject (parent), authPtr (0) {

    reset ();
}

PostBasePrivate::~PostBasePrivate () {
    // authPtr is owned by the plugin's PostBase implementation. The destructor
    // of that class would already have been called before this destructor is
    // called. Hence, can set the authPtr pointer to 0 without bothering about
    // disconnects
    authPtr = 0;
    reset ();
}

void PostBasePrivate::reset () {
    if (authPtr != 0) {
        authPtr->disconnect (this);
        authPtr = 0;
    }

    entry = 0;
    media = 0;
    state = STATE_IDLE;
    totalSize = 0;
    sentSize = 0;
    ofItemDone = 0.0;
    transferError.clearError ();
}


bool PostBasePrivate::setEntry (Entry *entry_t) {
    if (entry)
        return false;

    entry = entry_t;
    media = entry->nextUnsentMedia ();
    totalSize = entry->totalSize ();
    sentSize = totalSize - entry->unsentSize ();
    ofItemDone = ((float)sentSize)/((float)totalSize);
    transferError.setTransferFileCount (entry->mediaCount ());

    return true;
}

void PostBasePrivate::errorFixed (AuthBase *authP) {

    DBG_STREAM << "Error fix successful";

    entry->clearFailed();
    media = entry->nextUnsentMedia ();
    Q_EMIT (mediaStarted (media));
    startAuthentication (authP);
}


void PostBasePrivate::startAuthentication (AuthBase *authP) {
    state = STATE_AUTH_PENDING;
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

    if (entry == 0) {
        WebUpload::Error err = WebUpload::Error::transferFailed ();
        qWarning () << "Null entry pointer";
        reset ();
        Q_EMIT (error (err));
        return;
    }

    unsigned int unsentFiles = entry->mediaCount () - entry->mediaSentCount ();
    if (authPtr == 0) {
        WebUpload::Error err = WebUpload::Error::transferFailed ();
        err.setFailedCount (unsentFiles);
        qWarning () << "Null authPtr";
        reset ();
        Q_EMIT (error (err));
        return;
    }

    //WebUpload::Account * account = entry->account().data ();
    WebUpload::SharedAccount account = entry->account();
    if ((account.isNull()) || (account->accountsObject() == 0)) {
        WebUpload::Error err = WebUpload::Error::accountRemoved ();
        err.setFailedCount (unsentFiles);
        account.clear ();
        reset ();
        Q_EMIT (error (err));
        return;
    } else if (account->isValid() == false) {
        WebUpload::Error err = WebUpload::Error::accountDisabled ();
        err.setFailedCount (unsentFiles);
        account.clear ();
        reset ();
        Q_EMIT (error (err));
        return;
    }

    // Since AuthBase::startAuth expects WebUpload::Account *
    authPtr->startAuth (account.data ());
}

void PostBasePrivate::authResultSlot (int err) {

    // Handle user cancels
    if (state == STATE_CANCEL_PENDING) {
        reset ();
        Q_EMIT (stopped());
        return;
    }

    if (err == WebUpload::AuthBase::RESULT_SUCCESS) {
        DBG_STREAM << "Media authentication successful";
        state = STATE_UPLOAD_PENDING;
        Q_ASSERT (media->setActive());
        Q_EMIT (nowUploadMedia(media));
        return;
    } 

    DBG_STREAM << "Media authentication failed with code " << err;

    unsigned int unsentCount = entry->mediaCount () - entry->mediaSentCount ();
    reset ();

    WebUpload::Error authError;
    switch (err) {
        case WebUpload::AuthBase::RESULT_NO_CONNECTION:
            authError = WebUpload::Error::noConnection();
            break;

        case WebUpload::AuthBase::RESULT_UNAUTHORIZED:
        // If cancel is not started by us, but by user -> same as unauth        
        case WebUpload::AuthBase::RESULT_CANCELED:
            authError = WebUpload::Error::authorizationFailed();
            break;

        case WebUpload::AuthBase::RESULT_CONNECTION_ERROR:
            authError = WebUpload::Error::connectFailure();
            break;

        case WebUpload::AuthBase::RESULT_CONN_DATE_ERROR:
            authError = WebUpload::Error::dateTimeError ();
            break;

        default:
            authError = WebUpload::Error::transferFailed ();
            break;
    }

    authError.setFailedCount (unsentCount);
    Q_EMIT (error (authError));

    return;
}


void PostBasePrivate::authUnknownErrorSlot (const QString &errMsg) {
    qDebug() << "enter PostBasePrivate::authUnknownErrorSlot";
    unsigned int unsentCount = entry->mediaCount () - entry->mediaSentCount ();
    qDebug() << "media count = " << entry->mediaCount ();
    qDebug() << "media sent count = " << entry->mediaSentCount ();
    reset ();

    WebUpload::Error authError = WebUpload::Error::serviceError (errMsg);
    qDebug() << "setting failed count as " << unsentCount;
    qDebug() << "errMsg is " << errMsg;
    authError.setFailedCount (unsentCount);
    qDebug() << "exit PostBasePrivate::authUnknownErrorSlot, before emitting error";
    Q_EMIT (error(authError));
}


void PostBasePrivate::mediaDoneSlot (QString destUrl) {
    media->setCompleted (destUrl);

    sentSize = totalSize - entry->unsentSize ();
    ofItemDone = ((float)sentSize)/((float)totalSize);
    Q_EMIT (progress (ofItemDone));

    media = entry->nextUnsentMedia ();
    if (!media) {
        // No more media to send
        if (transferError.code () != WebUpload::Error::CODE_NO_ERROR) {
            WebUpload::Error err = transferError;
            reset ();
            Q_EMIT (error (err));
        } else {
            reset ();
            Q_EMIT (done ());
        }
    } else {
        if (state == STATE_CANCEL_PENDING) {
            reset ();
            Q_EMIT (stopped ());
        } else {
            state = STATE_AUTH_PENDING;
            
            //Notify Media Started
            Q_EMIT (mediaStarted (media));
            startAuthentication ();
        }
    }
}
        
void PostBasePrivate::reAuthSlot () {
    state = STATE_AUTH_PENDING;
    startAuthentication ();
}

#ifdef LIBWEBUPLOAD_EXPERIENTAL
#ifdef WARNINGS_ENABLED
void PostBasePrivate::mediaWarningSlot(WebUpload::Error warningMessage)
{

    WARN_STREAM << "Media Upload warning (" << warningMessage.code() << "):"
        << warningMessage.title() << ":" << warningMessage.description();
    Q_EMIT(warning(warningMessage));

}
#endif
#endif


void PostBasePrivate::mediaErrorSlot (WebUpload::Error err) {

    WARN_STREAM << "Media Upload error (" << err.code() << "):"
        << err.title() << ":" << err.description();
        
    media->setFailed();
    transferError.merge (err);

    if (transferError.canContinue ()) {
        Media *nextMedia = entry->nextUnsentMedia ();
        if (nextMedia) {
            if (state == STATE_CANCEL_PENDING) {
                reset ();
                Q_EMIT (stopped ());
            } else {
                media = nextMedia;
                state = STATE_AUTH_PENDING;
                //Notify Media Started
                Q_EMIT (mediaStarted (media));
                startAuthentication ();
            }
        } else {
            // All done - emit error of 'X' media not uploaded
            WebUpload::Error err1 = transferError;
            reset ();
            Q_EMIT (error (err1));
        }
    } else {
        WebUpload::Error err1 = transferError;
        unsigned int unsentCount = entry->mediaCount () - 
            entry->mediaSentCount ();
        err1.setFailedCount (unsentCount);
        reset ();
        Q_EMIT (error (err1));
    }
}
        
void PostBasePrivate::mediaStoppedSlot () {
    reset ();
    Q_EMIT (stopped());
}

void PostBasePrivate::stop () {
    switch (state) {
        case STATE_AUTH_PENDING:
            state = STATE_CANCEL_PENDING;
            Q_ASSERT (authPtr != 0);
            authPtr->cancel ();
            break;

        case STATE_UPLOAD_PENDING:
            // Should have been handled in the PostBase class 
            break;

        case STATE_CANCEL_PENDING:
            // A cancel is already being handled. Nothing more to do.
            break;

        default:
            // There is no upload request or authorization request in progress.
            // Can just say that cancel has happened.
            Q_EMIT (stopped ());
            break;
    }
}
