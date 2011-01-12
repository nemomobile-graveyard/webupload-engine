
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

#ifndef _WEBUPLOAD_SERVICE_PRIVATE_H_
#define _WEBUPLOAD_SERVICE_PRIVATE_H_

#include <QString>
#include "WebUpload/PostOption"
#include "WebUpload/Service"
#include <Accounts/Manager>
#include <QList>

namespace WebUpload {

    class Service;
    class Account;

    class ServicePrivate {
    
    public:
    
        ServicePrivate (Service * parent);
        ~ServicePrivate ();
    
        void clear();
        bool validDataExists () const;
        void populatePresentationData (const QDomElement & element);
        void populateMediaData (const QDomElement & element);

        void addOption (PostOption * option);

        /*!
          \brief Read definition from our root node
          \param docElem Sharing service data's root element
          \return true if input is valid and read successfull
         */
        bool initFromDefinition (const QDomElement & element);
        
        /*!
          \brief Initialize service from Accounts' service XML document
          \param doc XML that contains sharing part
          \return true if success
         */
        bool initFromAccountsServiceXml (const QDomDocument & doc); 
        
        /*!
          \brief Loads post options
          \return true if options are loaded after this function
         */
        bool loadServiceOptions ();
        
        /*!
          \brief Init options from XML node
          \param element XML content used
         */
        void initOptions (const QDomElement & element);
        
        /*!
          \brief Clear all options from memory
         */
        void clearOptions ();
        
        /*!
          \brief Try to resolve Accounts' service related to this
          \return Accounts' service or null
         */
        Accounts::Service * resolveAccountsService();
        
        /*!
          \brief Finds caption tag under parentElem and localizes it's
                 content
          \param parentElem Element which should have caption tag
          \return Localizated string or empty string if not defined
         */                     
        static QString captionValue (QDomElement & parentElem);
        
        /*!
          \brief Filter function for public accepts functions
         */
        bool hasMimePrefix (const QString & prefix) const;
        
        QString m_name; //!< Name of service
        QString m_uploadPlugin; //!< Upload plugin used
        //! Contains map of option order against option
        QList<PostOption *> m_postOptions; 
        QStringList m_mimeTypes; //!< Mime types supported by service
        Service * m_service; //!< Owner of private class
        Account * m_account; //!< If account is parent then pointer is stored
        QString m_accServiceName; //!< Accounts' service name
        QString m_iconName; //!< Icon name            

        bool m_serviceOptionsLoaded; //!< Are service options loaded or not
        
        //! Publish custom level of service
        Service::PublishCustomLevel m_publishCustom;
        //! Name of publish plugin
        QString m_publishPlugin;
    };
}

#endif