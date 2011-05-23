 
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

#include "uploadqueue.h"
#include "uploaditem.h"
#include <QDebug>
#include "WebUpload/entry.h"

#define DBGPREFIX "Queue:"
#define DBGSTREAM qDebug() << DBGPREFIX
#define WARNSTREAM qWarning() << DBGPREFIX
#define CRITSTREAM qCritical() << DBGPREFIX

UploadQueue::UploadQueue (QObject * parent) : QObject (parent) {

}

UploadQueue::~UploadQueue() {
    DBGSTREAM << "destroyed with size" << size();
}

void UploadQueue::initing (const QString & xmlPath) {
    if (!m_initItems.contains (xmlPath)) {
        m_initItems << xmlPath;
    }
}

void UploadQueue::initFailed (const QString & xmlPath) {
    m_initItems.removeOne (xmlPath);
}

bool UploadQueue::push (UploadItem * item) {
    if (item == 0) {
        WARNSTREAM << "Can't push null item";
        return false;
    }

    // Remove the corresponding xml file path from m_initItems list
    WebUpload::Entry * entry = item->getEntry ();
    if (entry == 0) {
        WARNSTREAM << "Null entry in item";
        return false;
    }
    m_initItems.removeOne (entry->serializedTo ());
    
    // Mark queue as parent and push to queue model
    item->setParent (this);
    items.push_back (item);
    
    DBGSTREAM << "New item" << item->toString() << ", size =" << size();

    // Emit signal if queue was empty
    if (size() == 1) {
        DBGSTREAM << "Received first item";
        Q_EMIT (topItem (item));
    }
    
    return true;
}

int UploadQueue::size() const {
    return items.size();
}

void UploadQueue::removeItem (UploadItem * item) {
    if (!item) {
        WARNSTREAM << "Item is a NULL pointer";
        return;
    }
       
    if (!items.contains(item)) {
        WARNSTREAM << "Can't remove item that is not found in queue";
        return;
    }

    if (item->getOwner() != UploadItem::OWNER_QUEUE) {
        WARNSTREAM << "Can't remove item with wrong owner" << item->getOwner();
        return;
    }
    
    UploadItem * currentTop = items.first();
    
    items.removeAll (item);
    
    // TODO: Signal something before deleting item? some clear function in item?
    delete item;
    
    if (size() == 0) {
        Q_EMIT (done ());
    } else if (currentTop == item) {
        Q_EMIT (topItem (items.first())); 
    }
}

void UploadQueue::riseItem (UploadItem * item) {
    if (!items.contains (item)) {
        WARNSTREAM << "Can't rise item that is not found in queue";
        return;
    }
    
    UploadItem * currentTop = items.first();
    if (currentTop == item) {
        WARNSTREAM << "Item already at top of the queue";
        return;
    }
    
    items.removeAll (item);
    items.push_front (item);
    
    Q_EMIT (replacedTopItem (currentTop, item));
}

UploadItem * UploadQueue::getTop() const {
    if (size() > 0) {
        return items.first();    
    } else {
        DBGSTREAM << "Nothing at top";
        return 0;
    }
}

UploadItem * UploadQueue::getNextItem (UploadItem * item) const {
    UploadItem * next = 0;

    int currIdx = items.indexOf (item);

    if ((currIdx >= 0) && ((currIdx + 1) != items.size())) {
        next = items.at (currIdx + 1);
    }

    return next;
}

UploadItem * UploadQueue::itemForEntryPath (const QString & entryPath) {
    UploadItem * ret = 0;

    QListIterator<UploadItem *> iter (items);
    while (iter.hasNext()) {
        UploadItem * item = iter.next();
        if (item->getEntry()->serializedTo() == entryPath) {
            ret = item;
            break;
        }
    }
    
    return ret;
}

bool UploadQueue::isEntryInit (const QString & entryPath) const {
    return m_initItems.contains (entryPath);
}
