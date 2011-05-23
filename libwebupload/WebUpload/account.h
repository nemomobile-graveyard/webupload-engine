 
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

#ifndef _WEBUPLOAD_ACCOUNT_H_
#define _WEBUPLOAD_ACCOUNT_H_

#include <WebUpload/export.h>
#include <WebUpload/Service>
#include <WebUpload/enums.h>
#include <QObject>
#include <QVariant>
#include <Accounts/Account>
#include <QSharedPointer>

namespace WebUpload {

    class AccountPrivate;

    /*!
      \class Account
      \brief Account class is used to connect service account's in Accounts
             subsystem to webupload services and options.
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT Account : public QObject {
        Q_OBJECT
        
    public:
        /*!
          \brief Load account
          \param parent QObject parent
         */
        Account (QObject * parent = 0);
        
        virtual ~Account ();
        
        /*!
          \brief Initialize from Accounts.
          \param accountId ID of account is Accounts
          \param serviceName Name of service under account
          \return true if account was initialized correctly
         */
        bool init (Accounts::AccountId accountId,
            const QString & serviceName);
        
        /*!
          \brief Initialize from Accounts with string id
          \param id String presentation of account. Received from
                    <code>stringId</code> function.
          \return true if account was initialized correctly
         */
        bool initFromStringId (const QString & id);
            
        /*!
          \brief String id of account. Can be used with initFromStringId.
          \return String id of account
         */
        QString stringId() const;
        
        /*!
          \brief Get Accounts' account ID
          \return Id of account in Accounts
         */
        Accounts::AccountId id() const;
        
        /*!
          \brief Get Account's service name
          \return Name of service related to account
         */
        QString serviceName() const;

        /*!
          \brief Get id of active option value
          \param id ID of option which value we want
          \return id ID or empty string if not found
         */
        QVariant value (const QString & id);
        
        /*!
          \brief Set value of option
          \param id ID of option
          \param value New value
         */
        void setValue (const QString & id, const QVariant & value);
        
        /*!
          \brief Blocking call to make sure that values are synced. This is
                 needed to make sure multiple processes have identical values.
         */
        void syncValues ();
        
        /*!
          \brief Get default image resize option value
          \return Resize value
         */
        enum ImageResizeOption imageResizeOption ();
        
        /*!
          \brief Get default video resize option value
          \return Resize value
         */
        enum VideoResizeOption videoResizeOption ();
        
        /*!
          \brief Get default metadata filter option flags
          \return Filter flags. See enum WebUpload::MetadataFilter
         */
        int metadataFilters ();
        
        /*!
          \brief Set default image resize option value
          \param value New value for resize
         */
        void setImageResizeOption (enum ImageResizeOption value);
        
        /*!
          \brief Set default video resize option value
          \param value New value for resize
         */
        void setVideoResizeOption (enum VideoResizeOption value);
        
        /*!
          \brief Set default metadata filter flags
          \param flags Flags. See enum WebUpload::MetadataFilter
         */
        void setMetadataFilters (int flags);
        
        /*!
          \brief Get service class related to account
          \return Pointer to service class or null
         */
        Service * service() const;
        
        /*!
          \brief Get name for account than can be used in UI or logs
          \return Name of account (usually username)
         */
        QString name ();
        
        /*!
          \brief Get Account's account object related to this Account
          \return Accounts' account object or null if no connection. Do not
                  delete this object.
         */
        Accounts::Account * accountsObject();
        
        /*!
          \brief Check if account is initialized correctly and it is still
                 valid (not removed)
          \return true If account is valid
         */
        bool isValid() const;
        
    Q_SIGNALS:
    
        /*!
          \brief Signal emitted when account is removed from system.
                 When this is emitted, account or it's components should not
                 be used anymore.
         */
        void removed ();
    
        /*!
          \brief Signal emitted when account is enabled
         */
        void enabled ();
    
        /*!
          \brief Signal emitted when account is disabled
         */
        void disabled ();
    
    private:
        Q_DISABLE_COPY(Account)
        AccountPrivate * const d_ptr; //!< Private data
    };
    
    typedef QSharedPointer<Account> SharedAccount;    
}

#endif
