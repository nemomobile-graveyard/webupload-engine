 
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

#ifndef _PUBLISH_WIDGETS_FORM_WIDGET_PROVIDER_INTERFACE_H_
#define _PUBLISH_WIDGETS_FORM_WIDGET_PROVIDER_INTERFACE_H_

namespace PublishWidgets {

    // Forward declaration
    class FormWidgetInterface;

    /*!
      \class FormWidgetProviderInterface
      \brief Plugins providing custom widget will use this class as the plugin
             interface.
     */
    class FormWidgetProviderInterface {
    public:
    
        virtual ~FormWidgetProviderInterface() {}

        /*!
          \brief Get the custom widget
          \return Pointer to custom widget. Each call to this function will
                  return a new pointer
         */
        virtual FormWidgetInterface *getCustomWidget () = 0;
    };
}

Q_DECLARE_INTERFACE(PublishWidgets::FormWidgetProviderInterface,
    "com.meego.publish-widgets.form-widget-provider-interface/0.2");

#endif
