 
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

#ifndef _WEBUPLOAD_ERROR_PRIVATE_H_ 
#define _WEBUPLOAD_ERROR_PRIVATE_H_ 

#include <QString>
#include "WebUpload/Error"
#include <QVariant>
#include <QByteArray>

namespace WebUpload {
    class ErrorPrivate {
    
    public:
        ErrorPrivate ();
        
        ErrorPrivate (Error::Code iCode, const QString & iTitle,
            const QString & iDescription, const QString & iRecoverMsg = "");
            
        ErrorPrivate (const ErrorPrivate * src);

        ErrorPrivate (QByteArray & byteStream);

        QByteArray serialize () const;
        
        void clear ();
    
        Error::Code m_code; //!< Error code
        QString m_title; //!< Title of error
        QString m_description; //!< Longer description of error
        QString m_recoverMsg; //!< Recovery message(?)
        bool m_repairable; //!< If repair option should we offered
        bool m_canContinue; //!< If upload can continue with other files

        QString m_accountName; //!< Name of account for which this error is raised

        unsigned int m_count; //!< Count of failed files impacted by this error
        unsigned int m_totalCount; //!< Total count of files in the transfer
        
        //! The service can use this to put any additional information it wants
        // about the error. This would be specially useful when using custom
        // errors, with recovery support
        QVariant m_data;
                      
    };
}

#endif
