 
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

#ifndef _WEBUPLOAD_SYSTEM_H_
#define _WEBUPLOAD_SYSTEM_H_

#include <QObject>
#include <QList>
#include <QDir>
#include <QSharedPointer>
#include <WebUpload/Service>
#include <WebUpload/Entry>
#include <WebUpload/Account>

namespace WebUpload {

    class SystemPrivate;
    class Entry;

    /*!
        \class System
        \brief Class for handling system setup
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT System : public QObject {
    
        Q_OBJECT
    
    public:

        /*!
          \brief See QObject
          \param parent QObject parent
         */
        System (QObject * parent = 0);
    
        virtual ~System();
        
        /*!
          \brief Set path where plugins are loaded
          \param path Path where plugins are loaded
         */
        void setPluginProcessPath (const QString & path);
        
        /*!
          \brief Path where plugins are loaded
          \return Plugin path
         */
        QString pluginProcessPath ();

        /*!
          \brief Returns full file path to accounts binary plugin
          \return File path of the plugin process if account is valid
         */
        QString pluginProcessPathForAccount (const Account * account);
        
        /*!
          \brief Set outbox path for entries. Mainly useful for testing.
         */            
        void setEntryOutboxPath (const QString & path);

        /*!
          \brief Get outbox path of entries
          \return Path used to load entried
         */            
        QString entryOutputPath () const;
        
        /*!
          \brief Stores entry to outbox
          \param entry serialized to outbox
          \return Path to serialized entry or empty string if failure
         */
        QString serializeEntryToOutbox (Entry * entry);
        
        /*!
          \brief Read entries found in outbox
          \return List of entries found
         */
        QList <QSharedPointer<Entry> > outboxEntries ();
        
        /*!
          \brief Get all accounts of WebUpload. Use this only in case
                 where you will support most of all accounts available in
                 system. You you only need one type of accounts then use
                 Accounts framework directly. This is heavy operation as it will
                 load all information for accounts it finds.
          \param filterOutCustomUI If accounts with custom UI should be filtered
                                   out.
          \return List of accounts found.
         */        
        QList <QSharedPointer<Account> > allAccounts (
            bool filterOutCustomUI = true);
        
        /*!
          \brief Get account with id and service name
          \param stringId String ID of account to be loaded
          \param parent Parent for account loaded
          \return Account matching with given values. null if not found.
         */
        Account * account (const QString & stringId, QObject * parent = 0); 
        
        SharedAccount sharedAccount (const QString & stringId);   
                
        /*!
          \brief Enable account listener that will emit signal newAccount
                 when new account are created.
          \param enabled If <code>true</code> then signal newAccount is emitted
                         always when suitable account is created. If
                         <code>false</code> then account events are ignore and 
                         no signals are emitted.
         */
        void setAccountListenerEnabled (bool enabled = true);
        
        /*!
          \brief If presentation data is needed
          \return <code>true</code> if presentation data is needed
         */
        static bool loadPresentationDataEnabled();
        
        /*!
          \brief Set flag to load presentation data. By default all that will
                 be loaded for services and accounts received via this class.
                 If set to <code>false</code> then presentation data loading
                 is skipped to give performance boost. This is useful for
                 background processes. This is also way to avoid loading
                 translation catalogs.
          \param load If <code>false</code> some presentation data will not be
                      loaded to give performance boost.
         */
        static void setLoadPresentationDataEnabled (bool load);
        
        /*!
          \brief Register metatypes used in signals. Can be called multiple
                 times, but after first run will not do anything.
         */
        static void registerMetaTypes ();

        /*!
          \brief Loads locales needed by the library. Call this after QApplication
                 instance has been created.
        */
        static void loadLocales();
        
        /*!
          \brief Response codes for sendEntryToUploadEngine and validateEntry
         */
        enum EngineResponse {
            //! Entry sent successfully to engine        
            ENGINE_RESPONSE_OK,

            //! Failed to connect to engine
            ENGINE_RESPONSE_CONNECTION_FAILURE,
            
            //! Failed to serialize the entry
            ENGINE_REPONSE_FAILED_TO_SERIALIZED,

            //! Not enough disk space for processing the entry
            ENGINE_RESPONSE_NO_DISKSPACE,

            //! One or more files for sharing has been deleted
            ENGINE_RESPONSE_FILES_MISSING
        };
        
        /*!
          \brief Sent entry to be uploaded by engine. This function will
                 serialize entry before sending it if it isn't already
                 serialized. After this function is called entry must not be
                 serialized. Safest is to delete entry class after this
                 function returns successfully.
          \param entry Entry to be sent to engine
          \return Result of sent
         */      
        static EngineResponse sendEntryToUploadEngine (
            WebUpload::Entry * entry);    
        
        /*!
          \brief Validate the entry being sent to the engine. 
          \param entry Entry to be validated
          \return Validation result
         */
        static EngineResponse validateEntry (WebUpload::Entry * entry);

        /*!
          \brief Deprecated, use allAccounts
         */
        Q_DECL_DEPRECATED QList <QSharedPointer<Account> > * accounts (
            bool includeCustomUI = false);
            
        /*!
          \brief Deprecated use setAccountListenerEnabled
         */
        Q_DECL_DEPRECATED void enableAccountListener (bool on = true); 
        
        /*!
          \brief Use loadPresentationDataEnabled
         */
        Q_DECL_DEPRECATED static bool loadPresentationData();
        
        /*!
          \brief Use setLoadPresentationDataEnabled
         */        
        Q_DECL_DEPRECATED static void setLoadPresentationData (bool load);        
        
    Q_SIGNALS:
    
        /*!
          \brief Signal emitted when new account is created to system.
                 enableAccountListener has to be called with true to get
                 these signals.
          \param account Account created and loaded  
         */
        void newAccount (QSharedPointer<WebUpload::Account> account);
        
    private:
        Q_DISABLE_COPY(System)
        SystemPrivate * const d_ptr; //!< Private data
        static bool checkDiskSpace(QDir targetPath, Entry* entry);
    };
}

Q_DECLARE_METATYPE(QSharedPointer<WebUpload::Account>)

#endif