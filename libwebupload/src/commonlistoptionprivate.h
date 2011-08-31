 
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

#ifndef _WEBUPLOAD_COMMON_LIST_OPTION_PRIVATE_H_
#define _WEBUPLOAD_COMMON_LIST_OPTION_PRIVATE_H_

#include <QStringList>

namespace WebUpload {

    class CommonListOption;

    class CommonListOptionPrivate {

    public:

        CommonListOptionPrivate (CommonListOption * parent);
        virtual ~CommonListOptionPrivate ();

        bool initMetadata ();
        bool initImageResize (int defaultLimit = 0);
        bool initVideoResize ();

        QStringList valueList () const;

        CommonListOption * m_parent; //!< Parent pointer

        struct OptionValue {
            QString valueId; //!< String id for the option value
            int value; //!< Enumeration corresponding to given string id
        };

        //! List of values for options with fixed values
        QList <struct OptionValue *> m_valueList;
        int m_currentValueIndex; //!< Index of the current value in the list
    };
}

#endif // #ifndef _WEBUPLOAD_COMMON_LIST_OPTION_PRIVATE_H_
