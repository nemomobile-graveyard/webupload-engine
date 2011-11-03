 
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

#ifndef _WEBUPLOAD_ACCOUNT_PRIVATE_H_
#define _WEBUPLOAD_ACCOUNT_PRIVATE_H_

#include <QObject>
#include <QString>
#include <QSettings>
#include <QSharedPointer>
#include <Accounts/Account>
#include <Accounts/Manager>

namespace WebUpload {

    class Account;
    class Service;

    class AccountPrivate : public QObject {
    
    Q_OBJECT

    public:
        AccountPrivate (Account * parent = 0);

        virtual ~AccountPrivate();
                    
        /*! Convert id values to string */
        QString stringPresentation() const;
        
        /*! Set id values from string */
        bool fromStringPresentation (const QString & str);
        
        /*!
          \brief Call after accountId and serviceName are set
          \return false if account failed to load
         */
        bool load ();
        
        /*!
          \brief Call after account is loaded. Tells whether the account is
                 valid (i.e both account and service are enabled)
          \return <code>true</code> if both account and service are enabled and
                  if the account has not been deleted yet, else
                  <code>false</code>
         */
        bool isValid () const; 
        
        Accounts::AccountId m_accountId; //!< Account Id (Accounts)
        QString m_serviceName; //!< Service name (Accounts)            
        
        Account * m_sAccount; //!< Parent
        Service * m_service; //!< Service loaded            
        Accounts::Account * m_aAccount; //!< Accounts' account

        //! Temp storage until we can store values to Accounts
        QSettings * m_settings;
        
        QSharedPointer<Accounts::Manager> m_aManager; //!< Account's manager

    public Q_SLOTS:
        
        /*!
          \brief Slot for Accounts::Account:removed
         */
        void accountsRemoved ();

        /*!
          \brief Slot for Accounts::Account::enabledChanged
          \param serviceName Name of the service which has been
                    enabled/disabled
          \param isEnabled Was the service enabled or disabled
         */
         void enabledChanged (const QString &serviceName, bool isEnabled);
        
    Q_SIGNALS:
    
        /*!
          \brief Emitted after accountsRemoved is called.
         */
        void removed();

        /*!
          \brief Emitted when the account-service pair is disabled
         */
        void disabled();

        /*!
          \brief Emitted when the account-service pair is enabled
         */
        void enabled();
                    
    private:
        // Currently Accounts::Account emits enabledChanged signal for the
        // service only if the service enable/disable value is changed,
        // irrespective of the actual account enable/disable status. So, we
        // store here both the account and the service enable status, and emit
        // enabled signal only if both account and service are enabled. 
        bool m_accountEnabled; //!< Is account is enabled in Accounts&SSO
        bool m_serviceEnabled; //!< Is service is enabled in Accounts&SSO
    
#ifdef TESTING
        friend class LibWebUploadTests; //!< For unit tests
#endif
            
    };
 
}

#endif
