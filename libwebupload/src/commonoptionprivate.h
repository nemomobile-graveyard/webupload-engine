 
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

#ifndef _WEBUPLOAD_COMMON_OPTION_PRIVATE_H_
#define _WEBUPLOAD_COMMON_OPTION_PRIVATE_H_

#include "WebUpload/PostOption"
#include <QStringList>

namespace WebUpload {

    class CommonOption;

    class CommonOptionPrivate {

    public:

        CommonOptionPrivate (CommonOption * parent);
        ~CommonOptionPrivate ();

        static PostOption::Type getType (QString tagName);
        QStringList supportedMimes (QDomElement & elem) const;
        QString captionValue (QDomElement & elem, PostOption::Type type) const;
        static QString defaultCaption (PostOption::Type type);

        CommonOption * m_parent; //!< Parent pointer
        bool m_isShown; //!< Should option be shown to user
    };
}

#endif // #ifndef _WEBUPLOAD_COMMON_OPTION_PRIVATE_H_
