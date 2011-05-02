 
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

#include "uploaditem.h"
#include "WebUpload/Entry"
#include "WebUpload/Account"
#include "WebUpload/Service"
#include <QDebug>
#include "logger.h"
#include <MDataUri>

#define CLIENT_ERROR_WARNING_STMT \
    WARNSTREAM << "TUI client Error:" << m_tuiTransfer->lastError()

UploadItem::UploadItem(QObject * parent) : QObject (parent), m_tuiTransfer (0),
    m_entry(0), m_cancelled (false), m_processed (false), m_mediaIter (0),
    m_currMedia (0), m_totalSize (0), m_filesCompletedCount (0),
    m_ownerType (OWNER_QUEUE) {

    connect (&m_statistics, SIGNAL (timeLeftEstimate(int)), this, 
        SLOT (estimateTime(int)));
    connect (this, SIGNAL (itemProcessingDone()), this,
        SLOT (processingDone()), Qt::QueuedConnection);
}

UploadItem::~UploadItem() {
    if (m_entry != 0) {
        m_entry->reSerialize();
    }

    if (m_mediaIter != 0) {
        delete m_mediaIter;
    }

    if (m_tuiTransfer != 0) {
        m_tuiTransfer->disconnect (this);
    }
}

bool UploadItem::init (const QString & path, TransferUI::Client * tuiClient) {
    if (m_entry != 0) {
        WARNSTREAM << "UploadItem Already initialized";
        return false;
    }

    DBGSTREAM << "Starting to read upload item ";
    m_entry = new WebUpload::Entry (this);
    if(!m_entry->init(path)) {
        WARNSTREAM << "Could not initialize Entry with" << path;
        delete m_entry;
        m_entry = 0;
        return false;
    }

    m_totalSize = m_entry->totalSize();
    m_mediaIter = new QVectorIterator<WebUpload::Media *>(m_entry->media());

    m_processed = true;
    // If the state of the entry is something other than pending, no need to
    // bother checking if it is processed or not - either it is cancelled or it
    // has already been processed
    if (m_entry->isPending()) {
        while ((m_processed) && (m_mediaIter->hasNext())) {
            WebUpload::Media *media = m_mediaIter->next();
            if ((media->isPending()) && media->copyFilePath().isEmpty()) {
                m_processed = false;
            }
        }

        // Rewind back to the front
        m_mediaIter->toFront();
    }
    
    if (tuiClient == 0) {
        WARNSTREAM << "No TUI connection, skipped TUI register";
        return true;
    }

    DBGSTREAM << "Registering transfer in TUI ";
    //Get the first media to display thumbnail / title
    WebUpload::Media *media = 0;
    if (m_mediaIter->hasNext()) {
         media = m_mediaIter->next();
    } else {
        WARNSTREAM << "No Media in the entry" ;
        return false;
    }
    m_mediaIter->toFront();
    QString transferName = getPresentationString (media);
    m_tuiTransfer = tuiClient->registerTransfer (transferName,
        TransferUI::Client::TRANSFER_TYPES_UPLOAD);
    
    if (m_tuiTransfer != 0) {
        DBGSTREAM << "Registered transfer to TUI with id"
            << m_tuiTransfer->transferId();
    
        connect (m_tuiTransfer, SIGNAL (cancel()), this,
            SIGNAL (cancel()));
        connect (m_tuiTransfer, SIGNAL (start()), this,
            SIGNAL (start()));
        connect (m_tuiTransfer, SIGNAL (repairError()), this,
            SIGNAL (repairError()));
        
        //Group TUI changes
        m_tuiTransfer->waitForCommit();
            
        //provide account information to TUI
        WebUpload::SharedAccount account = m_entry->account();
        // WebUpload::Account *account = m_entry->account().data();
        if (account) {
            //service()->name provides the proper service name
            QString accName = account->service()->name();
            m_tuiTransfer->setTargetName (accName);
            // account = 0;
            account.clear ();
        }
        
        //set thumbnail
        bool isThumbnail = false;
        QString iconName = UploadItem::iconForMedia (media, isThumbnail);
        
        m_tuiTransfer->setIcon(iconName);
        
        if (isThumbnail == true) {
            if (media->origURI().isEmpty()) {
                // In case where original file might have already been deleted,
                // we send the copy file path for thumbnail setting
                m_tuiTransfer->setImageFromFilePath (media->copyFilePath());
            } else {
                m_tuiTransfer->setThumbnailForFile
                    (media->origURI().toLocalFile(), media->mimeType());
            }
        }
        
        //set total no of files
        m_tuiTransfer->setFilesCount(m_entry->mediaCount());
        
        // Send size to TUI only if this ittem has been processed. Otherwise
        // size will always be 0
        if (m_processed) {
            m_tuiTransfer->setSize (m_totalSize);
        }
        
        //Commit TUI changes        
        m_tuiTransfer->commit();            
        
    } else {
        WARNSTREAM << "Failed to register transfer"
            << m_entry->trackerIRI() << "to TUI";
    }
    
    return true;
}

void UploadItem::setOwner (UploadItem::Owner owner) {
    DBGSTREAM << "Change owner of item" << this->toString() << "from"
        << m_ownerType << "to" << owner;
    m_ownerType = owner;
}

UploadItem::Owner UploadItem::getOwner () const {
    return m_ownerType;
}

WebUpload::Entry * UploadItem::getEntry () {
    return m_entry;
}

WebUpload::Media * UploadItem::getNextUnprocessedMedia () {
    bool unprocessedMediaFound = false;

    // This entry should not have any more unprocessed media since processed
    // flag is set to true
    if (m_processed) {
        return 0;
    }

    if (m_entry->isPending() != true) {
        CRITSTREAM << "Media of only pending uploads can be processed";
        return 0;
    }

    if ((m_currMedia != 0) && (m_currMedia->copyFilePath().isEmpty()) &&
        (m_currMedia->isPending()))
        return m_currMedia;

    while (m_mediaIter->hasNext()) {
        m_currMedia = m_mediaIter->next();
        bool isFile = (m_currMedia->type() == WebUpload::Media::TYPE_FILE);
        
        if (isFile == true) {
        
            if (m_currMedia->copyFilePath().isEmpty() && 
                m_currMedia->isPending ()) {
            
                DBGSTREAM << "Media file" << m_currMedia->fileName()
                    << "not processed"; 
                unprocessedMediaFound = true;
                break;
            }

            DBGSTREAM << "Media File" << m_currMedia->fileName()
                << "already processed or already completed";
                
        } else {
            DBGSTREAM << "No processing for non file media";
        }
    }

    if(!unprocessedMediaFound) {
        m_currMedia = 0;
        m_processed = true;
        // Now item is processed. Set size
        m_totalSize = m_entry->totalSize ();
        Q_EMIT (itemProcessingDone());

        // Reset the media iterator so it can be used for uploading now
        m_mediaIter->toFront();
    }

    return m_currMedia;
}

void UploadItem::processingDone() {
    if (m_tuiTransfer != 0) {
        m_tuiTransfer->setSize (m_totalSize);
    }
}

WebUpload::Error UploadItem::getError() {
    return m_error;
}

WebUpload::Error UploadItem::takeError() {
    WebUpload::Error retVal = m_error;
    m_error.clearError ();

    return retVal;
}

bool UploadItem::uploadProgress (float done) {   
    bool ret = false;

    if (m_tuiTransfer != 0) {
        if ((0.0 <= done) && (done <= 1.0)) {
            if (!m_statistics.nowDone (done)) {
                DBGSTREAM << "Ignoring this done value";
                return false;
            }
        }

        if (!(ret = m_tuiTransfer->setProgress (done))) {
            CLIENT_ERROR_WARNING_STMT;
        } 
    }

    return ret;
}

bool UploadItem::status (const QString & message) {
    bool ret = false;

    if (m_tuiTransfer != 0) {
        ret = m_tuiTransfer->setMessage (message);
    }

    return ret;
}

bool UploadItem::markDone () {

    Q_ASSERT (m_entry != 0);

    bool ret = false;

    if (!m_entry->reSerialize()) {
        WARNSTREAM << "Failed to reserialize";
    }

    if (m_tuiTransfer != 0 && !m_cancelled) {    
        ret = m_tuiTransfer->markCompleted (true);
    }

    return ret;
}


bool UploadItem::markFailed (const WebUpload::Error & newError) {

    Q_ASSERT (m_entry != 0);

    bool ret = false;

    m_error = newError;
    
    if (!m_entry->reSerialize()) {
        WARNSTREAM << "Failed to reserialize";
    }    
    
    if (m_tuiTransfer != 0) {
        // Set thumbnail to first failed media's image
        int mediaCount = m_entry->mediaCount ();
        for (int i = 0; i < mediaCount; ++i) {
            WebUpload::Media *media = m_entry->mediaAt (i);
            // The first unsent media would definitely have an error since we 
            // upload media sequentially.
            if (!media->isSent ()) {
                QString transferName = getPresentationString (media);
                m_tuiTransfer->setName (transferName);

                if (media->origURI().isEmpty()) {
                    // In case where original file might have already been
                    // deleted, we send the copy file path for thumbnail
                    // setting
                    m_tuiTransfer->setImageFromFilePath
                        (media->copyFilePath());
                } else {
                    m_tuiTransfer->setThumbnailForFile
                        (media->origURI().toLocalFile(), media->mimeType());
                }
                break;
            }
        }
    }

    if (m_tuiTransfer != 0) {
        if (newError.recoverMsg().isEmpty()) {
            DBGSTREAM << "Calling tuiTransfer->markFailure";
            ret = m_tuiTransfer->markFailure (newError.title(), 
                newError.description());
        } else {
            DBGSTREAM << "Calling tuiTransfer->markRepairableFailure";
            ret = m_tuiTransfer->markRepairableFailure (newError.title(), 
                newError.description(), newError.recoverMsg());
        }
    }    

    DBGSTREAM << "markFailed, returning " << ret;
    return ret;
}

bool UploadItem::markActive() {
    DBGSTREAM << "Setting item as active";
    bool ret = false;

    if (m_tuiTransfer != 0) {
        ret = m_tuiTransfer->setActive (m_statistics.lastProgressValue());
    }        

    return ret;
}

QString UploadItem::getTransferId () {
    QString tid;
    if (m_tuiTransfer != 0) {
        tid = m_tuiTransfer->transferId ();
    }

    return tid;
}

bool  UploadItem::setCancelled() {
    bool ret = false;
    m_cancelled = true;
    
    m_entry->cancel ();

    if (m_tuiTransfer != 0) {
        if (!(ret = m_tuiTransfer->markCancelled ())) {
            CLIENT_ERROR_WARNING_STMT;
        }
    }

    return ret;
}

bool UploadItem::isCancelled() const {
    return m_cancelled;
}

bool UploadItem::isProcessed() const {
    return m_processed;
}

bool UploadItem::markPending (PendingReason reason) {
    bool ret = false;
    QString pReason;
    
    if (m_tuiTransfer != 0) {
        switch (reason) {
            case PENDING_CONNECTIVITY:
                //% "No internet connection"
                pReason = qtTrId ("qtn_comm_share_no_connection");
                break;
            case PENDING_QUEUED:
                //% "Waiting for previous upload to finish"
                pReason = qtTrId ("qtn_tui_transfer_waiting");
                break;
            case PENDING_PROCESSING:
                //% "Processing files before transfer"
                pReason = qtTrId ("qtn_tui_transfer_waiting2"); 
                break;
            case PENDING_MSM:
                //% "Transfer is disabled in mass storage mode"
                pReason = qtTrId ("qtn_tui_transfer_waiting_msm");
                break;
            default:
                WARNSTREAM << "Unknown pending state";
                pReason = "Unknown pending state";
                break;
        }

        ret = m_tuiTransfer->setPending (pReason);
    }
    
    return ret;
}

void UploadItem::estimateTime (int seconds) {
    bool ret;
    
    if (m_tuiTransfer != 0) {
        if (!(ret = m_tuiTransfer->setEstimate (seconds))) {
            CLIENT_ERROR_WARNING_STMT;
        }
    }    
}

void UploadItem::mediaStarted (quint32 mediaIndex) {
    WebUpload::Media * media = m_entry->mediaAt (mediaIndex);
    
    DBGSTREAM << "New Media uploading started";
    if ((m_tuiTransfer != 0) && (media != 0)) {

        bool isThumbnail = false;
        QString transferName = getPresentationString (media);
        QString iconName = UploadItem::iconForMedia (media, isThumbnail);
        
        m_tuiTransfer->waitForCommit();
        m_tuiTransfer->setIcon(iconName);
        m_tuiTransfer->setCurrentFileIndex (m_entry->mediaSentCount() + 1);
        m_tuiTransfer->setName (transferName);
        
        if (isThumbnail == true) {
            DBGSTREAM << "Media changed with thumbnail";
            if (media->origURI().isEmpty()) {
                // In case where original file might have already been deleted,
                // we send the copy file path for thumbnail setting
                m_tuiTransfer->setImageFromFilePath (media->copyFilePath());
            } else {
                m_tuiTransfer->setThumbnailForFile
                    (media->origURI().toLocalFile(), media->mimeType());
            }
        }

        m_tuiTransfer->commit();

    }
}

void UploadItem::mediaStarted (WebUpload::Media *media) {
    DBGSTREAM << "New Media uploading started";
    if ((m_tuiTransfer != 0) && (media != 0)) {
        bool isThumbnail = false;
        QString transferName = getPresentationString (media);
        QString iconName = UploadItem::iconForMedia (media, isThumbnail);

        m_tuiTransfer->waitForCommit();
        m_tuiTransfer->setIcon(iconName);
        m_tuiTransfer->setCurrentFileIndex (m_entry->mediaSentCount() + 1);
        m_tuiTransfer->setName (transferName);
        
        if (isThumbnail == true) {
            DBGSTREAM << "Media changed with thumbnail";
            if (media->origURI().isEmpty()) {
                // In case where original file might have already been deleted,
                // we send the copy file path for thumbnail setting
                m_tuiTransfer->setImageFromFilePath (media->copyFilePath());
            } else {
                m_tuiTransfer->setThumbnailForFile
                    (media->origURI().toLocalFile(), media->mimeType());
            }
        }
        
        m_tuiTransfer->commit();
    }
}

QString UploadItem::toString() const {
    return QString::number (((unsigned int)(this)), 16);
}

QString UploadItem::iconForMedia (WebUpload::Media * media,
    bool & isThumbnail) {
    
    isThumbnail = false;
    
    QString iconId = "icon-m-content-file-unknown";
    
    QString mime = media->mimeType();
    bool isFile = (media->type() == WebUpload::Media::TYPE_FILE);

    if (mime.startsWith ("image/")) {
        isThumbnail = isFile;
        iconId = "icon-m-content-image";
    } else if (mime.startsWith ("video/")) {
        isThumbnail = isFile;
        iconId = "icon-m-content-videos";
    } else if ((mime.compare ("text/x-url", Qt::CaseInsensitive) == 0) || 
        (mime.compare ("text/x-uri", Qt::CaseInsensitive) == 0)) {

        iconId = "icon-m-content-url";

    } else if (mime.startsWith("application/") || mime.startsWith("text/")) {
        QString suffix = mime.mid (mime.indexOf ("/") + 1);
        if (mime.contains ("pdf") || mime == "acrobat") {
            // Matching mime types:
            // application/pdf, application/x-pdf, application/acrobat,
            // applications/vnd.pdf, text/pdf, text/x-pdf
            iconId = "icon-m-content-pdf";
        } else if (isTextMime (suffix)) {
            iconId = "icon-m-content-word";
        } else if (isPresentationMime (suffix)) {
            iconId = "icon-m-content-powerpoint";
        } else if (isSpreadSheetMime (suffix)) {
            iconId = "icon-m-content-excel";
        }
    }

    return iconId;
}

QString UploadItem::getPresentationString (WebUpload::Media * media) {

    QString presentationString;

    presentationString = media->option (media->PresentationOptionId);
    if (presentationString.isEmpty () == true) {
        if (media->copiedTextData ().isEmpty ()) {
            // This is mostly a file
            presentationString = media->fileName();
        } else {
            // This is a MDataUri
            MDataUri duri;
            duri.read (media->copiedTextData ());
            presentationString = duri.textData ();
        }
    }

    return presentationString;
}

bool UploadItem::isTextMime (const QString & suffix) {
    // Mime types to be handled here:
    // text/plain
    // application/txt
    // application/rtf
    // application/x-rtf
    // text/rtf
    // application/doc
    // application/msword
    // application/vnd.msword
    // application/vnd.ms-word
    // application/vnd.oasis.opendocument.text
    // application/x-vnd.oasis.opendocument.text
    // application/vnd.openxmlformats-officedocument.wordprocessingml.document
    return (suffix == "plain" || suffix == "txt" || suffix == "doc" ||
            suffix.contains ("word") || suffix.contains ("text") || 
            suffix.contains ("rtf"));
}


bool UploadItem::isPresentationMime (const QString & suffix) {
    // Mime types to be handled here:
    // application/vnd.oasis.opendocument.presentation
    // application/x-vnd.oasis.opendocument.presentation
    // application/vnd.ms-powerpoint
    // application/mspowerpoint
    // application/ms-powerpoint
    // application/powerpoint
    // application/vnd.openxmlformats-officedocument.presentationml.presentation
    // application/vnd.openxmlformats-officedocument.presentationml.slideshow
    return (suffix.contains ("presentation") || 
            suffix.contains ("powerpoint"));
}

bool UploadItem::isSpreadSheetMime (const QString & suffix) {
    // Mime types to be handled here
    // application/vnd.oasis.opendocument.spreadsheet
    // application/x-vnd.oasis.opendocument.spreadsheet
    // application/vnd.ms-excel
    // application/msexcel
    // application/x-msexcel
    // application/vnd.ms-excel
    // application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
    return (suffix.contains ("excel") || suffix.contains ("spreadsheet"));
}
    
