 
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

#include "WebUpload/ServiceOption"
#include "serviceoptionprivate.h"
#include "WebUpload/ServiceOptionValue"
#include <QDomDocument>
#include <QDebug>
#include <limits.h>
#include "xmlhelper.h"
#include "WebUpload/Account"
#include "WebUpload/Media"
#include "WebUpload/Entry"

using namespace WebUpload;

ServiceOption::ServiceOption(QObject * parent) : PostOption (parent),
    d_ptr (new ServiceOptionPrivate(this)) {
}

ServiceOption::~ServiceOption() {
    delete d_ptr;
}

bool ServiceOption::init (QDomElement & element) {
    if (element.tagName() != "option") {
        qWarning() << "Invalid tag name" << element.tagName();
        return false;
    }
    
    QString id = element.attribute ("id", "");
    if (id.isEmpty()) {
        qWarning() << "Missing id";
        return false;
    }

    d_ptr->m_id = id;
    
    QString type = element.attribute ("type", "static");
    d_ptr->m_updatable = (type == "updatable" || type == "changeable");
    d_ptr->m_changeable = (type == "changeable");

    QString orderStr = element.attribute ("order", "");
    int order = INT_MAX;
    if (!orderStr.isEmpty ()) {
        bool ok;
        order = orderStr.toInt (&ok, 10);
        if (!ok) {
            order = INT_MAX;
        }
    }
    setOrder (order);

    // If updatable try to read updated values from account
    if ((d_ptr->m_updatable) && (account () != 0)) {
        d_ptr->valueList ();
        d_ptr->activeValue ();
    }

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
    
        if (node.isElement() == false) {
            node = node.nextSibling();
            continue;
        }
    
        QDomElement childElem = node.toElement();
        
        if (childElem.tagName() == "value") {
            QString valueId = childElem.attribute ("id", "");
            if (!valueId.isEmpty()) {
                ServiceOptionValue value (valueId,
                    XmlHelper::getLocalizatedContent (childElem));
                d_ptr->m_defaultValues.append (value);
            }

        } else if (childElem.tagName() == "caption") {
            setCaption (XmlHelper::getLocalizatedContent (childElem));

        } else if (childElem.tagName() == "formats") {
            setSupportedMimeTypes (XmlHelper::readMimeTypes (childElem));
            
        } else if (childElem.tagName() == "add") {
            if (d_ptr->m_changeable == false) {
                qWarning() << "Ignored add element in unchangeable service option";
            } else if (!d_ptr->readAddValueSettings (childElem)) {
                qWarning() << "Problems reading add element";
            }

        } else if (childElem.tagName() == "update") {
            if (d_ptr->m_updatable == false) {
                qWarning() << "Ignored update element in non-updatable service option";
            } else if (!d_ptr->readUpdateSettings (childElem)) {
                qWarning() << "Problems reading update element";
            }

            /*
        } else if (childElem.tagName() == "input") {
            QString lenStr = childElem.attribute ("maxLength", "0");
            bool inputValid = false;
            int lenInt = lenStr.toInt (&inputValid, 10);
            if (inputValid == true) {
                d_ptr->m_maxLength = lenInt;
            }
            */

        } else {
            qWarning() << "Unknown tag" << childElem.tagName();
        }

        node = node.nextSibling();
    }
    
    // If the account is defined, get the last known active value for this
    // option from the account
    if (account () != 0) {
        d_ptr->activeValue ();
    }

    QString defValueId = element.attribute ("default", "");
    if (!d_ptr->setDefaultValueId (defValueId)) {
        qWarning() << "\tDefault value id" << id << "not found";
    }

    if (d_ptr->m_updatable == (d_ptr->m_updateSettings == 0)) {
        qWarning() << "Update settings not defined for updateable option";
        return false;
    } else if (d_ptr->m_changeable == (d_ptr->m_addSettings == 0)) {
        qWarning() << "Add settings not defined for changeable option";
        return false;
    }
    
    setType (PostOption::OPTION_TYPE_SERVICE);
    return true;
}

bool ServiceOption::init (enum PostOption::Type optionType, QString caption) {
    Q_UNUSED (optionType)
    Q_UNUSED (caption)
    qWarning() << "Service options can only be initialized from xml";
    return false;
}

bool ServiceOption::validForMedia (const Media * media) const {
    Q_UNUSED (media)

    // Service specific options will normally be defined for the entire
    // transfer and not for single items in the transfer
    return false;
}

bool ServiceOption::validForEntry (const Entry * entry) const {

    if (entry == 0) {
        return false;
    }

    // Service specific options will normally be defined for the entire
    // transfer and not for single items in the transfer. So need to only check
    // if the transfer has even one media of the current mime type
    
    bool ret = false;
    QStringList mimeList = entry->mimeTypes ();

    for (int i = 0; i < mimeList.size() && !ret; ++i) {
        ret = validForMimeType (mimeList [i]);
    }

    return ret;
}

QString ServiceOption::id() const {
    return d_ptr->m_id;
}

QString ServiceOption::activeValueId () const {
    return d_ptr->m_activeValueId;
}

int ServiceOption::activeValueIndex() const {    
    QList<ServiceOptionValue> & values = d_ptr->m_values;
    
    if (values.isEmpty()) {
        values = d_ptr->m_defaultValues;
    }

    for (int i = 0; i < values.size(); ++i) {
        if (values.at(i).id () == activeValueId()) {
            return i;
        }
    }
    
    return -1;
}

QString ServiceOption::defaultValueId () const {
    return d_ptr->m_defaultValueId;
}

void ServiceOption::updateValues (
    const QList<ServiceOptionValue> & optionValues, bool storeToAccount) {
    
    d_ptr->m_values.clear ();
    
    if (optionValues.size() == 0) {
        setActiveValueId (d_ptr->m_defaultValueId);
    } else {
        for (int i = 0; i < optionValues.count(); ++i) {
            d_ptr->m_values.append (optionValues.at(i));
        }
                
        if (activeValueIndex() == -1) {
            setActiveValueId (d_ptr->m_values[0].id());
        }

    }

    if (storeToAccount == true) {
        d_ptr->storeValueList ();
    }

    return;
}

bool ServiceOption::isUpdatable() const {
    return d_ptr->m_updatable;
}

const UpdateValueSettings * ServiceOption::getUpdateSettings () const {
    return d_ptr->m_updateSettings;
}

bool ServiceOption::isChangeable() const {
    return d_ptr->m_changeable;
}

const AddValueSettings * ServiceOption::getAddValueSettings () const {
    return d_ptr->m_addSettings;
}

QList<ServiceOptionValue> ServiceOption::values() const {
    if (d_ptr->m_values.size() == 0) {
        return d_ptr->m_defaultValues;
    } else {
        return d_ptr->m_values;
    }
}

int ServiceOption::valueCount() const {
    int retVal = d_ptr->m_values.size();
    if (retVal == 0) {
        retVal = d_ptr->m_defaultValues.size();
    }
    return retVal;
}

ServiceOptionValue ServiceOption::valueAt (int index) const {
    if (index >= valueCount()) {
        qWarning() << "Invalid value index:" << index;
        return ServiceOptionValue();
    } else if (d_ptr->m_values.isEmpty()) {
        return d_ptr->m_defaultValues.at (index);
    } else {
        return d_ptr->m_values.at (index);
    }
}

bool ServiceOption::setActiveValueId (const QString & valueId) {
    bool retVal = d_ptr->setActiveValueId (valueId);
    int index = activeValueIndex(); 

    if (index == -1) {
        retVal = false;
    } else {
        ServiceOptionValue value = valueAt (index);
        if (value.isEmpty()) {
            retVal = false;
        } else {
            retVal = true;
            Q_EMIT (activeValueChanged(id(), value.id()));

            if (account() != 0) {
                account()->syncValues();
            }
        }
    }

    return retVal;
}

void ServiceOption::refresh (bool sync) {
    if ((sync == true) && (account() != 0)) {
        account()->syncValues();
    }

    qDebug() << "Post option refreshed";
    d_ptr->valueList ();
    d_ptr->activeValue ();
}

/* --- Private class functions ---------------------------------------------- */
ServiceOptionPrivate::ServiceOptionPrivate (ServiceOption * parent) :
    m_parent (parent), m_updatable (false), m_updateSettings (0),
    m_changeable (false), m_addSettings (0), m_account (0) {

}

ServiceOptionPrivate::~ServiceOptionPrivate () {
    if (m_updateSettings) {
        delete m_updateSettings;
        m_updateSettings = 0;
    }

    if (m_addSettings) {
        delete m_addSettings;
        m_addSettings = 0;
    }
}

bool ServiceOptionPrivate::readUpdateSettings (QDomElement & element) {

    if (m_updateSettings) {
        qWarning () << "Update settings already present. Ignoring ...";
        return false;
    }

    QDomNode node = element.firstChild();

    while (!node.isNull()) {
    
        if (node.isElement() == false) {
            node = node.nextSibling();
            continue;
        }
    
        QDomElement childElem = node.toElement();

        if (childElem.tagName() == "command") {
            QString command = XmlHelper::getLocalizatedContent (childElem);
            if (!command.isEmpty ()) {
                m_updateSettings = new UpdateValueSettings (command);
                break;
            }
        }
        node = node.nextSibling();
    }
        
    return (m_updateSettings != 0);
}

bool ServiceOptionPrivate::readAddValueSettings (QDomElement & element) {

    if (m_addSettings) {
        qWarning () << "Add value settings already present. Ignoring ...";
        return false;
    }

    m_addSettings = new AddValueSettings ();

    QDomNode node = element.firstChild();

    while (!node.isNull()) {
    
        if (node.isElement() == false) {
            node = node.nextSibling ();
            continue;
        }
    
        QDomElement childElem = node.toElement();

        if (childElem.tagName() == "command") {
            m_addSettings->setCommand ( 
                XmlHelper::getLocalizatedContent (childElem));
        } else if (childElem.tagName () == "caption") {
            m_addSettings->setCaption (
                XmlHelper::getLocalizatedContent (childElem));
        } else if (childElem.tagName () == "tooltip") {
            m_addSettings->setTooltip ( 
                XmlHelper::getLocalizatedContent (childElem));
        } else if (childElem.tagName () == "note") {
            m_addSettings->setNote(
                XmlHelper::getLocalizatedContent (childElem));
        } else if (childElem.tagName () == "button") {
            m_addSettings->setAddButton (
                XmlHelper::getLocalizatedContent (childElem));
        } else if (childElem.tagName () == "input") {
            QString lenStr = childElem.attribute ("maxLength", "0");
            bool inputValid = false;
            int lenInt = lenStr.toInt (&inputValid, 10);
            if (inputValid == true) {
                m_addSettings->setMaxLength (lenInt);
            }
        }

        node = node.nextSibling ();
    }

    if (m_addSettings->command().isEmpty() || 
        m_addSettings->caption().isEmpty() || 
        m_addSettings->addButton().isEmpty()) {

        qWarning () << "Command, Caption and Add button string are mandatory elements in the add element";
        delete m_addSettings;
        m_addSettings = 0;
    }
        
    return (m_addSettings != 0);
}

void ServiceOptionPrivate::clearValues() {
    m_values.clear();
}

void ServiceOptionPrivate::activeValue () {
    Account * account = m_parent->account ();
    if (account != 0) {
        QString key = m_id;
        key.append ("/current");
        QString resVal = account->value (key).toString();
        if (!resVal.isEmpty()) {
            m_activeValueId = resVal;
            qDebug() << "active" << m_id << "value is:" << resVal;
        }
        //TODO: Validate that the active value is a valid value?
    } else {
        qWarning() << "Can't resolve active value without account relation";
    }
}

bool ServiceOptionPrivate::setActiveValueId (const QString & value) {
    Account * account = m_parent->account ();
    if (account != 0) {
        QString key = m_id;
        key.append ("/current");
        account->setValue (key, value);
    } else {
        qWarning() << "Can't write value change to account";
    }

    m_activeValueId = value;
    return true;
}

bool ServiceOptionPrivate::setDefaultValueId (const QString & value) {
    if (m_activeValueId.isEmpty()) {
        setActiveValueId (value);
    }
    m_defaultValueId = value;
    return true;
}

void ServiceOptionPrivate::valueList () {
    if (!m_updatable) {
        return;
    }
    
    Account * account = m_parent->account ();
    if (account == 0) {
        qWarning() << "Can't load value lists";
        return;
    }    
    
    QString key = m_id;
    key.append ("/ids");
    QStringList ids = account->value (key).toStringList();
        
    key = m_id;
    key.append ("/names");
    QStringList names = account->value (key).toStringList();
    
    if (ids.isEmpty() || names.count() != ids.count()) {
        qDebug() << "No updated option list found";
        return;
    }

    clearValues();    
    
    for (int i = 0; i < ids.count(); ++i) {
        ServiceOptionValue value (ids.at(i), names.at(i));
        m_values.append (value);
    }
}

void ServiceOptionPrivate::storeValueList () {
    Account * account = m_parent->account ();
    if (account == 0) {
        return;
    }

    QStringList ids;
    QStringList names;

    ServiceOptionValue value;
    foreach(value, m_values) {
        ids.append (value.id());
        names.append (value.name());        
    }
    
    QString key = m_id;
    key.append ("/ids");
    account->setValue (key, QVariant(ids));
        
    key = m_id;
    key.append ("/names");
    account->setValue (key, QVariant(names));
}
