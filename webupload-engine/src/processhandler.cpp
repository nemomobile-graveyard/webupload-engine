 
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

#include <QDebug>
#include "processhandler.h"
#include <QFile>

ProcessHandler::ProcessHandler (QObject *parent) : QObject (parent), 
    m_myItem (0), m_media (0) {
    
    qRegisterMetaType<WebUpload::Media::CopyResult> ();    

    QObject::connect (this,
        SIGNAL (mediaProcessed(WebUpload::Media::CopyResult)), this, 
        SLOT (processNextMedia(WebUpload::Media::CopyResult)),
        Qt::QueuedConnection);
}

ProcessHandler::~ProcessHandler () {
    if (m_myItem) 
        stopProcess (0);
}

void ProcessHandler::startProcess (UploadItem * item) {
    Q_ASSERT (item != 0);

    qDebug() << "Start processing item : " << item;

    if (m_myItem != 0) {
        // Some other item is already being processed
        return;
    }

    if (item->isProcessed ()) {
        Q_EMIT (processDone (item));
    } else {
        m_myItem = item;
        processNextMedia ();
    }
}

void ProcessHandler::processNextMedia (
    WebUpload::Media::CopyResult prevCopyResult) {
    
    Q_ASSERT (m_myItem != 0);
    
    // Storage space error
    if (prevCopyResult == WebUpload::Media::COPY_RESULT_NO_SPACE) {
        qWarning() << "Previous media process failed for space";
    
        Q_EMIT (processFailed (m_myItem,
            (int)UploadItem::PROCESS_ERROR_STORAGE_MEMORY_FULL));
      
    // All other errors
    } else if (prevCopyResult != WebUpload::Media::COPY_RESULT_SUCCESS &&
        prevCopyResult != WebUpload::Media::COPY_RESULT_NOTHING_TO_COPY) {
        
        qWarning() << "Previous media process failed with error code"
            << prevCopyResult;
            
        Q_EMIT (processFailed (m_myItem, 
            (int)UploadItem::PROCESS_ERROR_UNDEFINED));
    
    // No error   
    } else {
        qDebug() << "Copy made - processing next media";
        m_media = m_myItem->getNextUnprocessedMedia ();

        WebUpload::Media::CopyResult res = 
            WebUpload::Media::COPY_RESULT_SUCCESS;

        if (m_media) {
            QString originalFilePath = m_media->srcFilePath ();
            if (!QFile::exists (originalFilePath)) {
                if (!m_media->setFailed ()) {
                    Q_EMIT (processFailed (m_myItem, 
                        (int)UploadItem::PROCESS_ERROR_FILE_NOT_FOUND));
                    return;
                }
                // Otherwise letting res stay success - the error would anyways
                // be reported once upload processing starts
            }  else {
                res = m_media->makeCopy ();
            }

            Q_EMIT (mediaProcessed (res));
        } else {
            qDebug() << "All media processed";
            Q_EMIT (processDone (m_myItem));
            m_myItem = 0;
        }
    }
}

void ProcessHandler::stopProcess (UploadItem *item) {
    qDebug() << "Asked to stop process";
    if (((item != 0) && (item == m_myItem)) || (item == 0)) {
        m_myItem = 0;
        qDebug() << "emitting processStopped signal";
        Q_EMIT (processStopped (item));
    } 
}
