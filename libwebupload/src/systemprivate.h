 
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

#ifndef _WEBUPLOAD_SYSTEM_PRIVATE_H_
#define _WEBUPLOAD_SYSTEM_PRIVATE_H_

#include <QObject>
#include <QString>
#include "WebUpload/System"
#include <accounts-qt/manager.h>

namespace WebUpload {

    class SystemPrivate : public QObject {
    
        Q_OBJECT
    
    public:
    
        /*!
          \brief Constructor
          \param parent Public object
         */
        SystemPrivate (System * parent = 0);
        
        virtual ~SystemPrivate();
        
        /*!
          \brief Will use Accounts' manager and load all accounts
          \param serviceType Filtering type
          \param includeCustomUI If accounts with custom UI services should be
                                 included to list.
         */
        QList <QSharedPointer<Account> > loadAccounts (
            const QString & serviceType = QLatin1String("sharing"),
            bool includeCustomUI = false);
        
        /*!
          \brief Connect and disconnect signals
          \param on If true connect, if false disconnect
         */
        void enableAccountListener (bool on);
        
        //! Static variable for presentation data loading
        static bool m_loadPresentationData;
        
        //! Static variable for metatype registering
        static bool m_metatypesRegistered;

        System * publicObject; //!< Public object                
    
        QString serviceDefinitionsPath; //!< Filepath to definitions
        QString entryOutboxPath; //!< Filepath to outbox
        QString m_pluginProcessPath; //!< Filepath to plugins

        Accounts::Manager manager; //!< Account manager

        //!< If true account events are followed
        bool accountsListenerEnabled;
        
    public Q_SLOTS:
        
        /*!
          \brief Slot for Accounts account created signal
         */
        void accountsAccountCreated (Accounts::AccountId id);
        
    Q_SIGNALS:
    
        /*!
          \brief Signal emitted when account is created and loaded
         */
        void newAccount (QSharedPointer<WebUpload::Account> account);
    };
}

#endif
