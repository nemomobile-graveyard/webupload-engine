 
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

#ifndef _UPLOAD_QUEUE_H_
#define _UPLOAD_QUEUE_H_

#include <QObject>
#include <QQueue>
#include <QStringList>

class UploadItem;

/*!
    \class UploadQueue
    \brief Upload queue that owns upload items
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
*/
class UploadQueue : public QObject {

    Q_OBJECT

public:
    UploadQueue (QObject * parent = 0);
    ~UploadQueue();
    
    /*!
      \brief Add xml path to list of paths of xml files corresponding to upload
             requests
      \param xmlPath Path of the xml file being used to init an upload
     */
    void initing (const QString & xmlPath);

    /*!
      \brief Initializing the upload request with the xml file failed. Remove
             it from the queue.
      \param xmlPath Path of the xml file corresponding to the transfer whose
             initialization failed
     */
    void initFailed (const QString & xmlPath);

    /*!
      \brief Push new task to end of queue
      \param item Item pushed to queue. Ownership item will move to queue.
      \return true if item was successfully added
     */
    bool push (UploadItem * item);
    
    /*!
      \brief How many items there is in queue
      \return Number of items in queue
     */
    int size() const;
    
    /*!
      \brief Give current top item
      \return Pointer to current top item
     */
    UploadItem * getTop() const;
    
    /*!
      \brief  Get the item in the queue that is after this one
      \param  item Item whose next is required
      \return Pointer to the next item in the queue or NULL if current item
              is the last item
     */
    UploadItem * getNextItem ( UploadItem * item) const;
    
    /*!
      \brief Will check if queue already has item with given entry path
      \param entryPath Entry path
      \return Pointer to item if found. Null if not found.
     */
    UploadItem * itemForEntryPath (const QString & entryPath);

    /*!
      \brief Will check if the the queue already has the given xml in its init
             list
      \param entryPath Entry path
      \return true if the queue has that path in its init'ing list, else false
     */
    bool isEntryInit (const QString & entryPath) const;
    
Q_SIGNALS:

    /*!
      \brief Signal emitted when there is new item at top
      \param item Item at top
     */
    void topItem (UploadItem * item);
    
    /*!
      \brief Called when previous top item has moved down in queue and
             replaced by new. When this is called then topItem signal
             isn't emitted.
      \param down Old top item
      \param top New top item
     */
    void replacedTopItem (UploadItem * down, UploadItem * top);
    
    //! \brief All tasks in queue done (queue is empty)
    void done ();
    
public Q_SLOTS:

    /*!
      \brief Remove item from queue (used when item is done or cancelled).
             It also destroys the item
      \param item Item removed from queue
     */
    void removeItem (UploadItem * item);
            
    /*!
      \brief Ask item to be moved to top of queue
      \param item Item rised to top of queue
     */
    void riseItem (UploadItem * item);
    
private:

    QQueue <UploadItem *> items; //!< Items owned by queue and in queue
    //! Xml file path of items being initialized. Once these items are
    // initialized, they will be added to the list, and the corresponding xml 
    // file paths will be removed from the list
    QStringList m_initItems; 

};

#endif // #ifndef _UPLOAD_QUEUE_H_
