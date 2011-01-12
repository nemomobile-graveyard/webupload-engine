 
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

#include <QDebug>
#include "xmlhelper.h"
#include "WebUpload/CommonTextOption"

using namespace WebUpload;

CommonTextOption::CommonTextOption (QObject * parent) : CommonOption (parent),
    m_isMultiline (false) {

}

CommonTextOption::~CommonTextOption () {

}

bool CommonTextOption::init (QDomElement & element) {

    if (!CommonOption::init (element)) {
        return false;
    }

    bool defMultilineValue;

    PostOption::Type optType = type ();
    if (optType == PostOption::OPTION_TYPE_DESC) {
        defMultilineValue = true;
    } else if (optType == PostOption::OPTION_TYPE_TITLE) {
        defMultilineValue = false;
    } else {
        return false;
    }
    m_isMultiline = XmlHelper::attributeValueToBool (element, "multiline",
        defMultilineValue);

    QDomNodeList nodes = element.elementsByTagName ("tooltip");
    if (nodes.count () > 0) {
        m_tooltip = XmlHelper::getLocalizatedContent (nodes.at(0).toElement());
    }

    if (m_tooltip.isEmpty ()) {
        m_tooltip = defaultTooltip ();
    }

    return true;
}

bool CommonTextOption::init (PostOption::Type optionType, QString caption) {

    if (!CommonOption::init (optionType, caption)) {
        return false;
    }

    if (optionType == PostOption::OPTION_TYPE_DESC) {
        m_isMultiline = true;
    } else if (optionType == PostOption::OPTION_TYPE_TITLE) {
        m_isMultiline = false;
    } else {
        return false;
    }

    m_tooltip = defaultTooltip ();

    return true;
}


bool CommonTextOption::isMultiline () const {
    return m_isMultiline;
}

QString CommonTextOption::tooltip () const {
    return m_tooltip;
}

QString CommonTextOption::defaultTooltip () const {
    QString ttip;
    switch (type()) {
        case PostOption::OPTION_TYPE_TITLE:
            //% "Add title"
            ttip = qtTrId ("qtn_comm_share_title_tooltip");
            break;

        case PostOption::OPTION_TYPE_DESC:
            //% "Add description"
            ttip = qtTrId ("qtn_comm_share_desc_tooltip");
            break;

        default:
            break;
    }

    return ttip;
}