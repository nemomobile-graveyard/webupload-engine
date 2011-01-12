 
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

#ifndef _WEBUPLOAD_PLUGIN_APPLICATION_H_ 
#define _WEBUPLOAD_PLUGIN_APPLICATION_H_ 

#include <WebUpload/export.h>
#include <QCoreApplication>
#include <WebUpload/PluginInterface>

namespace WebUpload {

    class PluginApplicationPrivate;

    /*!
       \class PluginApplication
       \brief Application wrapper for plugins. Will provide communication layer
              between upload engine process and PluginInterface object.
       \author  Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT PluginApplication : public QCoreApplication {
    
    Q_OBJECT
    
    public:
    
        /*!
          \brief Constructor
          \param interface Interface object to be used
          \param argc Start up arguments count
          \param argv Start up arguments
         */
        PluginApplication (PluginInterface * interface, int & argc,
            char ** argv);
    
        /*!
          \brief  Virtual destructor
         */
        virtual ~PluginApplication();
        
    private:
        Q_DISABLE_COPY(PluginApplication)
        PluginApplicationPrivate * const d_ptr; //!< Private data

    };
}

#define PLUGIN_MAIN_FUNCTION(interfaceClass) \
int main (int argc, char ** argv) { \
    WebUpload::PluginApplication app (new interfaceClass(), argc, argv); \
    return app.exec(); \
}

#endif