 
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

#include "WebUpload/CommonOption"
#include "commonoptionprivate.h"
#include <QDomNodeList>
#include <limits.h>
#include <QDebug>
#include "xmlhelper.h"
#include "WebUpload/CommonTextOption"
#include "WebUpload/CommonListOption"
#include "WebUpload/Media"
#include "WebUpload/Entry"

using namespace WebUpload;

CommonOption::CommonOption (QObject * parent) : PostOption (parent),
    d_ptr (new CommonOptionPrivate (this)) {
}

CommonOption::~CommonOption () {
    delete d_ptr;
}

bool CommonOption::init (QDomElement & element) {

    PostOption::Type optionType = d_ptr->getType (element.tagName ());
    if (optionType >= PostOption::OPTION_TYPE_COMMON_TYPES_N) {
        qDebug () << "Service: skip invalid tag" << element.tagName ();
        return false;
    }

    setType (optionType);

    d_ptr->m_isShown = !XmlHelper::attributeValueToBool (element, "hide",
        false);
    
    if (d_ptr->m_isShown == false) {
        qDebug() << "Option" << optionType << "is hidden";
        return true;
    }

    QString caption = d_ptr->captionValue (element, optionType);
    setCaption (caption);

    setSupportedMimeTypes (d_ptr->supportedMimes (element));

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

    return true;
}

bool CommonOption::init (PostOption::Type optionType, QString caption) {
    
    if (optionType >= PostOption::OPTION_TYPE_COMMON_TYPES_N) {
        qDebug () << "CommonOption class can be created only for common "
            "option types, not" << optionType;
        return false;
    }
    
    if (caption.isEmpty () == true) {
        caption = d_ptr->defaultCaption (optionType);
    }
    
    return PostOption::init (optionType, caption);
}

bool CommonOption::validForMedia (const Media * media) const {

    if (media == 0 || d_ptr->m_isShown == false) {
        return false;
    }

    switch (type ()) {
        case PostOption::OPTION_TYPE_TITLE:
        case PostOption::OPTION_TYPE_DESC:
        case PostOption::OPTION_TYPE_TAGS:
            break;

        default:
            return false;
    }

    return validForMimeType (media->mimeType ());
}

bool CommonOption::validForEntry (const Entry * entry) const {

    if (entry == 0 || d_ptr->m_isShown == false) {
        return false;
    }



    switch (type ()) {
        case PostOption::OPTION_TYPE_TAGS:
        case PostOption::OPTION_TYPE_METADATA:
        case PostOption::OPTION_TYPE_IMAGE_RESIZE:
            break;

        case PostOption::OPTION_TYPE_VIDEO_RESIZE:
        // TODO: Fix when video resizing is supported. For now, adding this
        // as work-around for bug 205915
            return false;

        default:
            return false;
    }

    bool ret = false;
    QStringList mimes = entry->mimeTypes ();
    
    for (int i = 0; ret == false && i < mimes.count(); ++i) {
        ret = validForMimeType (mimes[i]);
    }

    return ret;
}

CommonOption * CommonOption::getCommonOption (QDomElement & element, 
    QObject * parent) {

    CommonOption * ret = 0;
    PostOption::Type type = CommonOptionPrivate::getType (element.tagName ());

    if (type == PostOption::OPTION_TYPE_TITLE || 
        type == PostOption::OPTION_TYPE_DESC) {

        CommonTextOption * option = new CommonTextOption (parent);
        if (option->init (element) == false) {
            delete option;
        } else {
            ret = option;
        }
    } else if (type == PostOption::OPTION_TYPE_TAGS) {

        ret = new CommonOption (parent);
        if (ret->init (element) == false) {
            delete ret;
            ret = 0;
        }
    } else {
        CommonListOption * option = new CommonListOption (parent);
        if (option->init (element) == false) {
            delete option;
        } else {
            ret = option;
        }
    }

    return ret;
}


CommonOption * CommonOption::getCommonOption (PostOption::Type type,
    QString caption, QObject * parent) {

    CommonOption * ret = 0;

    if (type == PostOption::OPTION_TYPE_TITLE || 
        type == PostOption::OPTION_TYPE_DESC) {

        CommonTextOption * option = new CommonTextOption (parent);
        if (option->init (type, caption) == false) {
            delete option;
        } else {
            ret = option;
        }
    } else if (type == PostOption::OPTION_TYPE_TAGS) {

        ret = new CommonOption (parent);
        if (ret->init (type, caption) == false) {
            delete ret;
            ret = 0;
        }
    } else {

        CommonListOption * option = new CommonListOption (parent);
        if (option->init (type, caption) == false) {
            delete option;
        } else {
            ret = option;
        }
    }

    return ret;
}


QString CommonOption::defaultCaption (PostOption::Type type) const {
    return d_ptr->defaultCaption (type);
}

/* --- Private functions ---------------------------------------------------- */

CommonOptionPrivate::CommonOptionPrivate (CommonOption * parent) : 
    m_parent (parent), m_isShown (true) {

}

CommonOptionPrivate::~CommonOptionPrivate () {
}

PostOption::Type CommonOptionPrivate::getType (QString tagName) {

    PostOption::Type retType = PostOption::OPTION_TYPE_COMMON_TYPES_N;

    if (tagName == "title") {
        retType = PostOption::OPTION_TYPE_TITLE;
    } else if (tagName == "description") {
        retType = PostOption::OPTION_TYPE_DESC;
    } else if (tagName == "tags") {
        retType = PostOption::OPTION_TYPE_TAGS;
    } else if (tagName == "metadata") {
        retType = PostOption::OPTION_TYPE_METADATA;
    } else if (tagName == "image_resize") {
        retType = PostOption::OPTION_TYPE_IMAGE_RESIZE;
    } else if (tagName == "video_resize") {
        retType = PostOption::OPTION_TYPE_VIDEO_RESIZE;
    }

    return retType;
}

QStringList CommonOptionPrivate::supportedMimes (QDomElement & elem) const {

    QDomNodeList nodes = elem.elementsByTagName ("formats");
    if (nodes.count() > 0) {
        return XmlHelper::readMimeTypes (nodes.at(0).toElement());
    }

    return QStringList ();
}

QString CommonOptionPrivate::captionValue (QDomElement & elem,
    PostOption::Type type) const {

    QDomNodeList nodes = elem.elementsByTagName ("caption");
    if (nodes.count() > 0) {
        return XmlHelper::getLocalizatedContent (nodes.at(0).toElement());
    }
    
    return defaultCaption (type);
}

QString CommonOptionPrivate::defaultCaption (PostOption::Type type) {
    QString caption;
    switch (type) {
        case PostOption::OPTION_TYPE_TITLE:
            //% "Title"
            caption = qtTrId ("qtn_comm_share_title");
            break;

        case PostOption::OPTION_TYPE_DESC:
            //% "Description"
            caption = qtTrId ("qtn_comm_share_desc");
            break;

        case PostOption::OPTION_TYPE_TAGS:
            //% "Tags"
            caption = qtTrId ("qtn_comm_share_tags");
            break;

        case PostOption::OPTION_TYPE_METADATA:
            //% "Metadata"
            caption = qtTrId ("qtn_tui_share_metadata");
            break;

        case PostOption::OPTION_TYPE_IMAGE_RESIZE:
            //% "Image size"
            caption = qtTrId ("qtn_tui_share_resize_image");
            break;

        case PostOption::OPTION_TYPE_VIDEO_RESIZE:
            //% "Video size"
            caption = qtTrId ("qtn_tui_share_resize_video");
            break;

        default:
            break;
    }

    return caption;
}
