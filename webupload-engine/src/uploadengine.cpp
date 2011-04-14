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


#include "uploadengine.h"
#include "uploaditem.h"
#include "uploadqueue.h"
#include "logger.h"
#include <stdlib.h>

/*****************************************************************
 * UploadEngine class function definitions
 *****************************************************************/

UploadEngine::UploadEngine(int argc, char **argv) :
    QCoreApplication(argc, argv), connection (this), uploadProcess (this),
    tuiClient (new TransferUI::Client (this)), shutdownWhenEmptyQueue (true),
    state (IDLE), processThread (0), usbModeDetector (this) {
    
    // Parse input parameters
    for (int i = 1; i < argc; ++i) {
        QString param = argv[i];
    
        if (param == "--immortal") {
            shutdownWhenEmptyQueue = false;
        }
    }
    
    // Init TUI connection
    if (!tuiClient->init ()) {
        WARNSTREAM << "Failed to open TUI connection. Upload engine will"
            << "run without TUI communication.";
        delete tuiClient;
        tuiClient = 0;
    }

    connect (this, SIGNAL (uploadReceived(QString)), this,
        SLOT (newUploadReceived(QString)), Qt::QueuedConnection);
    
    // - Queue signals ---
    // Connect queue done signal
    connect (&queue, SIGNAL(done()), this, SLOT(queueDone()));   
    // Connect queue top signal
    connect (&queue, SIGNAL(topItem(UploadItem*)), this,
        SLOT(queueTop(UploadItem*)));    
    // Connect queue reorder top signal
    connect (&queue, SIGNAL(replacedTopItem(UploadItem*,UploadItem*)),
        this, SLOT(queueChangeTop(UploadItem*,UploadItem*)));
    // Connect remove signal to queue
    connect (this, SIGNAL(removeUpload(UploadItem*)),
        &queue, SLOT(removeItem(UploadItem*))); 
        
    connect (&connection, SIGNAL (connected()), this, SLOT (connected()));
    connect (&connection, SIGNAL (disconnected()), this, SLOT (disconnected()));

    connect (this, SIGNAL (startUpload(UploadItem*)), &uploadProcess,
        SLOT (startUpload(UploadItem*)));
    connect (this, SIGNAL (stopUpload(UploadItem*)), &uploadProcess,
        SLOT (stopUpload(UploadItem*)));

    connect (&uploadProcess, SIGNAL (uploadDone(UploadItem*)), this,
        SLOT (uploadDone(UploadItem*)), Qt::QueuedConnection);
    connect (&uploadProcess, SIGNAL (uploadStopped(UploadItem*)), this,
        SLOT (uploadStopped(UploadItem*)), Qt::QueuedConnection);
    connect (&uploadProcess,
        SIGNAL (uploadFailed(UploadItem*,WebUpload::Error)), this,
        SLOT (uploadFailed(UploadItem*,WebUpload::Error)), 
        Qt::QueuedConnection);

    connect (&usbModeDetector, SIGNAL(modeChanged(MeeGo::QmUSBMode::Mode)),
        this, SLOT(usbModeChanged(MeeGo::QmUSBMode::Mode)));
    connect (&usbModeDetector, 
        SIGNAL(fileSystemWillUnmount(MeeGo::QmUSBMode::MountPath)),
        this, SLOT(fileSystemWillUnmount(MeeGo::QmUSBMode::MountPath)));
    currentUSBMode = usbModeDetector.getMode ();
}


UploadEngine::~UploadEngine() {
    connection.disconnect (this);
    uploadProcess.disconnect (this);
    usbModeDetector.disconnect (this);
}

bool UploadEngine::newUpload (const QString &path) {
    DBGSTREAM << "New upload task received:" << path;

    Q_EMIT (uploadReceived(path));
    return true;
}

    
void UploadEngine::newUploadReceived (const QString &path) {

    // Safety check for duplicate adding
    if ((queue.itemForEntryPath (path) != 0) || queue.isEntryInit (path))  {
        WARNSTREAM << "Entry" << path << "received multiple times. Ignored";
        return;
    }

    queue.initing (path);
    UploadItem * item = new UploadItem();
    
    if (!item->init (path, tuiClient)) {
        WARNSTREAM << "Invalid upload request with" << path;
        delete item;
        queue.initFailed (path);
        return;
    }
    
    // Connect user input signals
    connect (item, SIGNAL(cancel()), this, SLOT(cancelItem()));
    connect (item, SIGNAL(repairError()), this, SLOT(repairError()));
    
    queue.push (item);
    
    // Mark item to be in queue
    if (queue.size() > 1) {
        item->markPending (UploadItem::PENDING_QUEUED);

        // Handle the case where the new transfer pushed is not the top of the
        // queue, and the process thread is not already running. Only in this
        // case do we need to restart the process thread and send this item for
        // processing. 
        // If the item is the top item, it will get sent to the process thread
        // as a part of the processing of the top item. 
        // If the process thread is running, then this item will get picked up
        // at some point of time when the item before it is finished being
        // processed.
        if (processThread == 0) {
            startProcessThread ();
            item->setOwner (UploadItem::OWNER_PROCESS_THREAD);
            // Mark item as pending processing only if it is the top of the
            // queue
            Q_EMIT (startProcess (item));
        }
    }
}

void UploadEngine::shutdown() {

    setState (SHUTTING_DOWN);
    
    // Disconnect and don't recieve any more signals from any other object
    queue.disconnect (this);
    
    connection.disconnect (this);

    if (uploadProcess.processCount () > 0) {
        uploadProcess.killAll ();
    }

    if (processThread != 0) {
        DBGSTREAM << "Process thread pointer is non-NULL. It is probably "
            "running. Stop it ...";
        stopProcessThread ();
    } else {
        DBGSTREAM << "Shutdown upload engine ...";
        quit();
    }
}

void UploadEngine::queueDone () {

    DBGSTREAM << "Queue done";

    setState (IDLE);

    if (shutdownWhenEmptyQueue) {
        DBGSTREAM << "Queue done. Shutdown upload engine...";    
        shutdown();
    } else {
        DBGSTREAM << "Queue done. Move to idle state.";       
    }
}

void UploadEngine::queueTop (UploadItem * item) {
    Q_ASSERT (item != 0);

    DBGSTREAM << "New top in queue"; 

    if (currentUSBMode == MeeGo::QmUSBMode::MassStorage) {
        usbModeChanged (currentUSBMode);
        return;
    } 
    
    if (item->getOwner () == UploadItem::OWNER_PROCESS_THREAD) {
        DBGSTREAM << "Process thread is owner";
        item->markPending (UploadItem::PENDING_PROCESSING);
    } else if (item->isCancelled ()) {
        DBGSTREAM << "Item was marked cancelled - remove it";
        tuiClient->removeTransfer (item->getTransferId ());
        Q_EMIT (removeUpload (item)); 
    } else if (!item->isProcessed ()) {
        if (processThread == 0) {
            startProcessThread ();
        }
        item->setOwner (UploadItem::OWNER_PROCESS_THREAD);
        item->markPending (UploadItem::PENDING_PROCESSING);
        Q_EMIT (startProcess (item));
    } else {
        if (item->getOwner () == UploadItem::OWNER_UPLOAD_THREAD) {
            // This can only happen if the previous upload attempt for the item
            // was asked to be stopped, but plugin has not stopped - perhaps
            // because it is stuck somewhere. In this case, we should
            // explicitly kill the plugin
            Q_ASSERT (uploadProcess.isProcessStopping() == true);
            processStopped (item);
        } else {
            // Should never try to upload an item which is still being
            // processed
            Q_ASSERT (item->getOwner() == UploadItem::OWNER_QUEUE);
        }

        DBGSTREAM << "Now checking for connection";
        // If we don't have connection ask for it
        if (connection.isConnected() == false) {
            item->markPending (UploadItem::PENDING_CONNECTIVITY);    
            setState (OFFLINE);
            return;
        } else {
            DBGSTREAM << "Can finally send item";
            setState (SENDING);
        }
        
        item->setOwner (UploadItem::OWNER_UPLOAD_THREAD);
        Q_EMIT (startUpload (item));
    }
}

void UploadEngine::queueChangeTop (UploadItem * down, UploadItem * top) {
    Q_UNUSED (down);

    DBGSTREAM << "New top in queue (old one moved down)";    

    if (top->getOwner () != UploadItem::OWNER_QUEUE) {
        DBGSTREAM << "New top is already being handled by one of the queues";
    } else {
        if (!top->isProcessed()) {
            
            // ProcessThread should be actively processing something at this
            // point because otherwise this item would have been processed 
            Q_ASSERT (processThread != 0);

            // Stop processing of whatever is being processed currently.
            // We can come back to that later. We should not hold up the upload
            // queue right now
            Q_EMIT (stopProcess (0));
        }

        queueTop (top);
    }
}

void UploadEngine::startProcessThread() {

    if (getState() == SHUTTING_DOWN) {
        DBGSTREAM << "Shutting down - should not start thread here";
        return;
    }

    if (processThread == 0) {
        processThread = new ProcessThread (this);
        DBGSTREAM << "ProcessThread initialized";
    }

    if (processThread->isRunning()) {
        DBGSTREAM << "Process Thread already running";
    } else {
        DBGSTREAM << "Connecting signals and starting the process thread";
        // Connect signals
        connect (this, SIGNAL (startProcess(UploadItem*)), processThread,
            SIGNAL (startProcess(UploadItem*)));
        connect (this, SIGNAL (stopProcess(UploadItem*)), processThread,
            SIGNAL (stopProcess(UploadItem*)));
                
        connect (processThread, SIGNAL(processDone(UploadItem*)), this,
            SLOT(processDone(UploadItem*)));
        connect (processThread, SIGNAL(processStopped(UploadItem*)), this,
            SLOT(processStopped(UploadItem*)));    
        connect (processThread, SIGNAL (processFailed(UploadItem*,int)), this,
            SLOT (processThreadFailed(UploadItem*,int)));
        connect (processThread, SIGNAL(finished()), this,
                SLOT(processThreadFinished()));    
    
        processThread->start();
    }
}

void UploadEngine::stopProcessThread () {
    if (processThread && processThread->isRunning ()) {
        DBGSTREAM << "Ask process thread to stop";
        processThread->stop ();
    } else {
        processThreadFinished ();
    }
}

void UploadEngine::processThreadFinished () {
    if (processThread != 0) {
        DBGSTREAM << "Process Thread finished";
        this->disconnect (processThread);
        delete processThread;
        processThread = 0;
    }
        
    if (getState() == SHUTTING_DOWN) {
        DBGSTREAM << "Shutdown upload engine.";
        quit();
    }
}

void UploadEngine::processDone (UploadItem * item) {

    DBGSTREAM << "Process done signal recieved";
    item->setOwner (UploadItem::OWNER_QUEUE);
    UploadItem *next = queue.getNextItem (item);

    if (next) {
        next->setOwner (UploadItem::OWNER_PROCESS_THREAD);
        Q_EMIT (startProcess (next));
    } else {
        DBGSTREAM << "No more items to process. Stopping thread";
        // No more items to process
        stopProcessThread ();
    }

    if (item->isCancelled()) {
        tuiClient->removeTransfer (item->getTransferId ());
        // We could alternately wait for the item to become top of the queue
        // and then get cancelled, but that does not make sense from UI
        // perspective
        Q_EMIT (removeUpload (item));
        return;
    }

    if (item == queue.getTop ()) {
        DBGSTREAM << "Item was at the top of the queue";
        queueTop (item);
    }
}

void UploadEngine::processStopped (UploadItem *item) {
    DBGSTREAM << "Process stopped signal recieved";
    if (item) {
        item->setOwner (UploadItem::OWNER_QUEUE);

        if (item->isCancelled ()) {
            tuiClient->removeTransfer (item->getTransferId ());
            Q_EMIT (removeUpload (item));
        } else if (getState () == MASS_STORAGE) {
            item->markPending (UploadItem::PENDING_MSM);
        }
    }

    return;
}

void UploadEngine::processThreadFailed (UploadItem *item,
    int processErrorCode) {
    
    Q_ASSERT (item != 0);
    item->setOwner (UploadItem::OWNER_QUEUE);

    if (getState () == MASS_STORAGE) {
        DBGSTREAM << "Mass storage is enabled. Error might have been"
            "because of that";
        item->markPending (UploadItem::PENDING_MSM);
        return;
    }

    UploadItem::ProcessError processError =
        (UploadItem::ProcessError)processErrorCode;
    
    DBGSTREAM << "Handle process failed reponse" << processError;

    WebUpload::Error itemError;
    if (processError == UploadItem::PROCESS_ERROR_FILE_NOT_FOUND) {
        itemError = WebUpload::Error::missingFiles();
    } else {
        itemError = WebUpload::Error::transferFailed();
    }

    WebUpload::Entry * entry = item->getEntry();
    unsigned int notSent = entry->mediaCount() - entry->mediaSentCount();
    itemError.setFailedCount (notSent);

    if (!item->markFailed (itemError)) {
        item->setCancelled ();
    } 

    if (item->isCancelled()) {
        tuiClient->removeTransfer (item->getTransferId ());
        Q_EMIT (removeUpload (item));
    }

}

void UploadEngine::uploadDone (UploadItem * item) {
    DBGSTREAM << "Upload done signal from thread";
    item->markDone(); 
    item->setOwner (UploadItem::OWNER_QUEUE);
    tuiClient->removeTransfer (item->getTransferId ());
    Q_EMIT (removeUpload (item));
}

void UploadEngine::uploadStopped (UploadItem * item) {
    DBGSTREAM << "Upload stopped signal from thread";
    
    if (item == 0) {
        WARNSTREAM << "Upload stopped called without item";
        return;
    }

    if (m_stoppingItems.contains(item)) {
        m_stoppingItems.removeAll(item);
        item->setCancelled();
    }
    
    item->setOwner (UploadItem::OWNER_QUEUE);
    if (item->isCancelled ()) {
        tuiClient->removeTransfer (item->getTransferId ());
        Q_EMIT (removeUpload (item));
    } else if (getState () == MASS_STORAGE) {
        item->markPending (UploadItem::PENDING_MSM);
    } else if (getState () == OFFLINE) {
        item->markPending (UploadItem::PENDING_CONNECTIVITY);
    } else {
        WARNSTREAM << "Stop called after stop";
    }
}

void UploadEngine::uploadFailed (UploadItem * item, WebUpload::Error error) {
    DBGSTREAM << "Upload failed signal" << error.code();

    Q_ASSERT (item != 0);

    if (m_stoppingItems.contains(item)) {
        DBGSTREAM << "Fail: Marked to be cancelled";
        m_stoppingItems.removeAll(item);
        item->setCancelled();
    }

    if (item->isCancelled() == false) {
        item->setOwner (UploadItem::OWNER_QUEUE);

        if (getState () == MASS_STORAGE) {
            DBGSTREAM << "Fail: Mass storage is enabled.";
            item->markPending (UploadItem::PENDING_MSM);
        } else if (error.code() == WebUpload::Error::CODE_NO_CONNECTION || 
            getState () == OFFLINE) {
            
            DBGSTREAM << "Fail: Connection lost";

            setState (OFFLINE);
            item->markPending (UploadItem::PENDING_CONNECTIVITY);
        } else {
        
            DBGSTREAM << "Fail: Code:" << error.code();
        
            // Try to mark failed (cancel if failing)
            if (item->markFailed (error) == false) {
                WARNSTREAM << "Fail: No TUI: Cancel";
                item->setCancelled ();
            }
        }
    }

    // Handling two cases here:
    // (1) Cancel was requested by the user. At that time, the item was being
    // processed. But by the time the cancel request reaches UploadHandler, the
    // item has returned an error, and the thread no longer owns it. So, the
    // uploadFailed in the engine should check the cancelled flag as well.
    // (2) UploadEngine is not connected with the Transfer UI. In this case,
    // rather than letting a failed item stay in the queue, we should remove it
    if (item->isCancelled()) {
        DBGSTREAM << "Fail: Removing from queue";
        tuiClient->removeTransfer (item->getTransferId ());
        Q_EMIT (removeUpload (item));
    }
}

void UploadEngine::repairError () {

    UploadItem * item = qobject_cast<UploadItem *>(QObject::sender());
    
    if (item == 0) {
        WARNSTREAM << "Repair: Slot repairError called wrongly. Sender:"
            << QObject::sender();
        return;
    }

    if (item->getOwner () == UploadItem::OWNER_UPLOAD_THREAD) {
        DBGSTREAM << "Repair: Ignoring repair (already uploading)";
        return;
    }

    if (item->isProcessed ()) {
        DBGSTREAM << "Repair: Item is processed";
        item->setOwner (UploadItem::OWNER_QUEUE);
        if (item == queue.getTop ()) {
            DBGSTREAM << "Repair: Item is the top item";
            queueTop (item);
        } else {
            item->markPending (UploadItem::PENDING_QUEUED);
        }
    } else {
        if (processThread) {

            if (item->getOwner () == UploadItem::OWNER_PROCESS_THREAD) {
                DBGSTREAM << "Repair: Item is already being processed";
                return;
            }

            // Stop whatever is being processed right now, so we can get back
            // to processing this item
            Q_EMIT (stopProcess (0));
        } else {
            // Process thread is not running. Start it
            startProcessThread ();
        }

        item->setOwner (UploadItem::OWNER_PROCESS_THREAD);
        if (item == queue.getTop ()) {
            item->markPending (UploadItem::PENDING_PROCESSING);
        } else {
            item->markPending (UploadItem::PENDING_QUEUED);
        }
        Q_EMIT (startProcess (item));
    }
}

void UploadEngine::cancelItem () {

    UploadItem * item = qobject_cast<UploadItem *>(QObject::sender());
    
    if (item == 0) {
        WARNSTREAM << "Slot cancelItem called wrongly. QObject::sender() = "
            << QObject::sender();
        return;
    }

    if (item->isCancelled ()) {
        WARNSTREAM << "Cancel called on already cancelled item " << 
            item->toString();
        return;
    }
    
    DBGSTREAM << "Cancel item" << item->toString() << ", owner"
        << item->getOwner();
    
    // TODO: What to do first if item is processed
    switch (item->getOwner()) {
        case UploadItem::OWNER_QUEUE:
        {
            item->setCancelled();
            tuiClient->removeTransfer (item->getTransferId ());
            Q_EMIT (removeUpload (item));
            break;
        }

        case UploadItem::OWNER_UPLOAD_THREAD:
        {
            DBGSTREAM << "Cancel with item that is being uploaded";
            m_stoppingItems.append(item);
            Q_EMIT (stopUpload (item));
            break;
        }

        case UploadItem::OWNER_PROCESS_THREAD:
        {
            Q_ASSERT (processThread != 0);
            item->setCancelled();
            DBGSTREAM << "Cancel with item that is being processed";
            break;
        }
        
        default:
            CRITSTREAM << "Cancel with invalid owner";
            break;
    }
}

UploadEngine::State UploadEngine::getState () const {
    return state;
}

void UploadEngine::setState (State newState) {
    if (state != newState) {
        DBGSTREAM << "Engine state" << newState;
        state = newState;
        Q_EMIT (stateChanged (state));
    }
}

void UploadEngine::connected () {
    if (getState () != SENDING) {
        setState (IDLE);
        DBGSTREAM << "Continue uploading after offline mode";
        queueTop (queue.getTop());
    } else {
        DBGSTREAM << "Engine ignoring connected signal";
    }
}

void UploadEngine::disconnected () {
    if (getState() == SENDING) {
        DBGSTREAM << "Entering to offline mode. Stop sending.";
        setState (OFFLINE);
        Q_EMIT (stopUpload(0));
    } else {
        DBGSTREAM << "Engine ignoring disconnected signal";    
    }
}


void UploadEngine::usbModeChanged (MeeGo::QmUSBMode::Mode mode) {
    currentUSBMode = mode;

    switch (mode) {
        case MeeGo::QmUSBMode::Connected :
            DBGSTREAM << "USB MODE --> Connected";
            break;
        case MeeGo::QmUSBMode::DataInUse :
            DBGSTREAM << "USB MODE --> DataInUse";
            break;
        case MeeGo::QmUSBMode::Disconnected :
            DBGSTREAM << "USB MODE --> Disconnected";
            break;
        case MeeGo::QmUSBMode::MassStorage :
            DBGSTREAM << "USB MODE --> MassStorage";
            break;
        case MeeGo::QmUSBMode::ChargingOnly :
            DBGSTREAM << "USB MODE --> ChargingOnly";
            break;
        case MeeGo::QmUSBMode::OviSuite :
            DBGSTREAM << "USB MODE --> OviSuite";
            break;
        case MeeGo::QmUSBMode::ModeRequest :
            DBGSTREAM << "USB MODE --> ModeRequest";
            break;
        case MeeGo::QmUSBMode::Ask :
            DBGSTREAM << "USB MODE --> Ask";
            break;
        case MeeGo::QmUSBMode::Undefined :
            DBGSTREAM << "USB MODE --> Undefined";
            break;
    }
    
    // Handle only the MassStorage and Disconnected modes. Can ignore all the 
    // other mode changes
    if (mode == MeeGo::QmUSBMode::MassStorage) {
        if (state == IDLE || state == MASS_STORAGE) {
            UploadItem * top = queue.getTop ();
            if (top != 0) {
                top->markPending (UploadItem::PENDING_MSM);
            }
        }

        if (state == MASS_STORAGE || state == OFFLINE || 
            state == SHUTTING_DOWN) {
            return;
        }

        setState (MASS_STORAGE);
        if (processThread) {
            processThread->stop ();
        }

        UploadItem *nowSending = uploadProcess.currentlySendingMedia ();
        if (nowSending != 0) {
            Q_EMIT (stopUpload(nowSending));
        } 
    } else if (mode == MeeGo::QmUSBMode::Disconnected) {
        if (getState () != MASS_STORAGE) {
            return;
        }
        
        DBGSTREAM << "Entering USB disconnected mode";
        setState (IDLE);

        // First continue processing from where it was stopped
        UploadItem *item = 0; 
        for (item = queue.getTop (); item != 0 && item->isProcessed (); 
            item = queue.getNextItem (item));
        if (item != 0) {
            if (processThread == 0) {
                startProcessThread ();
            }
            item->setOwner (UploadItem::OWNER_PROCESS_THREAD);
            if (item == queue.getTop ()) {
                item->markPending (UploadItem::PENDING_PROCESSING);
            } else {
                item->markPending (UploadItem::PENDING_QUEUED);
            }
            Q_EMIT (startProcess (item));
        }

        // Now handle the queue top element as it should be handled
        queueTop (queue.getTop ());
    }
}


void UploadEngine::fileSystemWillUnmount (MeeGo::QmUSBMode::MountPath path) {

    Q_UNUSED (path)

    setState (MASS_STORAGE);
    UploadItem *nowSending = uploadProcess.currentlySendingMedia ();
    if (nowSending != 0) {
        Q_EMIT (stopUpload(nowSending));
    }
}


