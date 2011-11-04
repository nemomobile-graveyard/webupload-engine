 
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

#include <QDebug>
#include "xmlhelper.h"
#include "WebUpload/CommonSwitchOption"
#include "commonswitchoptionprivate.h"
#include "WebUpload/enums.h"
#include "WebUpload/Account"

using namespace WebUpload;

CommonSwitchOption::CommonSwitchOption (QObject * parent) : CommonOption (parent),
    d_ptr (new CommonSwitchOptionPrivate (this)) {

}

CommonSwitchOption::~CommonSwitchOption () {

    delete d_ptr;
}

bool CommonSwitchOption::init (QDomElement & element) {

    if (!CommonOption::init (element)) {
        return false;
    }

    bool initDone;
    switch (type()) {
        case PostOption::OPTION_TYPE_FACE_TAGS:
        {
            initDone = d_ptr->initFaceTags (
                XmlHelper::attributeValueToBool(element, "default", false),
                XmlHelper::getLocalizatedContent(element.firstChildElement("note")));

            QStringList mimeList;
            mimeList << "image/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        default:
            initDone = false;
    }

    return initDone;
}

bool CommonSwitchOption::init (PostOption::Type optionType, QString caption) {

    if (CommonOption::init (optionType, caption) == false) {
        return false;
    }

    bool initDone;
    switch (optionType) {
        case PostOption::OPTION_TYPE_FACE_TAGS:
        {
            initDone = d_ptr->initFaceTags ();

            QStringList mimeList;
            mimeList << "image/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        default:
            initDone = false;
    }

    return initDone;
}


bool CommonSwitchOption::isChecked () const {

    return d_ptr->m_checked;
}

void CommonSwitchOption::setChecked (bool checked) {

    d_ptr->m_checked = checked;
    switch (type()) {
        case PostOption::OPTION_TYPE_FACE_TAGS:
            account()->setValue("face-tags", checked);
            break;

        default:
            break;
    }
}

QString CommonSwitchOption::note() const {
    return d_ptr->m_note;
}


/* --- Private functions ---------------------------------------------------- */

CommonSwitchOptionPrivate::CommonSwitchOptionPrivate (CommonSwitchOption * parent) :
    m_parent (parent), m_checked(false) {

}

CommonSwitchOptionPrivate::~CommonSwitchOptionPrivate () {

}


bool CommonSwitchOptionPrivate::initFaceTags (bool defaultChecked,
    const QString& note) {

    m_checked = defaultChecked;
    m_note = note;

    if (m_parent->account () == 0) {
        qDebug() << "Account not defined";
        return false;
    }

    QVariant checked = m_parent->account()->value("face-tags");
    if (checked.isValid() && checked.canConvert(QVariant::Bool)) {
        m_checked = checked.toBool();
    }

    return true;
}
