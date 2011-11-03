
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

#ifndef _PUBLISH_WIDGETS_FORM_LOADER_H_
#define _PUBLISH_WIDGETS_FORM_LOADER_H_

#include "export.h"
#include <WebUpload/Entry>
#include <PublishWidgets/FormWidgetInterface>

namespace PublishWidgets {

    class FormLoaderPrivate;

    /*!
      \class FormLoader
      \brief Plugin loader for publish implementations
     */
    class PUBLISH_WIDGETS_EXPORT FormLoader : public QObject {
    Q_OBJECT
    public:
    
        /*!
          \brief Constructor
          \param parent QObject parent
         */
        FormLoader (QObject * parent = 0);
        virtual ~FormLoader();
     
        /*!
          \brief Get form widget to be used in UI
          \param pluginName Name of plugin used
          \param entry Entry given to plugin
          \return Pointer to form widget if loaded successfully
         */
        FormWidgetInterface * formWidget (const QString & pluginName,
            WebUpload::Entry * entry);
            
    private:
        Q_DISABLE_COPY (FormLoader)
        FormLoaderPrivate * const d_ptr; //!< Private data            
        
    };
}

#endif
