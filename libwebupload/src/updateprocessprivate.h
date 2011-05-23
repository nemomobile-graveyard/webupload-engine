
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
 
#ifndef _WEBUPLOAD_UPDATE_PROCESS_PRIVATE_H_
#define _WEBUPLOAD_UPDATE_PROCESS_PRIVATE_H_

#include "WebUpload/Error"
#include <QObject>
#include <QStringList>

namespace WebUpload {

class Account;
class ServiceOption;
class UpdateProcess;

/*!
   \class  UpdateProcessPrivate
   \brief  Private implementation class for UpdateProcess
   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class UpdateProcessPrivate : public QObject {
    Q_OBJECT

public:
    UpdateProcessPrivate (UpdateProcess *publicObject);
    virtual ~UpdateProcessPrivate();

public Q_SLOTS:

    /*!
      \brief Slot for ProcessExchangeData::doneSignal
     */
    void doneSlot ();

    /*!
      \brief Slot for ProcessExchangeData::stoppedSignal
     */
    void stoppedSlot ();
    
    /*!
      \brief Slot for ProcessExchangeData::updateFailedSignal
     */
    void failedSlot (WebUpload::Error::Code errorId, QStringList failedIds);

#ifdef WARNINGS_ENABLED
    /*!
      \brief Slot for ProcessExchangeData::updateWarningSignal
     */
    void warningSlot (WebUpload::Error::Code warningId, QStringList failedIds);
#endif
        
    /*!
      \brief Slot for PluginProcess::currentProcessStopped
     */
    void pluginProcessCrashed (); 
    
private:

    UpdateProcess *q_ptr;

    WebUpload::Account * m_account; //!< Account updated

    WebUpload::ServiceOption * m_option; //!< Post option updated or null

    QString m_value; //!< Value added if not empty

    //! <code>true</code> if cancel has been called, else <code>false</code>
    bool m_cancelCalled; 

    friend class UpdateProcess;

};

}

#endif
