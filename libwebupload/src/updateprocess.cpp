
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
 
#include "WebUpload/UpdateProcess"
#include "WebUpload/System"
#include "updateprocessprivate.h"
#include <QDebug>

#include "connectionmanager.h"

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
#ifdef WARNINGS_ENABLED
    connect (&m_pdata,
        SIGNAL (updateWarningSignal(WebUpload::Error::Code,QStringList)), d_ptr,
        SLOT (warningSlot(WebUpload::Error::Code,QStringList)),
        Qt::QueuedConnection);
#endif

}

UpdateProcess::~UpdateProcess () {
    m_pdata.disconnect (this);

    delete d_ptr;
    d_ptr = 0;
}

bool UpdateProcess::cancelCalled () const {
    return d_ptr->m_cancelCalled;
}

void UpdateProcess::startUpdate (WebUpload::Account * account,
    WebUpload::ServiceOption * option) {
    
    if (startProcess (account) == false) {
        Q_EMIT (failed (WebUpload::Error::CODE_CUSTOM, QStringList()));
        return;
    }

    d_ptr->m_account = account;
    d_ptr->m_option = option;
    d_ptr->m_value.clear();

}

void UpdateProcess::startUpdateAll (WebUpload::Account * account) {

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
        d_ptr->m_cancelCalled = true;
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
    m_cancelCalled (false) {

}

UpdateProcessPrivate::~UpdateProcessPrivate() {
}

void UpdateProcessPrivate::doneSlot () {
    if (m_option != 0) {
        m_option->refresh();
    } else {
        qDebug() << __FUNCTION__ << "Refreshing all options";
        m_account->service()->refreshPostOptions();
    }
    
    Q_EMIT (q_ptr->finished());
}

void UpdateProcessPrivate::failedSlot (const WebUpload::Error::Code errorId,
    const QStringList failedIds) {
    
    qDebug() << __FUNCTION__ << errorId << ": Refreshing all options";
    m_account->service()->refreshPostOptions();

    ConnectionManager connection (this, false);
    if (connection.isConnected () == false) {
        Q_EMIT(q_ptr->failed(WebUpload::Error::CODE_NO_CONNECTION, failedIds));
    } else {
        Q_EMIT (q_ptr->failed (errorId, failedIds));
    }
}

#ifdef WARNINGS_ENABLED
void UpdateProcessPrivate::warningSlot (const WebUpload::Error::Code warningId,
    const QStringList failedIds) {

    Q_EMIT (q_ptr->warning (warningId, failedIds));
}
#endif

void UpdateProcessPrivate::stoppedSlot () {
    m_cancelCalled = false;
    Q_EMIT (q_ptr->canceled());
}

void UpdateProcessPrivate::pluginProcessCrashed () {

    qDebug() << "Plugin process crashed";

    //TODO: Proper error code
    failedSlot (WebUpload::Error::CODE_CUSTOM, QStringList());
}

