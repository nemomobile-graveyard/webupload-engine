 
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

#include "WebUpload/Service"
#include "WebUpload/Account"
#include "WebUpload/System"
#include "serviceprivate.h"
#include "WebUpload/CommonOption"
#include "WebUpload/ServiceOption"
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "xmlhelper.h"
#include <Accounts/Manager>
#include <MLocale>

using namespace WebUpload;

Service::Service(QObject * parent) : QObject(parent),
    d_ptr (new ServicePrivate(this)) {
    
}

Service::~Service() {
    delete d_ptr;
}

bool Service::initFromAccountsService (Accounts::Service * accService) {
    if (accService == 0) {
        qCritical() << "Can't init from null Accounts' service";
        return false;
    }
    
    if (!d_ptr->initFromAccountsServiceXml (accService->domDocument())) {
        return false;
    }
    
    d_ptr->m_accServiceName = accService->name();    
    d_ptr->m_iconName = accService->iconName();
    
    return true;
}

QString Service::name() const {
    return d_ptr->m_name;
}

QString Service::pluginProcessName() const {
    return d_ptr->m_uploadPlugin;
}

bool Service::acceptMimeType (const QString & mime) const {
    return XmlHelper::mimeTypeListCheck (mime, d_ptr->m_mimeTypes);
}

QListIterator<PostOption *> Service::postOptions() {
    d_ptr->loadServiceOptions ();
    QListIterator <PostOption *> iter (d_ptr->m_postOptions);
    return iter;
}

CommonOption * Service::commonOption (PostOption::Type optionType) {
    if (optionType >= PostOption::OPTION_TYPE_COMMON_TYPES_N) {
        return 0;
    }

    CommonOption * ret = 0;
    QListIterator<PostOption *> iter (d_ptr->m_postOptions);

    while (iter.hasNext() == true) {

        PostOption * option = iter.next ();
        if (option->type() == optionType) {

            CommonOption * opt = qobject_cast <CommonOption *> (option);
            if (opt) {
                ret = opt;
                break;
            }
        }
    }

    return ret;
}

ServiceOption * Service::serviceOption (const QString & id) {
    ServiceOption * ret = 0;

    if (d_ptr->loadServiceOptions ()) {
        QListIterator<PostOption *> iter (d_ptr->m_postOptions);
        while (iter.hasNext() == true) {
            PostOption * option = iter.next ();
            if (option->type() == PostOption::OPTION_TYPE_SERVICE) {

                ServiceOption * opt = qobject_cast <ServiceOption *> (option);
                if (opt && opt->id() == id) {
                    ret = opt;
                    break;
                }
            }
        }
    }
    
    return ret;
}

bool Service::hasUpdatablePostOptions() {

    if (d_ptr->loadServiceOptions ()) {
        QListIterator<PostOption *> iter (d_ptr->m_postOptions);
        while (iter.hasNext() == true) {
            PostOption * option = iter.next();
            if (option->type() == PostOption::OPTION_TYPE_SERVICE) {

                ServiceOption * opt = qobject_cast <ServiceOption *> (option);
                if (opt && opt->isUpdatable ()) {
                    return true;
                }
            }
        }
    }

    return false;
}

Account * Service::account() {
    return d_ptr->m_account;
}

QString Service::iconName() {
    return d_ptr->m_iconName;
}

bool Service::acceptsImages() const {
    return d_ptr->hasMimePrefix ("image/");
}

bool Service::acceptsVideos() const {
    return d_ptr->hasMimePrefix ("video/");
}

Service::PublishCustomLevel Service::publishCustom() const {
    return d_ptr->m_publishCustom;
}

void Service::refreshPostOptions () {
    if (d_ptr->m_account != 0) {
        d_ptr->m_account->syncValues();
    }

    if (d_ptr->loadServiceOptions ()) {
        QListIterator<PostOption *> iter (d_ptr->m_postOptions);
        while (iter.hasNext() == true) {
            PostOption * option = iter.next();
            if (option->type() == PostOption::OPTION_TYPE_SERVICE) {

                ServiceOption * opt = qobject_cast <ServiceOption *> (option);
                if (opt && opt->isUpdatable ()) {
                    opt->refresh (false);
                }
            }
        }
    }
}

QString Service::publishPlugin() const {
    return d_ptr->m_publishPlugin;
}

unsigned int Service::maxMediaLimit() const {
    return d_ptr->m_maxMedia;
}

/* --- Private functions ---------------------------------------------------- */

ServicePrivate::ServicePrivate (Service * parent) : m_service (parent), 
    m_serviceOptionsLoaded (false),
    m_publishCustom (Service::PUBLISH_CUSTOM_XML), m_maxMedia (0) {

    // Store account if relation between service and account
    if (m_service != 0) {
        m_account = qobject_cast<Account *>(m_service->parent());
    } else {
        m_account = 0;
    }
}

ServicePrivate::~ServicePrivate () {
    clearOptions ();
}

void ServicePrivate::clear () {
    clearOptions();
    m_name.clear();
    m_uploadPlugin.clear();
    m_mimeTypes.clear();
    m_publishCustom = Service::PUBLISH_CUSTOM_XML;
    m_publishPlugin.clear();
}

/*!
    \brief Check that valid data exists
    \return true if data is valid
 */
bool ServicePrivate::validDataExists () const {
    bool valid = true;

    if (m_uploadPlugin.isEmpty()) {
        qWarning() << "Missing webupload plugin";
        valid = false;
    }
        
    return valid;
}

QString ServicePrivate::captionValue (QDomElement & parentElem) {
    QDomNodeList nodes = parentElem.elementsByTagName ("caption");
    if (nodes.count() > 0) {
        return XmlHelper::getLocalizatedContent (nodes.at(0).toElement());
    }
    
    return "";
}

void ServicePrivate::populatePresentationData (const QDomElement & element) {

    bool loadPres = System::loadPresentationDataEnabled ();
    
    bool commonOptionPresent [PostOption::OPTION_TYPE_COMMON_TYPES_N];
    for (int i = 0; i < PostOption::OPTION_TYPE_COMMON_TYPES_N; ++i) {
        commonOptionPresent[i] = false;
    }

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement childElem = node.toElement();

        if (childElem.tagName() == "name") {
            m_name = XmlHelper::getLocalizatedContent (childElem);
            qDebug() << "Service with name" << m_name;            
    
        // To avoid loading unneeded information
        } else if (loadPres == false && 
            m_publishCustom == Service::PUBLISH_CUSTOM_XML) {
            
            qDebug() << "Service: skip tag" << element.tagName() << loadPres <<
                m_publishCustom;

        // Read things useful for other service types
        } else {
            CommonOption * option = CommonOption::getCommonOption (childElem,
                m_service);
            if (option != 0) {
                PostOption::Type optType = option->type ();
                if ((optType < PostOption::OPTION_TYPE_N) && 
                    (optType >= PostOption::OPTION_TYPE_TITLE)) {
                    addOption (option);
                    commonOptionPresent [optType] = true;
                }
            }
        }

        node = node.nextSibling();
    }

    for (int i = 0; i < PostOption::OPTION_TYPE_COMMON_TYPES_N; ++i) {
        if (commonOptionPresent[i] == false) {
            CommonOption * option = CommonOption::getCommonOption (
                (PostOption::Type)i, QString(), m_service);

            if (option == 0) {
                qWarning() << "Got null pointer with simple init for" << i;
            } else {
                addOption (option);
            }
        }
    }
}

void ServicePrivate::populateMediaData (const QDomElement & element) {

    //media amount limitations
    QString value = element.attribute (QLatin1String("maxAmount"),
        QLatin1String("0"));
    m_maxMedia = value.toUInt (0, 10);

    QDomNode node = element.firstChild();
    while (node.isNull() == false) {
    
        if (node.isElement() == true) {
            QDomElement childElem = node.toElement();

            if (childElem.tagName() == QLatin1String("formats")) {
                m_mimeTypes = XmlHelper::readMimeTypes (childElem);
            
            } else {
                qWarning() << "Service/Media: Unknown tag"
                    << childElem.tagName();
            }
        }

        node = node.nextSibling();
    }  
}

void ServicePrivate::addOption (PostOption * option) {
    if (option == 0) {
        qWarning() << __FUNCTION__ << "called with null pointer";
        return;
    }

    QList<PostOption *>::iterator iter;
    for (iter = m_postOptions.begin(); iter != m_postOptions.end(); ++iter) {
        if ((*iter)->order () > option->order ()) {
            break;
        }
    }

    m_postOptions.insert (iter, option);
    return;
}

bool ServicePrivate::initFromDefinition (const QDomElement & element) {

    QDomElement docElem = element;
    clear();
    
    //TODO: Check version
    QString version = docElem.attribute ("version", "1.0");
    qDebug() << "Used input definition version" << version; 
    
    // Load catalogs
    if (System::loadPresentationDataEnabled() == true) {
        QString catalogAttr = element.attribute ("trcatalogs", "");
        if (catalogAttr.isEmpty() == false) {
            QStringList catalogs = catalogAttr.split (" ");
            QString catalog;
            MLocale locale;           
            foreach (catalog, catalogs) {
                qDebug() << "Loading catalog" << catalog << "for service";
                locale.installTrCatalog (catalog);
            }
            MLocale::setDefault (locale);
        }    
    }
        
    // Load plugin
    m_uploadPlugin = docElem.attribute ("uploadPlugin", "");
    
    // Check custom UI
    bool oldCustom = XmlHelper::attributeValueToBool (docElem, "customUI",
        false);
        
    if (oldCustom == true) {
        m_publishCustom = Service::PUBLISH_CUSTOM_TOTAL;
    } else {
        m_publishCustom = Service::PUBLISH_CUSTOM_XML;
            
        QString pluginName = docElem.attribute ("customPage", "");
        if (pluginName.isEmpty() == false) {
            m_publishPlugin = pluginName;
            m_publishCustom = Service::PUBLISH_CUSTOM_PAGE;
        } else {
            pluginName = docElem.attribute ("customWidget", "");
            if (pluginName.isEmpty() == false) {
                m_publishPlugin = pluginName;
                m_publishCustom = Service::PUBLISH_CUSTOM_WIDGET;            
            }
        }
    }
    
    qDebug() << "Loading service" << m_uploadPlugin << m_publishCustom
        << m_publishPlugin;
    
    
    QDomNode node = docElem.firstChild();
    while (node.isNull() == false) {
    
        if (node.isElement() == true) {
            QDomElement childElem = node.toElement();

            if (childElem.tagName() == "presentation") {
                populatePresentationData (childElem);
                
            } else if (m_publishCustom != Service::PUBLISH_CUSTOM_TOTAL &&
                childElem.tagName() == "media") {
            
                populateMediaData (childElem);

            } else if (childElem.tagName().isEmpty() == false) {
                qDebug() << "Init skip tag" << childElem.tagName();
            }
        }
        
        node = node.nextSibling();        
    }
    
    qDebug() << "Service loaded" << validDataExists();
    
    return validDataExists ();
}

bool ServicePrivate::initFromAccountsServiceXml (const QDomDocument & doc) {
    
    bool valid = false;
    QDomElement docElem = doc.documentElement();
    
    QDomNode node = docElem.firstChild();
    while (!node.isNull()) {
        docElem = node.toElement();
        
        if (docElem.tagName() == "webService") {
            valid = initFromDefinition (docElem);
            break;
        }
        
        node = node.nextSibling();
    }

    return valid;
}

// This doesn't work with current Accounts
Accounts::Service * ServicePrivate::resolveAccountsService() {
    if (m_account == 0) {
        qCritical() << "Missing account connection, can't load post options";
        return 0;
    }
    
    Accounts::Account * aAcc = m_account->accountsObject();
    if (aAcc == 0) {
        qCritical() << "Failed to receive Accounts' account";
        return 0;
    }
    
    Accounts::Service * service = aAcc->selectedService();
    if (service == 0) {
        qCritical() << "Failed to receive Accounts' service";
        return 0;
    }
    
    return service;
}

bool ServicePrivate::loadServiceOptions () {
    if (m_serviceOptionsLoaded) {
        return true;
    }
    
    if (m_accServiceName.isEmpty()) {
        qWarning() << "No Accounts' service connection";
        return false;
    }

    Accounts::Manager aManager ("sharing");
    Accounts::Service * accountsService = aManager.service (m_accServiceName);
    
    if (accountsService == 0) {
        qWarning() << "Failed to load service" << m_accServiceName;    
        return false;
    }
    
    QDomDocument serviceXML = accountsService->domDocument ();
    QDomNodeList optionsNodeList = serviceXML.elementsByTagName ("postOptions");
    
    for (int i = 0; i < optionsNodeList.size(); ++i) {
        QDomNode node = optionsNodeList.at (i);
        if (!node.isElement()) {
            continue;
        }
        QDomElement element = node.toElement();
        initOptions (element);
        break;
    }
    
    m_serviceOptionsLoaded = true;
    return true;
}

void ServicePrivate::initOptions (const QDomElement & element) {
    QDomNode optionNode = element.firstChild();
    while (!optionNode.isNull()) {
        QDomElement optElem = optionNode.toElement();
        ServiceOption * option = new ServiceOption (m_service);
        if (!option->init (optElem)) {
            qWarning() << "Failed to load post option";
            delete option;
        } else {
            addOption (option);
        }
        optionNode = optionNode.nextSibling();
    }

}

void ServicePrivate::clearOptions () {
    while (!m_postOptions.isEmpty ()) {
        PostOption * option = m_postOptions.takeFirst (); 
        delete option;
    }
}

bool ServicePrivate::hasMimePrefix (const QString & prefix) const {

    // Empty list == accepts all
    if (m_mimeTypes.isEmpty() == true) {
        return true;
    }
    
    for (int i = 0; i < m_mimeTypes.count(); ++i) {
    
        //Starts with star, accepts all
        if (m_mimeTypes.at(i).startsWith ("*") == true) {
            return true;
            
        //TODO: Doesn't support format "imag*" (can be documented as invalid
        //mimetype definition option in service xml files)
        } else if (m_mimeTypes.at(i).startsWith (prefix) == true) {
            return true;
        }
    }
    
    return false;
}
