 
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

#ifndef _WEBUPLOAD_SERVICE_OPTION_PRIVATE_H_
#define _WEBUPLOAD_SERVICE_OPTION_PRIVATE_H_

#include <QList>
#include <QString>
#include "WebUpload/ServiceOption"
#include "WebUpload/ServiceOptionSupport"

namespace WebUpload {

    class Account;
    class ServiceOptionValue;

    class ServiceOptionPrivate {
    
    public:

        ServiceOptionPrivate (ServiceOption * parent);
        ~ServiceOptionPrivate ();
        
        /*!
          \brief Read update settings from XML
          \return <code>true</code> if the settings could be got from XML and
                  the m_updateSettings instance was created successfully, else
                  <code>false</code>
         */
        bool readUpdateSettings (QDomElement & element);

        /*!
          \brief Read add value settings from XML
          \return <code>true</code> if the settings could be got from XML and
                  the m_addSettings instance was created successfully, else
                  <code>false</code>
         */
        bool readAddValueSettings (QDomElement & element);

        /*!
          \brief TODO
         */
        bool setActiveValueId (const QString & value);

        /*!
          \brief TODO
         */
        bool setDefaultValueId (const QString & value);
        
        /*!
          \brief Tries to get currect active value from account
         */
        void activeValue ();
        
        /*!
          \brief Clear values
         */
        void clearValues();
        
        /*!
          \brief Get values from account if possible
         */
        void valueList ();
        
        /*!
          \brief Store values to account if possible
         */
        void storeValueList ();
        
        ServiceOption * m_parent; //!< Instance using private class

        QString m_id; //!< ID of the service option

        QString m_defaultValueId; //!< Id of default active value
        QString m_activeValueId; //!< Id of current active value
        QList<ServiceOptionValue> m_values; //!< Values related to option
        
        //! Default values read from the service xml file. These are used only
        //  if there are no updated values (i.e if update hasn't happened yet,
        //  or update happened and zero values were recieved
        QList<ServiceOptionValue> m_defaultValues;
        
        bool m_updatable; //!< If value is marked as updatable
        UpdateValueSettings *m_updateSettings; //!< Settings if updatable

        bool m_changeable; //!< If value is marked as changeable
        AddValueSettings *m_addSettings; //!< Settings for add value
    };
}

#endif /* #ifndef _WEBUPLOAD_SERVICE_OPTION_PRIVATE_H_ */
