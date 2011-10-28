 
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

#ifndef _WEBUPLOAD_PLUGIN_BASE_H_ 
#define _WEBUPLOAD_PLUGIN_BASE_H_ 

#include <WebUpload/export.h>
#include <WebUpload/PluginInterface>

namespace WebUpload {

    /*!
       \class PluginBase
       \brief Base plugin class implementing dummy interface functions which then
              can be replaced by actual plug-in classes.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT PluginBase : public QObject, public PluginInterface {
        Q_OBJECT
        Q_INTERFACES(WebUpload::PluginInterface)
        
    public:

        /*!
          \brief Create new plugin base object
          \param parent QObject parent
         */
        PluginBase (QObject *parent = 0);

        virtual ~PluginBase ();
        
        /*!
          \brief See PluginInterface. In this base implementation does
                 not do anything and always returns true.
         */
        virtual bool init();

        /*!
          \brief See PluginInterface. In this base implementation does
                 not do anything
         */
        virtual void deinit();
        
        /*!
          \brief See PluginInterface. Will return 0 in this base
                 implementation if function isn't implemented in inheriting
                 classes.
         */        
        virtual PostInterface * getPost();

        /*!
          \brief See PluginInterface. Will always return null in this
                 base implementation if function isn't implemented in inheriting
                 classes.
         */        
        virtual UpdateInterface * getUpdate();        
        
    };
}

#endif
