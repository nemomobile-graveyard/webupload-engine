 
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

#ifndef _WEBUPLOAD_POST_OPTION_PRIVATE_H_
#define _WEBUPLOAD_POST_OPTION_PRIVATE_H_

#include <QList>
#include <QString>

namespace WebUpload {

    class Account;
    class PostOption;

    class PostOptionPrivate {
    
    public:

        PostOptionPrivate (PostOption * parent);
        ~PostOptionPrivate ();
        
        PostOption * m_parent; //!< Instance using private class

        QString m_caption; //!< Name of option shown to user
        QStringList m_mimeTypes; //!< List of mimetype or empty if all accepted
        int m_order; //!< Display order of the option

        enum PostOption::Type m_optionType; //!< Type of the option
    
        //! Relation to account - this might not be used by all the options
        Account * m_account; 
    };
}

#endif /* #ifndef _WEBUPLOAD_POST_OPTION_PRIVATE_H_ */
