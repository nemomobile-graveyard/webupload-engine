 
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

#ifndef _WEBUPLOAD_SERVICE_H_
#define _WEBUPLOAD_SERVICE_H_

#include "WebUpload/export.h"
#include <QObject>
#include <QString>
#include <QListIterator>
#include <QDomDocument>
#include "WebUpload/PostOption"
#include <Accounts/Service>

namespace WebUpload {

    class Account;
    class ServicePrivate;
    class ServiceOption;
    class CommonOption;
    
    /*!
        \class Service
        \brief Web service information
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT Service : public QObject {
        Q_OBJECT
    
    public:
    
        /*!
          \brief Create new service object
          \param parent Parent QObject
         */        
        Service (QObject *parent = 0);
        
        virtual ~Service();
        
        /*!
          \brief Initialize service from Accounts' service
          \param accService Accounts' service
          \return true if success
         */
        bool initFromAccountsService (Accounts::Service * accService);
        
        /*!
          \brief Get name of service
          \return Name of service
         */            
        QString name() const;
        
        /*!
          \brief Name for plugin process
          \return Name of plugin process
         */
        QString pluginProcessName() const;
                
        /*!
          \brief Does service accept this mime type
          \return true if mime type is accepted
         */            
        bool acceptMimeType (const QString & mime) const;
                
        /*! 
           \brief Get iterator to post options (both common and service
                  specific) for this service
           \return Iterator to post options. 
         */
        QListIterator<PostOption *> postOptions();

        /*!
          \brief Get the common option based on option type
                 No point supporting this for service options, since all of
                 them have the same option type, and hence it is better to use
                 the serviceOption function which gets a service option based
                 on the option id
          \param optionType Type of the required option
          \return Pointer to common option or null if not found, or if type is
                 not valid
         */
        CommonOption * commonOption (enum PostOption::Type optionType);
        
        /*!
          \brief Get the service option with id
                 This function is normally used when the option is
                 update-able/change-able, which is why it would be used only
                 for service options - common options would not be update-able
                 or change-able.
          \param id Service post option id
          \return Pointer to service post option or null if not found
         */
        ServiceOption * serviceOption (const QString & id);
        
        /*!
          \brief Refresh all update-able options. Will only work if service has
                 account parent. Without account function will not do anything.
         */
        void refreshPostOptions ();
                
        /*!
          \brief Has post options that can be updated
          \return true if service has updatable post options
         */
        bool hasUpdatablePostOptions();
        
        /*!
          \brief Returns account if it's marked as parent of service
          \return Parent account or null if there isn't account relation
         */
        Account * account();
        
        /*!
          \brief Name of service icon
          \return Icon name or empty string if icon not defined
         */
        QString iconName();     
        
        /*!
          \brief Check if service accepts some image formats
          \return <code>true</code> if service accepts some image formats
         */
        bool acceptsImages() const;
        
        /*!
          \brief Check if service accepts some video formats
          \return <code>true</code> if service accepts some video formats
         */        
        bool acceptsVideos() const;
        
        /*!
          \brief Customization of publish UI
         */
        enum PublishCustomLevel {
            PUBLISH_CUSTOM_TOTAL, //!< Do not present account at all in UI        
            PUBLISH_CUSTOM_XML, //!< Build UI from XML and present account
            PUBLISH_CUSTOM_PAGE, //!< Ask for page, but present account
            PUBLISH_CUSTOM_WIDGET, //!< Just ask for widget and present account
                        
            PUBLISH_CUSTOM_N //!< Last value do not use
        };
        
        /*!
          \brief Customization of publish views
          \return How service should be customizated
         */
        PublishCustomLevel publishCustom() const;
        
        /*!
          \brief Name of publish plugin. Check type of plugin from
                 publishCustom.
          \return Name of plugin or empty string if plugin not defined.
         */
        QString publishPlugin() const;
        
        /*!
          \brief Get maxium amount of media that single upload (entry) to this
                 service should contain.
          \return Number of media elements allowed. Or 0 if no limit defined.
         */
        unsigned int maxMediaLimit() const;
        
    private:
        Q_DISABLE_COPY(Service)
        ServicePrivate * const d_ptr; //!< Private data
    };
}

#endif
