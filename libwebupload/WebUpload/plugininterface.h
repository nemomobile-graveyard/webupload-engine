 
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

#ifndef _WEBUPLOAD_PLUGIN_INTERFACE_H_ 
#define _WEBUPLOAD_PLUGIN_INTERFACE_H_ 

#include <QtPlugin>
#include <WebUpload/PostInterface>
#include <WebUpload/UpdateInterface>

namespace WebUpload {
    /*!
       \class PluginInterface
       \brief This class defines the interfaces that need to be implemented by
              plugins for the upload engine.
       \author  Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class PluginInterface {
    
    public:
    
        /*!  \brief  Virtual destructor */
        virtual ~PluginInterface()  {}

        /*!
           \brief  Virtual initialization function. This function will do all
                   the initalizations that are required before the plugin can
                   be used.
           \return The return value is a boolean value signifying whether or
                   not the initialization could be done successfully.
         */
        virtual bool init() = 0;

        /*!
          \brief Virtual function returning a pointer to object implementing
                 WebUploadPostInterface interface. Class which is used to post
                 files to account/service. Each call of this will return new
                 instance of class. This object is marked as parent of object
                 returned.
          \return Pointer to object implementing WebUpload::PostInterface class
                  interfaces. Or null if plugin does not have post
                  functionality.
         */        
        virtual WebUpload::PostInterface *getPost() = 0;
        
        /*!
          \brief Virtual function returning a pointer to the
                 WebUpload::UpdateInterface object. Class which is used to
                 update settings of account. Each call of this will return new
                 instance of class. This object is marked as parent of object
                 returned.
          \return Pointer to WebUpload::UpdateInterface object. Or null if
                  plugin does not have update functionality.
         */
        virtual WebUpload::UpdateInterface * getUpdate() = 0;

        /*!
           \brief  This function de-initializes the service. It's activities
                   could include logging out of the service. Once this function
                   is called, no other function of this class, other than
                   destructor, should be called.
         */
        virtual void deinit() = 0;
    };
}

Q_DECLARE_INTERFACE(WebUpload::PluginInterface,
    "com.nokia.webupload.plugin/0.2");

#endif
