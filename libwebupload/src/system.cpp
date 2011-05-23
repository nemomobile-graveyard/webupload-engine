 
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

#include "WebUpload/System"
#include "WebUpload/Error"
#include "WebUpload/Media"
#include "WebUpload/Account"
#include "WebUpload/enums.h"
#include "systemprivate.h"
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QTemporaryFile>
#include <Accounts/Account>
#include <Accounts/Manager>
#include "uploadengine.h"
#include <QSystemStorageInfo>
#include <MLocale>

using namespace WebUpload;

System::System (QObject * parent) : QObject (parent),
    d_ptr (new SystemPrivate (this)) {
    
    QObject::connect (d_ptr,
        SIGNAL (newAccount (QSharedPointer<WebUpload::Account>)), this,
        SIGNAL (newAccount (QSharedPointer<WebUpload::Account>)));
}

System::~System() {
    delete d_ptr;
}

QString System::pluginProcessPathForAccount (const Account * account) {

    QString path; 
    
    if (account != 0 && account->service() != 0) {
        QString name = account->service()->pluginProcessName();
        
        if (name.isEmpty() == false) {
            QDir pluginsDir = QDir(pluginProcessPath());
            path = pluginsDir.absoluteFilePath (name);
        }
    }
    
    return path;        
}

void System::setEntryOutboxPath (const QString & path) {
    d_ptr->entryOutboxPath = path;
}

QString System::entryOutputPath () const {
    return d_ptr->entryOutboxPath;
}

QList <QSharedPointer<Entry> > System::outboxEntries () {
    QList <QSharedPointer<Entry> > list;
    
    QDir dir = d_ptr->entryOutboxPath;
    QStringList filters;
    filters << "entry_*.xml";
    dir.setNameFilters (filters);
    QStringList entryFiles = dir.entryList (filters, QDir::Files, QDir::Name);
    
    for (int i = 0; i < entryFiles.count(); ++i) {
        QSharedPointer <Entry> entry = QSharedPointer <Entry> (new Entry);
        if (entry->init (dir.filePath(entryFiles.at (i)))) {
            list.append (entry);
        } else {
            qWarning() << "Invalid entry found in outbox, removing it..." <<
                dir.filePath(entryFiles.at (i)) ;
            Entry::cleanUp (dir.filePath(entryFiles.at (i)));
        }
    }
    
    return list;    
}

QString System::serializeEntryToOutbox (Entry * entry) {
    if (entry == 0) {
        qWarning() << "Null entry not accepted";
        return "";
    }
        
    // Create path if needed
    QDir dir;
    if(!(dir.exists(entryOutputPath ()))) {
        if(!dir.mkpath(entryOutputPath ())) {
            qCritical() << "Could not create path" << dir.absolutePath();
            return "";
        }
    }

    // Start with template
    QString entryPath = entryOutputPath () + "/entry_XXXXXX.xml";

    // Make temp file
    QTemporaryFile tempFile (entryPath);
    tempFile.setAutoRemove (false);
    if (tempFile.open()) {
        entryPath = tempFile.fileName();
        tempFile.close();
    } else {
        qWarning() << "Can't create temp file" << tempFile.fileName();
        return "";
    }
    
    // Try to serialize
    if (entry->serialize(entryPath)) {
        qDebug() << "Entry serialized to" << entryPath;
        return entryPath;
    } else {
        qCritical() << "Failed to serialize entry to" << entryPath;
        // Clean out temp file left behind
        tempFile.remove ();
        return "";
    }
}

QList <QSharedPointer<Account> > System::allAccounts (bool filterOutCustomUI) {
    return d_ptr->loadAccounts ("sharing", !filterOutCustomUI);
}

SharedAccount System::sharedAccount (const QString & stringId) {
    SharedAccount ret;
    Account * account = new Account ();
    
    if (account->initFromStringId (stringId) == false) {
        qWarning() << "System failed to load account" << stringId;
        delete account;
        account = 0;
    } else {
        qDebug() << "System loaded account" << stringId;
        ret = SharedAccount (account);
    }

    return ret;
}

Account * System::account (const QString & stringId, QObject * parent) {

    Account * account = new Account (parent);
    if (!(account->initFromStringId (stringId))) {
        qWarning() << "System failed to load account" << stringId;
        delete account;
        account = 0;
    } else {
        qDebug() << "System loaded account" << stringId;
    }

    return account;
}

void System::setAccountListenerEnabled (bool enabled) {
    d_ptr->enableAccountListener (enabled);
}

bool System::loadPresentationDataEnabled() {
    return SystemPrivate::m_loadPresentationData;
}

void System::setLoadPresentationDataEnabled (bool load) {
    qDebug() << "Load presentation data set to" << load;
    SystemPrivate::m_loadPresentationData = load;
}

void System::registerMetaTypes () {
    
    if (SystemPrivate::m_metatypesRegistered == true) {
        return;
    }
    
    qDebug() << "Register WebUpload meta types";
    qRegisterMetaType<WebUpload::Error>("WebUpload::Error");
    qRegisterMetaType<WebUpload::Error::Code>("WebUpload::Error::Code");
    qRegisterMetaType<WebUpload::VideoResizeOption>(
        "WebUpload::VideoResizeOption");    
    qRegisterMetaType<WebUpload::ImageResizeOption>(
        "WebUpload::ImageResizeOption");
    qRegisterMetaType<WebUpload::Media*>("WebUpload::Media*");
    qRegisterMetaType<WebUpload::Media::CopyResult> (
        "WebUpload::Media::CopyResult");
    SystemPrivate::m_metatypesRegistered = true;
}

void System::loadLocales () {

    MLocale locale;
    // Translation path needs to be added translation to work with
    // non-meegotouch apps. We don't know if the translation path has been
    // added before calling this library, hence adding it again
    MLocale::addTranslationPath ("/usr/share/l10n/meegotouch");
    // Install actuall application catalog
    locale.installTrCatalog ("transfer");
    // Install application catalog (engineering english)
    locale.installTrCatalog ("libwebupload");
    MLocale::setDefault (locale);
}

System::EngineResponse System::sendEntryToUploadEngine (
    WebUpload::Entry * entry) {
    
    System::EngineResponse res = validateEntry (entry);
    if (res != System::ENGINE_RESPONSE_OK) {
        return res;
    }
    
    QString path = entry->serializedTo();
    if (path.isEmpty()) {
        // Write entry to filesystem    
        qDebug() << __FUNCTION__ << ": serialize entry";
        WebUpload::System system;
        path = system.serializeEntryToOutbox (entry);
    } else {
        qDebug() << __FUNCTION__ << ": entry already serialized";    
    }

    // Tell upload engine to add task to queue
    if (path.isEmpty() == false) {
    
        UploadEngine interface("com.meego.sharing.webuploadengine");
        if (interface.isValid() == true) {
            bool retVal = interface.newUpload (path);
            qDebug() << __FUNCTION__ << ": response" << retVal;            
        } else {
            qWarning() << __FUNCTION__ << ": dbus connection failure";
            res = System::ENGINE_RESPONSE_CONNECTION_FAILURE;
        }
    } else {
        res = System::ENGINE_REPONSE_FAILED_TO_SERIALIZED;
        qWarning() << __FUNCTION__ << ": failed to serialize entry";
    }
    
    return res;
}

System::EngineResponse System::validateEntry (WebUpload::Entry * entry) {
    
    System::EngineResponse res = System::ENGINE_RESPONSE_OK;

    qDebug() << __FUNCTION__ << ": called";

    // Ok here's the problem: the target dir can be different for each medium
    // in the entry, but will it ever in real life be other than .share?
    // We need to have enough space for preprocessing all media before trying
    // to upload anything. Further checks are done later anyway, so let's just
    // assume everything goes under ~/MyDocs/.share. Bying checking available
    // space now we make sure the engine doesn't hang up and doesn't leave 
    // garbage xml files.
    QDir targetDir = QDir::homePath() + "/MyDocs/.share/";
    bool enoughDiskSpace = checkDiskSpace(targetDir, entry);

    if (!enoughDiskSpace) {
        return System::ENGINE_RESPONSE_NO_DISKSPACE;
    }

    QVectorIterator<Media *> mediaIter = entry->media();
    while (mediaIter.hasNext ()) {
        Media * media = mediaIter.next ();
        QString filePath = media->srcFilePath ();
        qDebug() << "SYSTEM::" << __FUNCTION__ << filePath;
        if ((!filePath.isEmpty ()) && (!QFile::exists (filePath))) {
            return System::ENGINE_RESPONSE_FILES_MISSING;
        }
    }

    return res;
}

void System::setPluginProcessPath (const QString & path) {
    d_ptr->m_pluginProcessPath = path;
}

QString System::pluginProcessPath () {
    return d_ptr->m_pluginProcessPath;
}


// -- private functions -------------------------------------------------------
SystemPrivate::SystemPrivate (System * parent) : QObject (parent),
    publicObject (parent), accountsListenerEnabled (false) {
    
    m_pluginProcessPath = QLatin1String("/usr/lib/webupload/plugins/");
    serviceDefinitionsPath = QLatin1String("/usr/share/webupload/services");
    entryOutboxPath = QDir::homePath() + QLatin1String("/.share");
    
}

SystemPrivate::~SystemPrivate() {
    enableAccountListener (false);
}

QList <QSharedPointer<Account> > SystemPrivate::loadAccounts (
    const QString & serviceType, bool includeCustomUI) {
    
    qDebug() << "PERF: LoadAccounts START";
    
    QList <QSharedPointer<Account> > retList;
    Accounts::AccountIdList idList = manager.accountList (serviceType);
    
    for (int i = 0; i < idList.size (); ++i) {
        Accounts::Account * aAccount = manager.account (idList [i]);
        
        if (aAccount == 0) {
            qWarning() << "Null account received" << i + 1;
            continue;
        }
        
        Accounts::ServiceList sList = aAccount->services (serviceType);
         
        for (int j = 0; j < sList.size (); ++j) {
            Accounts::Service * aService = sList [j];
            
            aAccount->selectService (aService);
            qDebug() << "Service" << aService->displayName() << "found for"
                << "account" << aAccount->displayName();
            
            QSharedPointer <Account> sAccount = QSharedPointer <Account> (
                new Account);
                
            if (sAccount->init (idList [i], aService->name())) {
                bool append = true;
            
                if (includeCustomUI == false) {
                    if (sAccount->service()->publishCustom() ==
                        Service::PUBLISH_CUSTOM_TOTAL) {
                        append = false;
                    }
                }
                
                if (append == true) {
                    retList.append (sAccount);
                }
            }
        }
         
    }
    
    qDebug() << "PERF: LoadAccounts END";
    
    return retList;     
}

void SystemPrivate::enableAccountListener (bool on) {
    // If no change just return
    if (accountsListenerEnabled == on) {
        return;
    }
    
    accountsListenerEnabled = on;
    
    if (!on) {
        qDebug() << "Account listener disabled";
        manager.disconnect (SIGNAL (accountCreated(Accounts::AccountId)), this);
    } else {
        qDebug() << "Account listener enabled";
        QObject::connect (&manager,
            SIGNAL (accountCreated(Accounts::AccountId)), this,
            SLOT (accountsAccountCreated(Accounts::AccountId)));    
    }
}

void SystemPrivate::accountsAccountCreated (Accounts::AccountId id) {

    // Safety check
    if (accountsListenerEnabled == false) {
        qDebug() << "Account created and ignored";
        return;
    }
    
    //Check for sharing services under created account
    Accounts::Account * aAcc = manager.account (id);
    if (aAcc == 0) {
        qWarning() << "Invalid account ID from Accounts Manager";
        return;
    }
    
    if (aAcc->supportsService ("sharing") == false) {
        qDebug() << "Account created but without sharing support, ignored.";
        delete aAcc;
        return;
    }
    
    qDebug() << "Accounts' new account signal received" << id;
    
    Accounts::ServiceList services = aAcc->services ("sharing");
    
    for (int i = 0; i < services.count(); ++i) {
        Accounts::Service * aService = services.at (i);
        
        WebUpload::SharedAccount sAccount (new WebUpload::Account (0));
            
        if (sAccount->init (aAcc->id(), aService->name()) == true) {
            qDebug() << "Emitting signal for new account" << sAccount->name();
            Q_EMIT (newAccount(sAccount));
        }
    }
    
    delete aAcc;    
}



bool System::checkDiskSpace(QDir targetDir, WebUpload::Entry* entry)
{
    qDebug() << "check disk space for " << targetDir.absolutePath();

    while (!targetDir.exists()){
        targetDir.cdUp();
    }

    QString targetPath = targetDir.absolutePath();

    bool enoughSpace = true;
    qint64 spaceRequired = 0;

    if (entry != 0) {
        spaceRequired = entry->totalSize();
    }

    if (spaceRequired > 0) {
        qint64 safetyMargin = 2 * 1024 * 1024;        
        spaceRequired += safetyMargin;

        QtMobility::QSystemStorageInfo storageInfo;
        qint64 availableSpace =
                storageInfo.availableDiskSpace(targetPath);
        qDebug() << "available space: " << availableSpace;

        if (spaceRequired > availableSpace) {
            qWarning() << "not enough space";
            enoughSpace = false;
        }
    } else {
        // Size 0 is accepted. For example link shares have no size.
        enoughSpace = true;
        qCritical() << "Size unknown, no space check for copy done";
    }

    return enoughSpace;
}

//Set default values for static variables
bool SystemPrivate::m_loadPresentationData = true;
bool SystemPrivate::m_metatypesRegistered = false;
