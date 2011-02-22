 
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

#ifndef _PROCESS_HANDLER_H_
#define _PROCESS_HANDLER_H_

#include <QObject>
#include "uploaditem.h"
#include "WebUpload/Media"

/*!
  \class  ProcessThread
  \brief  This class does the actual processing of the UploadIem, creating
          copies for each WebUpload::Media belonging to the upload request.
  \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */

class ProcessHandler : public QObject 
{
    Q_OBJECT
public:
    /*!
      \brief  Constructor
      \param  parent: Pointer to the QObject parent of this class
     */
    ProcessHandler (QObject * parent = 0);

    /*! \brief Destructor */
    virtual ~ProcessHandler ();

Q_SIGNALS:

    /*!
      \brief Signal emitted when processing of an item is successfully done
      \param item Item processed successfully
     */
    void processDone (UploadItem *item);

    /*!
      \brief Signal emitted when processing of an item is stopped before it is
             completed
      \param item Item whose processing was stopped
     */
    void processStopped (UploadItem *item);

    /*! 
      \brief Signal emitted when processing of an item has an error
      \param item Item whose processing failed
      \param processErrorCode Error behind failure. See
             UploadItem::ProcessError
     */
    void processFailed (UploadItem *item, int processErrorCode);

    /*!
      \brief Signal emitted when a media in the upload item has been processed
             successfully. This is used internally to tell this class to
             proceed to the next unprocessed media.
     */
    void mediaProcessed (WebUpload::Media::CopyResult copyResult);

public Q_SLOTS:

    /*!
      \brief Slot invoked when an item needs to be processed.
      \param item Item that needs to be processed
     */
    void startProcess (UploadItem * item);

    /*!
      \brief Slot invoked when an item processing needs to be stopped
      \param item Item whose processing needs to be stopped
     */
    void stopProcess (UploadItem * item);

private Q_SLOTS:

    /*!
      \brief Connects to the signal mediaProcessed. Finds the next unprocessed
             media and if one exists, processes it. Else, emits the
             itemProcessed signal
      \param prevCopyResult Result of previous copy
     */
    void processNextMedia (WebUpload::Media::CopyResult prevCopyResult =
        WebUpload::Media::COPY_RESULT_SUCCESS);

private:
    UploadItem * m_myItem; //!< Item being processed
    WebUpload::Media *m_media; //!< Current media being processed
};

#endif // _PROCESS_HANDLER_H_
