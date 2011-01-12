 
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

#include "WebUpload/Account"
#include "accountprivate.h"
#include "WebUpload/Account"
#include <QDebug>

using namespace WebUpload;

Account::Account(QObject *parent): QObject(parent),
    d_ptr (new AccountPrivate(this)) {

    connect (d_ptr, SIGNAL (removed()), this, SIGNAL (removed()));
    connect (d_ptr, SIGNAL (enabled()), this, SIGNAL (enabled()));
    connect (d_ptr, SIGNAL (disabled()), this, SIGNAL (disabled()));
}
 
Account::~Account() {
    delete d_ptr;
}

bool Account::init (Accounts::AccountId accountId,
    const QString & serviceName) {
    
    // Validate state
    if (d_ptr->m_service != 0) {
        qWarning() << "Account already initialized";
        return false;
    }
    
    d_ptr->m_accountId = accountId;
    d_ptr->m_serviceName = serviceName;
    
    // Try to load account   
    return d_ptr->load();
}

bool Account::initFromStringId (const QString & id) {

    // Validate state
    if (d_ptr->m_service != 0) {
        qWarning() << "Account already initialized";
        return false;
    }

    // Validate input
    if (!(d_ptr->fromStringPresentation (id))) {
        return false;
    }
    
    // Try to load account
    return d_ptr->load();
}

QString Account::stringId() const {  
    return d_ptr->stringPresentation();
}

QVariant Account::value (const QString & id) {
    
    qDebug() << "Try to get option" << id << "from"
        << d_ptr->m_settings->fileName();
    
    QString key = d_ptr->stringPresentation();
    key.append ("/");
    key.append (id);
        
    return d_ptr->m_settings->value (key);        
}

void Account::syncValues () {
    qDebug() << "Account sync start";
    /* Account option
    d_ptr->m_aAccount->syncAndBlock();
    */
    d_ptr->m_settings->sync();
    qDebug() << "Account sync end";
}

void Account::setValue (const QString & id, const QVariant & value) {

/* Account option
    d_ptr->m_aAccount->setValue (id, value);
*/

    QString key = d_ptr->stringPresentation();
    key.append ("/");
    key.append (id);

    qDebug() << "Writing to " << id << " value " << value;
    d_ptr->m_settings->setValue (key, value);
    
    //TODO: Workaround for QSetting issues
    d_ptr->m_settings->sync ();
}

Accounts::AccountId Account::id() const {
    return d_ptr->m_accountId;
}

QString Account::serviceName() const {
    return d_ptr->m_serviceName;
}

Service * Account::service() const {
    return d_ptr->m_service;
}

QString Account::name() {
    if (d_ptr->m_aAccount == 0) {
        return QString();
    } else {
        return d_ptr->m_aAccount->displayName();
    }
}

Accounts::Account * Account::accountsObject() {
    return d_ptr->m_aAccount;
}

bool Account::isValid() const {
    if (d_ptr != 0 && d_ptr->m_aAccount != 0 && d_ptr->m_service != 0) {
        return d_ptr->isValid ();
    } else {
        return false;
    }
}

enum ImageResizeOption Account::imageResizeOption () {
    QVariant val = value ("image-resize");
    
    bool ok = false;
    enum ImageResizeOption ret = (enum ImageResizeOption) val.toInt (&ok);
    
    if (!ok || ret < IMAGE_RESIZE_NONE || ret >= IMAGE_RESIZE_N) {
        ret = IMAGE_RESIZE_NONE;
    }
    
    return ret;
}

enum VideoResizeOption Account::videoResizeOption () {
    QVariant val = value ("video-resize");
    
    bool ok = false;
    enum VideoResizeOption ret = (enum VideoResizeOption) val.toInt (&ok);
    
    qDebug() << "Account::videoResizeOption:" << val.toInt () << ok;
    if (!ok || ret < VIDEO_RESIZE_NONE || ret >= VIDEO_RESIZE_N) {
        ret = VIDEO_RESIZE_NONE;
    }
    
    return ret;
}

int Account::metadataFilters () {
    QVariant val = value ("metadata-filters");
    bool ok = false;
    int ret = val.toInt (&ok);
    
    if (!ok) {
        ret = METADATA_FILTER_NONE;
    }
    
    return ret;
}
            
void Account::setImageResizeOption (enum ImageResizeOption value) {
    if (value >= IMAGE_RESIZE_NONE && value < IMAGE_RESIZE_N) {
        int num = value;
        setValue ("image-resize", num);
    } else {
        qWarning() << "Invalid image resize value given:" << value;
    }
}

void Account::setVideoResizeOption (enum VideoResizeOption value) {
    qDebug() << "Account::setVideoResizeOption" << value;
    if (value >= VIDEO_RESIZE_NONE && value < VIDEO_RESIZE_N) {
        int num = value;
        setValue ("video-resize", num);
    } else {
        qWarning() << "Invalid video resize value given:" << value;
    }
}

void Account::setMetadataFilters (int flags) {
    setValue ("metadata-filters", flags);
}

/* -- private class functions ----------------------------------------------- */
AccountPrivate::AccountPrivate (Account * parent) : QObject (parent),
    m_accountId (0), m_sAccount (parent), m_service (0), m_aAccount (0),
    m_settings (new QSettings ("nokia", "webupload-engine")), m_aManager (0),
    m_accountEnabled (false), m_serviceEnabled (false) {

}

AccountPrivate::~AccountPrivate() {
    if (m_service != 0) {
        delete m_service;
    }
    
    if (m_aAccount != 0) {
        delete m_aAccount;
    }
    
    if (m_aManager != 0) {
        delete m_aManager;
    }
    
    if (m_settings != 0) {
        delete m_settings;
    }
}

QString AccountPrivate::stringPresentation() const {

    //<<account id>>:<<service name>>
    QString pres;
    
    pres.setNum (m_accountId, 16);
    pres.append (":");
    pres.append (m_serviceName);
    
    QString withPrefix = "Accounts:";
    withPrefix.append (pres);
    
    return withPrefix;
}

bool AccountPrivate::fromStringPresentation (const QString & str) {    
    if (!(str.startsWith ("Accounts:"))) {
        qWarning() << "Invalid string prefix in account" << str;
        return false;
    }

    QString aIdStr = str.section(':', 1, 1);
    QString serviceStr = str.section(':', 2);
    
    qDebug() << "Received account id" << aIdStr << serviceStr;
    
    bool ok = false;
    Accounts::AccountId aId = aIdStr.toUInt (&ok, 16);
    if (ok) {
        ok = !(serviceStr.isEmpty());
    }
 
    if (ok) {
        m_accountId = aId;
        m_serviceName = serviceStr;
    } else {
        qWarning() << "Invalid account string:" << str << "=>" << aId
            << serviceStr;
    }
    
    return ok;   
}

bool AccountPrivate::load () {

    // Validate state
    if (m_aAccount != 0 || m_service != 0) {
        qWarning() << "Account already initialized";
        return false;
    }

    // Validate input
    if (m_serviceName.isEmpty()) {
        qCritical() << "Service name not set";
        return false;
    }
    
    if (m_aManager == 0) {
        m_aManager = new Accounts::Manager ("sharing", m_sAccount);
    }
    
    m_aAccount = m_aManager->account (m_accountId);
    if (m_aAccount == 0) {
        qCritical() << "Failed to load account from Accounts";
        return false;
    }
    
    m_accountEnabled = m_aAccount->enabled ();

    // Load Accounts' service
    Accounts::Service * aService = m_aManager->service (m_serviceName);
    if (aService == 0) {
        qCritical() << "Failed to load service from Accounts";
        m_aAccount = 0;
        return false;
    }
    
    // Pair Accounts' service to Accounts' account
    m_aAccount->selectService (aService);
    m_serviceEnabled = m_aAccount->enabled ();
    
    /* When we move to use account storage
    m_aAccount->beginGroup ("sharing");
    */
    
    // Finally load Sharing's service
    m_service = new Service (m_sAccount);
    if (!m_service->initFromAccountsService (aService)) {
        qCritical () << "Failed to initialize service from Accounts' service";
        delete m_service;
        m_service = 0;
        delete m_aAccount;
        m_aAccount = 0;       
        return false;
    }
    
    // Connect removed signal from Accounts' side
    connect (m_aAccount, SIGNAL (removed()), this,
        SLOT (accountsRemoved()));
    connect (m_aAccount, SIGNAL (enabledChanged(QString,bool)), this,
        SLOT (enabledChanged(QString,bool)));
    
    return true;
}

bool AccountPrivate::isValid () const {
    return m_accountEnabled && m_serviceEnabled;
}

void AccountPrivate::accountsRemoved () {
    qDebug() << "Account removed received";

    delete m_aAccount;
    m_aAccount = 0;
    delete m_aManager;
    m_aManager = 0;
    
    m_accountId = 0;
    m_serviceName = "";
    m_accountEnabled = false;
    m_serviceEnabled = false;

    Q_EMIT (removed());
}

void AccountPrivate::enabledChanged (const QString &serviceName, 
    bool isEnabled) {

    qDebug() << "enabled changed:" << isEnabled << serviceName << "("
        << m_serviceName << ")";
        
    bool other;
    if ((serviceName == m_serviceName) && (m_serviceEnabled != isEnabled)) {
        m_serviceEnabled = isEnabled;
        other = m_accountEnabled;
    } else if ((serviceName == "global") && (m_accountEnabled != isEnabled)) {
        m_accountEnabled = isEnabled;
        other = m_serviceEnabled;
    } else {
        // Ignore this call - nothing changed
        return;
    }

    if (other == true) {
        if (isEnabled == true) {
            Q_EMIT (enabled());
        } else {
            Q_EMIT (disabled());
        }
    }
}