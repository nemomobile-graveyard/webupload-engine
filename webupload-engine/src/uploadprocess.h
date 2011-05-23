 
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

#ifndef _UPLOAD_PROCESS_H_
#define _UPLOAD_PROCESS_H_

#include <QProcess>
#include "uploaditem.h"
#include "WebUpload/Error"
#include "WebUpload/processexchangedata.h"
#include "WebUpload/pluginprocess.h"

/*!
   \class  UploadProcess
   \brief  This class manages the process in which the actual uploads are done.
           
           Please note, there is a primary assumption that only one upload can
           be active at a time. This class definition currently does not cater
           to multiple uploads happening at the same time.

   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class UploadProcess : public WebUpload::PluginProcess {

    Q_OBJECT

public:

    /*!
      \brief Constructor
      \param parent : Pointer to the QObject parent of this class (in this
                case, it will be a pointer to the UploadEngine instance)
     */
    UploadProcess (QObject *parent = 0);

    //! \brief Destructor 
    virtual ~UploadProcess ();

    UploadItem * currentlySendingMedia() const;

    /*!
      \brief Checks whether the current process has been sent the stop request
             already or not.
      \return <bool>true</code> if the current process has been sent the stop
              request, else <code>false</code>
     */
    bool isProcessStopping () const;

Q_SIGNALS:

    /*!
      \brief Signal emitted when upload is successfully done
      \param item Item uploaded successfully
     */
    void uploadDone (UploadItem *item);

    /*!
      \brief Slot emitted when an upload is prematurely stopped
      \param item Item whose upload that was stopped
     */
    void uploadStopped (UploadItem *item);

    /*!
      \brief Signal emitted when upload has failed
      \param item Item whose upload failed
      \param error Details of the error
     */
    void uploadFailed (UploadItem * item, WebUpload::Error error);

public Q_SLOTS:

    /*!
      \brief Signal emitted to tell the UploadHandler to start processing an
             upload. This signal is emitted by being connected to a signal of
             the parent of this class.
      \param item Upload item asked to be uploaded
     */
    void startUpload (UploadItem *item);
    
    /*!
      \brief Signal emitted to tell the UploadHandler to stop processing an
             upload.
      \param item Upload item which should be stopped, or pointer with value 0
             if we just want upload to be stopped irrespective of which item is
             being uploaded
     */
    void stopUpload (UploadItem *item);

protected Q_SLOTS:
    
    //! \brief Reimplementation of PluginProcess::processStarted
    virtual void processStarted ();
    
private Q_SLOTS:
    
    /*!
      \brief Connects to ProcessExchangeData::sendingMediaSignal() signal
      \param index Index of the media file being currently sent
     */
    void sendingMedia (quint32 index);

    //! \brief Connects to ProcessExchangeData::doneSignal() signal
    void done ();

    //! \brief Connects to ProcessExchangeData::stoppedSignal() signal
    void stopped ();

    /*!
      \brief Connects to ProcessExchangeData::uploadFailedSignal() signal
      \param error Error with which upload failed
     */
    void failed (WebUpload::Error error);

    /*!
      \brief Connects to ProcessExchangeDate::optionValueChangedSignal signal
      \param optionId Id of the option that has changed
      \param optionValue Changed value for the option
      \param mediaIndex  If this is -1, then the option applies to all media,
                ie., it is an entry option, otherwise it is the index of the
                medi for which the option change is applicable

                If the index is greater than the count of media the entry has,
                then the change request is ignored
     */
    void optionValueChanged (QString optionId, QVariant optionValue,
        int mediaIndex);

    //! \brief Connects to PluginProcess::currentProcessStopped signal
    void pluginProcessCrashed ();

private:

    void startUploadProcess (UploadItem * item);
    bool canProcessNewRequest (UploadItem * item);

    UploadItem * m_currItem; //!< Item currently being uploaded
    //! Entry corresponding to item being uploaded
    WebUpload::Entry * m_currEntry; 
    int m_currMediaIdx; //!< Index of media currently being uploaded

    bool m_resultHandled;
    bool m_stopping;
};

#endif // _UPLOAD_PROCESS_H_
