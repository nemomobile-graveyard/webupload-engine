
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
 
#include "uploadprocess.h"
#include <QDebug>
#include "WebUpload/Entry"
#include "WebUpload/Media"
#include "WebUpload/Account"
#include <QFile>

UploadProcess::UploadProcess (QObject * parent) : 
    WebUpload::PluginProcess (parent), m_currItem (0), m_currEntry (0),
    m_currMediaIdx (-1), m_resultHandled(false), m_stopping(false) {

    // Making these connections queued connection so as to not block the event
    // loop when some data comes from the upload process
    connect (&m_pdata, SIGNAL (sendingMediaSignal(quint32)), this,
        SLOT (sendingMedia(quint32)), Qt::QueuedConnection);
    connect (&m_pdata, SIGNAL (doneSignal()), this, SLOT (done()),
        Qt::QueuedConnection);
    connect (&m_pdata, SIGNAL (stoppedSignal()), this, SLOT (stopped()),
        Qt::QueuedConnection);
    connect (&m_pdata, SIGNAL (uploadFailedSignal(WebUpload::Error)), this, 
        SLOT (failed(WebUpload::Error)), Qt::QueuedConnection);
    connect (&m_pdata, SIGNAL (optionValueChangedSignal(QString,QVariant,int)),
        this, SLOT (optionValueChanged(QString,QVariant,int)));

    connect (this, SIGNAL(currentProcessStopped()), this, 
        SLOT (pluginProcessCrashed()), Qt::QueuedConnection);
}

UploadProcess::~UploadProcess () {
    m_pdata.disconnect (this);
    if (m_currItem) {
        m_pdata.disconnect (m_currItem);
    }
}

UploadItem * UploadProcess::currentlySendingMedia () const {
    return m_currItem;
}

void UploadProcess::startUpload (UploadItem * item) {
    qDebug() << "UploadProcess::" << __FUNCTION__;
    if (canProcessNewRequest (item)) {
        m_pdata.clear ();
        startUploadProcess (item);
    }
}

void UploadProcess::stopUpload (UploadItem * item) {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    if ((item != m_currItem) && (item != 0)) {
        qDebug() << "Input item does not match item being uploaded. "
            "Ignoring the stop request";
        return;
    }

    if (isActive ()) {
        m_stopping = true;
        send (m_pdata.stop ());
        // Wait for the process to get completed. The default timeout for
        // waitForFinished is 3000 ms. WaitForFinished will fail if the process
        // is already terminated or time out. If timeout, kill the process
        // explicitly.
        if (m_currentProcess->waitForFinished() == false) {
            if (m_currentProcess != 0) {
                qDebug() << "Process termination timeout, " <<
                    "killing process explicitly";
                m_currentProcess->kill();
                m_currentProcess = 0;
                
                // Send upload stopped signal since we have killed the process
                // explicitly to stop it
                Q_EMIT (uploadStopped (item));
            }
        }
    }
    else {
        qCritical() << "The process was not active. Ignoring.";
    }

    return;
}

void UploadProcess::processStarted () {
    qDebug() << "UploadProcess::" << __FUNCTION__;
    if (isActive () == false)
        return;

    WebUpload::Error currError = m_currItem->takeError ();
    QString xmlPath = m_currEntry->serializedTo();

    send (m_pdata.startUpload (xmlPath, currError));

    return;
}

void UploadProcess::sendingMedia (quint32 index) {
    qDebug() << "UploadProcess::" << __FUNCTION__;
    WebUpload::Media * media;
    if ((m_currMediaIdx >= 0) && 
        ((quint32)m_currMediaIdx < m_currEntry->mediaCount())) {

        media = m_currEntry->mediaAt (m_currMediaIdx);
        media->refreshStateFromTracker ();
    }

    if (index > m_currEntry->mediaCount()) {
        qDebug() << "Invalid media index sent" << index;
        return;
    }

    m_currMediaIdx = index;

    m_currItem->markActive ();
    m_currItem->mediaStarted (index);
    return;
}

void UploadProcess::done () {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    if (m_resultHandled) {
        qDebug() << "Ignoring this signal";
        return;
    }

    m_resultHandled = true;
    // Can set the current process to NULL and disconnect from m_pdata before
    // doing further processing of the media and entry since these are not
    // needed any more here.
    m_currentProcess = 0;
    m_pdata.disconnect (m_currItem);

    WebUpload::Media * media;
    qDebug () << "Current media index is " << m_currMediaIdx;
    if ((m_currMediaIdx >= 0) && 
        ((quint32)m_currMediaIdx < m_currEntry->mediaCount())) {

        media = m_currEntry->mediaAt (m_currMediaIdx);
        media->refreshStateFromTracker ();
    }

    m_currItem->uploadProgress (1.0);

    Q_EMIT (uploadDone (m_currItem));
}

void UploadProcess::stopped () {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    if (m_resultHandled) {
        qDebug() << "Ignoring this signal";
        return;
    }
    m_resultHandled = true;
    m_stopping = false;

    WebUpload::Media * media;
    if ((m_currMediaIdx >= 0) && 
        ((quint32)m_currMediaIdx < m_currEntry->mediaCount())) {

        media = m_currEntry->mediaAt (m_currMediaIdx);
        media->refreshStateFromTracker ();
    }

    m_pdata.disconnect (m_currItem);
    m_currentProcess = 0;

    Q_EMIT (uploadStopped (m_currItem));
}

void UploadProcess::failed (WebUpload::Error error) {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    if (m_resultHandled) {
        qDebug() << "Ignoring this signal";
        return;
    }
    m_resultHandled = true;

    WebUpload::Media * media;
    if ((m_currMediaIdx >= 0) && 
        ((quint32)m_currMediaIdx < m_currEntry->mediaCount())) {

        media = m_currEntry->mediaAt (m_currMediaIdx);
        media->refreshStateFromTracker ();
    }

    // Set service name in the error
    WebUpload::SharedAccount account = m_currEntry->account ();
    QString accountName;
    if (account != 0) {
        accountName = account->service()->name ();
    } 
    account.clear();
    error.setAccountName (accountName);
    qDebug() << "Set account name as " << accountName;

    m_pdata.disconnect (m_currItem);
    m_currentProcess = 0;

    Q_EMIT (uploadFailed (m_currItem, error));
}

void UploadProcess::optionValueChanged (QString optionId, 
    QVariant optionValue, int mediaIndex) {

    qDebug() << "UploadProcess::" << __FUNCTION__ << optionId <<
        optionValue << mediaIndex;
    
    if (mediaIndex >= (int)m_currEntry->mediaCount ()) {
        qWarning () << "Invalid media index used" << mediaIndex;
        return;
    }

    if (mediaIndex == -1) {
        bool intValue = optionValue.canConvert<int>();
        bool strValue = optionValue.canConvert<QString>();
        if ((optionId == "image_resize") || (optionId == "video_resize")) {
            qWarning () << "No sense asking for image/video resize option now";
            qDebug () << "Ignoring option change request for" << optionId 
                << "...";
            return;
        }

        if (optionId == "metadata_filter") {
            if (!intValue) {
                qWarning () << "Invalid value passed for metadata_filter" <<
                    optionValue;
                return;
            }

            int value = optionValue.toInt ();
            m_currEntry->setMetadataFilter (value);
        } else {
            if (!strValue) {
                qWarning() << "Cannot handle non-string values for option" <<
                    optionId << optionValue;
                return;
            }

            QString value = optionValue.toString ();
            m_currEntry->setOption (optionId, optionValue.toString());
        }
    } else {
        WebUpload::Media * media = m_currEntry->mediaAt (m_currMediaIdx);

        bool strValue = optionValue.canConvert<QString>();
        if (!strValue) {
            qWarning() << "Cannot handle non-string values for media options"
                << optionId << optionValue;
            return;
        }

        QString value = optionValue.toString ();
        if (optionId == "title") {
            media->setTitle (value);
        } else if (optionId == "description") {
            media->setDescription (value);
        } else if (optionId == "tag" || optionId == "geotag") {
            qWarning() << "Cannot handle tag and geotag changes";
            return;
        } else {
            media->setOption (optionId, value);
        }
    }

    // To ensure that the changed option gets written back to the xml file
    m_currEntry->reSerialize ();
    return;
}


void UploadProcess::pluginProcessCrashed () {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    if (m_resultHandled) {
        qDebug() << "Ignoring this signal";
        return;
    }

    if (m_stopping) {
        stopped();
        return;
    }

    m_resultHandled = true;

    qWarning() << "Upload process crashed";

    m_pdata.disconnect (m_currItem);

    WebUpload::Error error = WebUpload::Error::transferFailed ();
    Q_EMIT (uploadFailed (m_currItem, error));
    return;
}


bool UploadProcess::canProcessNewRequest (UploadItem * item) {
    bool retVal = true;

    if (item == 0) {
        qDebug() << "Null item recieved";
        WebUpload::Error error = WebUpload::Error::custom (
            "Null item recieved", "Null item recieved");
        Q_EMIT (uploadFailed (item, error));
        retVal = false;
    } else if (isActive ()) {
        qDebug() << "Upload already being processed";
        WebUpload::Error error = WebUpload::Error::custom (
            "Cannot process upload request", 
            "One upload is already being processed");
        Q_EMIT (uploadFailed (item, error));
        retVal = false;
    }

    qDebug() << "UploadProcess::" << __FUNCTION__ << retVal;
    return retVal;
}

void UploadProcess::startUploadProcess (UploadItem * item) {
    qDebug() << "UploadProcess::" << __FUNCTION__;

    m_resultHandled = false;
    m_stopping = false;
    m_currMediaIdx = -1;
    m_currItem = item;
    m_currEntry = m_currItem->getEntry ();
    Q_ASSERT (m_currEntry != 0);

    unsigned int notSent = m_currEntry->mediaCount () - 
        m_currEntry->mediaSentCount ();
    if (notSent == 0) {
        qDebug() << "No more files to send";
        m_currItem = 0;
        Q_EMIT (uploadDone (item));
        return;
    }

    // WebUpload::Account * account = m_currEntry->account ().data ();
    WebUpload::SharedAccount account = m_currEntry->account ();
    if (account == 0) {
        qWarning() << "Failed to load account";
        /* Section 5.3.6 - if account could not be loaded, then it has probably
         * been deleted
         */
        WebUpload::Error itemError = WebUpload::Error::accountRemoved();
        itemError.setFailedCount (notSent);
        m_currItem = 0;
        account.clear ();
        Q_EMIT (uploadFailed (item, itemError));
        return;
    }

    // WebUpload::PluginProcess::startProcess expects WebUpload::Account * 
    bool startProcessResult = startProcess (account.data ());
    account.clear ();

    if (startProcessResult == false) {
        WebUpload::Error error = WebUpload::Error::custom (
            "Invalid plugin", "Cannot upload given request");
        m_currItem = 0;
        Q_EMIT (uploadFailed (item, error));
        return;
    }

    connect (&m_pdata, SIGNAL (progressSignal(float)), m_currItem,
        SLOT (uploadProgress(float)), Qt::QueuedConnection);

    return;
}
