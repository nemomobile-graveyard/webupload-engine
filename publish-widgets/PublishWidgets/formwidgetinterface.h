 
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

#ifndef _PUBLISH_WIDGETS_FORM_WIDGET_INTERFACE_H_
#define _PUBLISH_WIDGETS_FORM_WIDGET_INTERFACE_H_

#include "export.h"
#include <WebUpload/Entry>
#include <ShareWidgets/ApplicationViewInterface>

namespace PublishWidgets {

    /*!
      \class FormWidgetInterface
      \brief Class to define interface to FormWidgets
     */
    class PUBLISH_WIDGETS_EXPORT FormWidgetInterface {
    public:
    
        virtual ~FormWidgetInterface() {}
    
        /*!
          \brief Used to tell with what entry widget is used
          \param entry Entry modified in form
         */
        virtual void setEntry (WebUpload::Entry * entry) = 0;
       
        /*!
          \brief Used to tell in which application view form widget is used
          \param view View where widget is used
         */
        virtual void setApplicationView (
            ShareWidgets::ApplicationViewInterface * view) = 0;
        
        /*!
          \brief Commit changes to entry. This is called before entry is 
                 serialized and send to upload engine. Changes to UI that are
                 not yet stored to entry should be handled in this function
                 before returning.
         */
        virtual void commit () = 0;
    };
}

#endif
