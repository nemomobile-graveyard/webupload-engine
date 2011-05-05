 
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
 
/*!
  \file UploadItem.h
  \brief This file contains the definitions of the following type:
         UploadItem   : The class representing one item in the upload queue
 */ 

#ifndef _UPLOAD_ITEM_H_
#define _UPLOAD_ITEM_H_

#include <QString>
#include <QVectorIterator>
#include <TransferUI/Client>
#include <TransferUI/Transfer>
#include <QMetaType>
#include "WebUpload/Entry"
#include "WebUpload/Media"
#include "WebUpload/Error"
#include "uploadstatistics.h"

/*!
  \class  UploadItem
  \brief  This class contains details of one upload. The UploadEngine
          maintains a linked list (**temporary data structure**) of instances
          of this class, one instance per valid upload request. A valid upload
          request is one which is either not completed yet, or if it has an
          error, the error is a recoverable error.
  \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class UploadItem : public QObject {

    Q_OBJECT

public:

    /*!
      \brief Create new upload item
      \param parent QObject parent
     */
    UploadItem (QObject * parent = 0);
    
    virtual ~UploadItem();
    
    //! Pending reason for item
    enum PendingReason {
        PENDING_CONNECTIVITY, //!< Item waits for connectivity
        PENDING_QUEUED, //!< Item is in queue
        PENDING_PROCESSING, //!< Item is being processed
        PENDING_MSM //!< Item waits for mass storage mode to be disabled on device
    };

    //! Owner options for item
    enum Owner {
        OWNER_QUEUE, //!< Item is in the queue
        OWNER_UPLOAD_THREAD, //!< Item is currently being uploaded
        OWNER_PROCESS_THREAD //!< Item is currently being processed
    };
    
    //! Process error codes for items
    enum ProcessError {
        PROCESS_ERROR_UNDEFINED, //!< Undefined error
        PROCESS_ERROR_STORAGE_MEMORY_FULL, //!< Out of storage/flash memory
        PROCESS_ERROR_OUT_OF_MEMORY, //!< Not enough free memory (RAM)
        PROCESS_ERROR_FILE_NOT_FOUND, //!< Source file not found
        PROCESS_ERROR_FILE_CHANGED //!< Source file has changed
    };        
    
    /*!
      \brief Init item from entry XML
      \param path Path to entry XML
      \param tuiClient Transfer UI client used to connect item to UI
      \return true if init was success
     */
    bool init (const QString & path, TransferUI::Client * tuiClient = 0);

    /*!
      \brief Set owner of the item
      \param owner Enumeration defining who owns the item currently
     */
    void setOwner (Owner owner);

    /*!
      \brief  Check who owns the item currently
      \return Enumeration signifying who the owner is
     */
    Owner getOwner () const;

    /*!
      \brief  Get the id associated with this transfer
      \return String which is the id associated with this transfer or empty
              string if no id is associated with the transfer yet
     */
    QString getTransferId ();
    
    /*!
      \brief Get entry related to item
      \return Entry related to item
     */
    WebUpload::Entry * getEntry();

    /*!
      \brief  Get the next media that needs processing
      \return WebUpload::Media instance that needs a copy file to be made, or
              NULL if there are no more media that need to be sent
     */
    WebUpload::Media *getNextUnprocessedMedia();
    
    /*!
      \brief Get current error 
      \return Error stored for the item. If there was no error, the code
              would be WebUpload::Error::CODE_NO_ERROR
     */
    WebUpload::Error getError();
    
    /*!
      \brief Take current error 
      \return Return the error stored for the item and clear it out. If
              there was no error, the code would be
              WebUpload::Error::CODE_NO_ERROR
     */
    WebUpload::Error takeError();
    
    //! \brief Mark item cancelled
    bool setCancelled();
    
    /*!
      \brief Check if item is marked cancelled
      \return true if item is marked cancelled
     */
    bool isCancelled() const;

    /*!
      \brief  Checks if the item has been processed. Then we can start
              uploading it
      \return true if processed, else false
    */
    bool isProcessed() const;
    
    /*!
      \brief Give string presentation of item. Can be used in logging.
      \return String presentation of item
     */
    QString toString() const;
    
    /*!
      \brief What icon to sent to TUI
      \param media For which media we resolve the icon
      \param isThumbnail Will be set to <code>true</code> if return value is
                         path not name.
      \return Icon name or path
     */
    static QString iconForMedia (WebUpload::Media * media, bool & isThumbnail);    
    
Q_SIGNALS:
    
    //! \brief Signal emitted when processing of a media is completed
    void itemProcessingDone ();

    /*!
      \brief Signal emitted when item should be cancelled (from
             TransferUI::Transfer)
     */
    void cancel();

    /*!
      \brief Signal emitted when item should be started (from
             TransferUI::Transfer).
     */
    void start();

    //! \brief Signal emitted when item has error and needs repair
    void repairError ();
    
public Q_SLOTS:
    
    /*!
      \brief Slot for upload progress from upload thread
      \return true if information could be sent to TUI, else false
     */
    bool uploadProgress (float done);
    
    /*!
      \brief Slot for status update
      \return true if information could be sent to TUI, else false
     */
    bool status (const QString & message);
    
    /*!
      \brief  Tell upload item that it's done
      \return true if information could be sent to TUI, else false
     */
    bool markDone ();
    
    /*!
      \brief Slot for failure marking
      \return true if information could be sent to TUI, else false
     */
    bool markFailed (const WebUpload::Error & error);
    
    /*!
      \brief Slot for marking item active
      \return true if information could be sent to TUI, else false
     */
    bool markActive();
    
    /*!
      \brief Slot for marking item pending
      \param reason Why item is pending
      \return true if information could be sent to TUI, else false
     */
    bool markPending (PendingReason reason);
    
    /*!
      \brief Slot for handling Media Started Signal. When this signal is
             received, the properties of the media is sent to TUI.
      \param mediaIndex Index of the media for which upload stated
    */
    void mediaStarted (quint32 mediaIndex);
    
    /*!
      \brief Slot for handling Media Started Signal. When this signal is
             received, the properties of the media is sent to TUI.
      \param media Media for which upload stated
    */
    void mediaStarted (WebUpload::Media *media);
    
private Q_SLOTS:

    /*! 
      \brief Slot handling itemProcessingDone signal. This sends the size of
             the transfer to the TUI
     */
    void processingDone ();

    //! \brief Slot for UploadStatistics
    void estimateTime (int seconds);

private:

    /*!
      \brief Get presentation string for given media
      \return Presentation string - string to be shown in TUI for that media
     */
    static QString getPresentationString(WebUpload::Media * media);

    TransferUI::Transfer * m_tuiTransfer; //!< TUI transfer related to item
    WebUpload::Entry * m_entry; //!< Entry related to item
    WebUpload::Error m_error;
    bool m_cancelled;
    bool m_processed;
    QVectorIterator<WebUpload::Media *> * m_mediaIter;
    WebUpload::Media * m_currMedia; //!< Pointer to the media being processed. Not used during upload
    UploadStatistics m_statistics; //!< Class for estimates
    quint64 m_totalSize;
    int m_filesCompletedCount;
    //! Enum signifying who is using/working with this UploadItem currently.
    Owner m_ownerType; 
};

Q_DECLARE_METATYPE(UploadItem::ProcessError)

Q_DECLARE_METATYPE(WebUpload::Media*)

#endif // _UPLOAD_ITEM_H_
