 
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

#ifndef _PROCESS_THREAD_H_
#define _PROCESS_THREAD_H_

#include <QThread>
#include "processhandler.h"
#include "uploaditem.h"

/*!
   \class  ProcessThread
   \brief  This class process the upload request - creating the resized copy
           files with only that metadata information left in the copy which the
           user wants to share. Only after this step can an upload actually
           happen.

   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class ProcessThread : public QThread
{
    Q_OBJECT
public:
    /*!
       \brief  Constructor
       \param  parent : Pointer to the QObject parent of this class (in this
               case, it will be a pointer to the UploadEngine instance)
     */
    ProcessThread (QObject *parent = 0);

    /*! \brief  Destructor */
    virtual ~ProcessThread ();
    
    /*! \brief  Stop the thread */
    void stop (); 
    
Q_SIGNALS:

    /*!
       \brief  Signal emitted to tell the ProcessHandler to start processing an
               update item. This signal is emitted by being connected to a
               signal of the parent of this class.
       \param item Upload item to be processed
     */
    void startProcess (UploadItem *item);
    
    /*!
      \brief Signal emitted when an item has been processed successfully
      \param item Item processed successfully
     */
    void processDone (UploadItem *item);

    /*!
      \brief  Signal emitted to tell the ProcessHandler to stop processing an
              upload item
      \param item Upload item which should be stopped. 
     */
    void stopProcess (UploadItem *item);

    /*!
      \brief Slot emitted when a process is prematurely stopped
      \param item Item whose process that was stopped
     */
    void processStopped (UploadItem *item);

   /*!
      \brief Signal emitted when process has failed
      \param item Item whose process failed
      \param errorCode See UploadItem::ProcessError
     */
    void processFailed (UploadItem * item, int errorCode);

private Q_SLOTS:

    /*!
      \brief Slot connecting to the signal emitted by the handler when an
             process is stopped prematurely. The handler is also stopped when
             this slot is invoked
      \param item Item whose process that was stopped
     */
    void handlerStopped (UploadItem *item);

private:

    ProcessHandler *handler;
    bool stopThread;
};

#endif // _PROCESS_THREAD_H_