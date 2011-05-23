 
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

#include <QDebug>
#include "processthread.h"

ProcessThread::ProcessThread (QObject *parent) : QThread (parent), handler (0),
    stopThread (false) {

    // Need to construct the handler object in the constructor and make all the
    // connections here. Otherwise the connections do not happen in time and
    // webprocess-engine hangs since the startProcess signal emitted by the
    // ProcessEngine goes nowhere.
    qDebug() << "Create new ProcessHandler and do all the connections";
    handler = new ProcessHandler ();
    handler->moveToThread (this);
    
    connect (this, SIGNAL (startProcess(UploadItem*)), handler, 
        SLOT (startProcess(UploadItem*)), Qt::QueuedConnection);
    connect (this, SIGNAL (stopProcess(UploadItem*)), handler,
        SLOT (stopProcess(UploadItem*)), Qt::QueuedConnection);
    connect (handler, SIGNAL (processDone(UploadItem*)), this,
        SIGNAL (processDone(UploadItem*)), Qt::QueuedConnection);
    connect (handler, SIGNAL (processStopped(UploadItem*)), this,
        SLOT (handlerStopped(UploadItem*)), Qt::QueuedConnection);
    connect (handler, SIGNAL (processFailed(UploadItem*,int)), this,
        SIGNAL (processFailed(UploadItem*,int)), Qt::QueuedConnection);

}

ProcessThread::~ProcessThread () {
    if (handler) {
        delete handler;
        handler = 0;
    }
}

void ProcessThread::stop () {
    qDebug () << "Asked to stop ProcessThread";
    stopThread = true;

    Q_EMIT (stopProcess (NULL));
}

void ProcessThread::handlerStopped (UploadItem *item) {
    if (stopThread) {
        qDebug() << "Stopping process thread";
        quit ();
    } else {
        Q_EMIT (processStopped (item));
    }
}
