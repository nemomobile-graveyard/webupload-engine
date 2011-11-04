 
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

#ifndef _WEBUPLOAD_COMMON_SWITCH_OPTION_PRIVATE_H_
#define _WEBUPLOAD_COMMON_SWITCH_OPTION_PRIVATE_H_

#include <QString>

namespace WebUpload {

    class CommonSwitchOption;

    class CommonSwitchOptionPrivate {

    public:

        CommonSwitchOptionPrivate (CommonSwitchOption * parent);
        virtual ~CommonSwitchOptionPrivate ();

        bool initFaceTags(bool defaultChecked = false, const QString &note = "");

        CommonSwitchOption * m_parent; //!< Parent pointer

        bool m_checked; //!< Checked state of the option

        QString m_note;
    };
}

#endif
