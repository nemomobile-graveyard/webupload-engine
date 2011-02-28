 
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

#ifndef _UPLOAD_ENGINE_H_
#define _UPLOAD_ENGINE_H_

#include <QtCore>
#include <QString>
#include <QMap>
#include <QList>
#include <TransferUI/Client>
#include "uploadqueue.h"
#include "uploadprocess.h"
#include "connectionmanager.h"
#include "processthread.h"

// For getting signals when usb is connected in mass storage mode
#include <qmusbmode.h>


/*!
   \file    UploadEngine.h
   \brief   This file contains the definitions of the following type:
            UploadEngine : The class the provides the upload engine behaviour
 */

// Forward declaration of the class that stores the information of each upload
class UploadItem;
class WebuploadengineAdaptor;

/*!
   \class  UploadEngine
   \brief  This class is the main class that manages the upload engine. This
           class currently contains the definition of the slot to which it
           recieves new upload requests from other applications.
   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class UploadEngine : public QCoreApplication
{
    Q_OBJECT

public:

    //! \brief  Constructor
    UploadEngine(int argc, char **argv);

    //! \brief  Destructor
    virtual ~UploadEngine();
    
    //! \brief States of upload engine
    enum State {
        IDLE, //!< Upload engine is idle (nothing to send)
        OFFLINE, //!< Upload engine is waiting for proper connectivity
        SENDING, //!< Upload engine is sending
        FAILED, //!< Upload engine is blocked by failure
        PAUSED, //!< Upload engine is blocked by pause
        SHUTTING_DOWN, //!< Upload engine is shutting down
        MASS_STORAGE //!< Device is in mass storage mode
    };
    
    /*!
      \brief Get state of upload engine
      \return Current state of upload engine
     */
    State getState () const;

public Q_SLOTS:

    /*!
      \brief Slot that processes new upload requests.
      \param path Path to upload definition
      \return Always returns <code>true</code>
     */
    bool newUpload (const QString &path);
    
    //! \brief Shutdown upload engine (will stop all ongoing uploads)
    void shutdown();

    /*!
      \brief Slot to handle the event raised when processing of an upload item
             has been completed.
      \param item Item done
     */
    void processDone (UploadItem * item);

    /*!
      \brief Slot to handle the event raised when processing of an upload item
             has been stopped.
      \param item Item done
     */
    void processStopped (UploadItem * item);

    /*!
      \brief Slot to handle the event raised when processing of an upload item
             has failed.
      \param item Item done
      \param processErrorCode Error code, see UploadItem::ProcessError
     */
    void processThreadFailed (UploadItem * item, int processErrorCode);

    /*!
      \brief Slot to handle the done event of a transfer from the
             UploadThread/UploadProcess.
      \param item Item done
     */
    void uploadDone (UploadItem * item);

    /*!
      \brief Slot to handle the stopped event of a transfer from the
             UploadThread/UploadProcess.
      \param item Item stopped
     */
    void uploadStopped (UploadItem * item);

    /*!
      \brief Slot to handle the upload failure of an item
      \param item Item whose upload failed
      \param error Details of the error
     */
    void uploadFailed (UploadItem * item, WebUpload::Error error);

private Q_SLOTS:

    /*!
      \brief Slot that processes new upload requests. Connects to the signal
             uploadReceived emitted from newUpload
      \param path Path to upload definition
     */
    void newUploadReceived (const QString &path);

    //! \brief Slot for queue to tell when it's empty
    void queueDone ();
    
    /*!
      \brief Slot for queue to tell when there is new top item
      \param item New top item
     */
    void queueTop (UploadItem * item);

    /*!
      \brief Slot for queue to tell when reordering has happend to top item.
      \param down Old top item
      \param up New top item
     */
    void queueChangeTop (UploadItem * down, UploadItem * top);
    
    //! \brief Slot for item cancel signal (caller should be UploadItem)
    void cancelItem ();

    //! \brief Slot for item repair signal (caller should be UploadItem)
    void repairError ();
        
    //! \brief  Slot for signal when process thread is finished
    void processThreadFinished ();
    
    //! \brief Slot for ConnectionManager::connected
    void connected ();

    //! \brief Slot for ConnectionManager::disconnected
    void disconnected ();
    

    //! \brief Slot for Meego::QmUSBMode::modeChanged
    void usbModeChanged (MeeGo::QmUSBMode::Mode mode);

    /*! 
      \brief Slot for MeeGo::QmUSBMode::fileSystemWillUnmount
             Recieve this signal and stop any ongoing transfers so that device
             can move to mass storage mode
     */
     void fileSystemWillUnmount (MeeGo::QmUSBMode::MountPath path);

Q_SIGNALS:                    

    /*!
      \brief Signal emitted from newUpload
      \param path Path to upload definition
     */
    void uploadReceived (const QString &path);

    /*!
      \brief Signal emitted to inform ProcessThread to process an upload
             item
      \param item Item given to process thread
     */
    void startProcess (UploadItem * item);

    /*!
      \brief Signal emitted to inform ProcessThread to stop processing an
             upload item
      \param item Item given to process thread
     */
    void stopProcess (UploadItem * item);

    /*!
      \brief Signal emitted to inform the UploadThread/UploadProcess to start
             an upload
      \param item Item given to upload thread
     */
    void startUpload (UploadItem * item);
    
    /*!
      \brief Signal emitted to inform the UploadThread/UploadProcess to stop an
             upload
      \param item Item given to upload thread
     */
    void stopUpload (UploadItem * item);
    
    /*!
      \brief Signal emitted for queue
      \param item Item removed from queue
     */
    void removeUpload (UploadItem * item);
    
    /*!
      \brief Upload engine state has changed
      \param newState New state of upload engine
     */
    void stateChanged (State newState);

private:

    //! \brief  Start process thread
    void startProcessThread ();

    //! \brief Stop process thread if running
    void stopProcessThread();
    
#ifndef TEST_PROCESS_SPLIT
    //! \brief Start upload thread if not running
    void startUploadThreadIfNeeded();
    
    //! \brief Stop upload thread if running
    void stopUploadThread();
#endif
    
    /*!
      \brief Set state and emit signals
      \param newState New state
     */
    void setState (State newState);

    WebUpload::ConnectionManager connection; //!< Connection manager class
    UploadQueue queue; //!< Upload queue
    UploadProcess uploadProcess; //!< Communicate with upload process
    TransferUI::Client * tuiClient; //!< TransferUI client
    bool shutdownWhenEmptyQueue; //!< Shutdown when queue becomes empty
    State state; //!< State of upload engine
    ProcessThread * processThread; //!< Process thread

    MeeGo::QmUSBMode usbModeDetector; //!< To get signals regarding USB mode
    MeeGo::QmUSBMode::Mode currentUSBMode; //!< Current USB mode

    QList<UploadItem*> m_stoppingItems;
};


#endif // _UPLOAD_ENGINE_H_
