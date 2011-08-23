
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

#ifndef _SERVICE_OPTION_SUPPORT_PRIVATE_H_
#define _SERVICE_OPTION_SUPPORT_PRIVATE_H_

#include <QString>

namespace WebUpload {

    class ServiceOption;

    /*!
      \class UpdateValueSettingsPrivate
      \brief Private class for WebUpload::UpdateValueSettings
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class UpdateValueSettingsPrivate {
    public:
        //! String to show in the button in the view presenting the service
        // option values, clicking on which starts the update process
        QString m_command;
    };


    /*!
      \class UpdateValueSettingsPrivate
      \brief Private class for WebUpload::UpdateValueSettings
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT AddValueSettingsPrivate {
    public:
        //! String to show in the button in the view presenting the service
        // option values, clicking on which opens the add dialog
        QString m_command;

        QString m_caption; //!< Caption to be shown in the add dialog
        QString m_tooltip; //!< Tooltip shown in the text entry widget in the add dialog
        QString m_note; //!< Any additional note that might need to be shown
        int m_maxLength; //!< Maximum lenght of the text entry widget

        //! String to be shown in the button in the add dialog, clicking on
        // which starts the add process
        QString m_addButton;

        //! Service options related to add settings, for example privacy
        QList<ServiceOption *> m_serviceOptions;
    };
}

#endif
