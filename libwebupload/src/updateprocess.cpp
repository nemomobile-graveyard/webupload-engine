
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
 
#include "WebUpload/UpdateProcess"
#include "WebUpload/System"
#include "updateprocessprivate.h"
#include <QDebug>

using namespace WebUpload;
 
UpdateProcess::UpdateProcess (QObject * parent) : 
    WebUpload::PluginProcess (parent), d_ptr(new UpdateProcessPrivate(this))
{

    // This will register needed meta types (have to be done in private)
    WebUpload::System::registerMetaTypes ();    

    WebUpload::System::loadLocales ();

    connect (&m_pdata, SIGNAL (doneSignal()), d_ptr, SLOT (doneSlot()),
        Qt::QueuedConnection);
    connect (&m_pdata, SIGNAL (stoppedSignal()), d_ptr, SLOT (stoppedSlot()),
        Qt::QueuedConnection);
    connect (&m_pdata,
        SIGNAL (updateFailedSignal(WebUpload::Error::Code,QStringList)), d_ptr,
        SLOT (failedSlot(WebUpload::Error::Code,QStringList)),
        Qt::QueuedConnection);
    connect (&m_pdata,
        SIGNAL (updateWarningSignal(WebUpload::Error::Code,QStringList)), d_ptr,
        SLOT (warningSlot(WebUpload::Error::Code,QStringList)),
        Qt::QueuedConnection);

}

UpdateProcess::~UpdateProcess () {
    m_pdata.disconnect (this);

    delete d_ptr;
    d_ptr = 0;
}

void UpdateProcess::startUpdate (WebUpload::Account * account,
    WebUpload::ServiceOption * option) {
    
    if (d_ptr->m_connection.isConnected () == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_NO_CONNECTION, QStringList()));
        return;
    }

    if (startProcess (account) == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_CUSTOM, QStringList()));
        return;
    }

    d_ptr->m_account = account;
    d_ptr->m_option = option;
    d_ptr->m_value.clear();

}

void UpdateProcess::startUpdateAll (WebUpload::Account * account) {

    if (d_ptr->m_connection.isConnected () == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_NO_CONNECTION, QStringList()));
        return;
    }

    if (startProcess (account) == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_CUSTOM, QStringList()));
        return;
    }

    d_ptr->m_account = account;
    d_ptr->m_option = 0;
    d_ptr->m_value.clear();
}

void UpdateProcess::startAddValue (WebUpload::Account * account,
    WebUpload::ServiceOption * option, const QString & value) {

    if (d_ptr->m_connection.isConnected () == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_NO_CONNECTION, QStringList()));
        return;
    }

    if (startProcess (account) == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_CUSTOM, QStringList()));
        return;
    }

    d_ptr->m_account = account;
    d_ptr->m_option = option;
    d_ptr->m_value = value;
}

void UpdateProcess::cancel () {
    if (isActive()) {
        send (m_pdata.stop ());
    }
}

void UpdateProcess::processStarted () {

    m_pdata.clear ();
    QString accountId = d_ptr->m_account->stringId();

    if (d_ptr->m_option == 0) {
        qDebug() << "Calling updateAll";
        send (m_pdata.updateAll (accountId));
    } else if (d_ptr->m_value.isEmpty() == true) {
        qDebug() << "Calling update";
        send (m_pdata.update (accountId, d_ptr->m_option->id()));
    } else {
        qDebug() << "Calling addValue";
        send (m_pdata.addValue (accountId, d_ptr->m_option->id(), d_ptr->m_value));
    }
}

// -- private class functions -----------------------------------------------

UpdateProcessPrivate::UpdateProcessPrivate(UpdateProcess *publicObject) :
    QObject(publicObject), q_ptr(publicObject), m_account(0), m_option(0),
    m_connection (this, false)
{
    connect (&m_connection, SIGNAL (disconnected()), this, SLOT
        (disconnected()));
}

UpdateProcessPrivate::~UpdateProcessPrivate()
{
    disconnect (&m_connection, SIGNAL (disconnected()), this, SLOT
        (disconnected()));
}

void UpdateProcessPrivate::doneSlot () {
//    if (m_currentProcess == 0) {
//        return;
//    }

    if (m_option != 0) {
        m_option->refresh();
    } else {
        qDebug() << __FUNCTION__ << "Refreshing all options";
        m_account->service()->refreshPostOptions();
    }
    
//    m_currentProcess = 0;
    Q_EMIT (q_ptr->finished());
}

void UpdateProcessPrivate::failedSlot (const WebUpload::Error::Code errorId,
    const QStringList failedIds) {
    
//    if (m_currentProcess == 0) {
//        return;
//    }    
    
    qDebug() << __FUNCTION__ << errorId << ": Refreshing all options";
    m_account->service()->refreshPostOptions();
    
//    m_currentProcess = 0;
    Q_EMIT (q_ptr->failed (errorId, failedIds));
}

void UpdateProcessPrivate::warningSlot (const WebUpload::Error::Code warningId,
    const QStringList failedIds) {
    
//    if (m_currentProcess == 0) {
//        return;
//    }    
    
    //qDebug() << __FUNCTION__ << warningId << ": Refreshing all options";
    //m_account->service()->refreshPostOptions();
    
    Q_EMIT (q_ptr->warning (warningId, failedIds));
}

void UpdateProcessPrivate::stoppedSlot () {
//    if (m_currentProcess == 0) {
//        return;
//    }
// 
//    m_currentProcess = 0;
    if (m_isConnected == false) {
        failedSlot (WebUpload::Error::CODE_NO_CONNECTION, QStringList());
    } else {
        Q_EMIT (q_ptr->canceled());
    }
}

void UpdateProcessPrivate::pluginProcessCrashed () {

    qDebug() << "Plugin process crashed";

    //TODO: Proper error code
    failedSlot (WebUpload::Error::CODE_CUSTOM, QStringList());
}

void UpdateProcessPrivate::disconnected () {
    m_isConnected = false;
    q_ptr->cancel();
}