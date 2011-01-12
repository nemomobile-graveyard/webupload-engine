 
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

#ifndef _WEBUPLOAD_SERVICE_OPTION_SUPPORT_H_
#define _WEBUPLOAD_SERVICE_OPTION_SUPPORT_H_

#include "WebUpload/export.h"

#include <QString>

namespace WebUpload {

    class UpdateValueSettingsPrivate;
    class AddValueSettingsPrivate;

    /*!
        \class UpdateValueSettings
        \brief This class presents the settings required to present the
               update value option to the user for a given service option. A
               service option that is updatable should contain an instance of
               this class
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT UpdateValueSettings {
        
    public:
        /*!
          \brief Constructor
          \param command String to be displayed for the update command
         */
        UpdateValueSettings (const QString & command);

        //! \brief Destructor
        virtual ~UpdateValueSettings ();

        /*!
          \brief Set the string to be shown for the update command
          \param command String to be displayed for the update command
         */
        void setCommand (const QString & command);

        /*!
          \brief Read the string to be displayed for the update command
          \return String to be displayed
         */
        const QString & command () const;

    private:
        UpdateValueSettingsPrivate * const d_ptr;
    };

    /*!
        \class AddValueSettings
        \brief This class presents the settings required to present an add
               value option to the user for a given service option. A service
               option that is changeable should contain an instance of this
               class
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT AddValueSettings {
        
    public:
        //! \brief Constructor
        AddValueSettings ();

        //! \brief Destructor
        ~AddValueSettings ();

        /*!
          \brief Set the string to be shown for the add command
          \param command String to be displayed for the add command
         */
        void setCommand (const QString & command);

        /*!
          \brief Read the string to be displayed for the add command
          \return String to be displayed
         */
        const QString & command () const;

        /*!
          \brief Set the string to be shown as caption in add dialog
          \param caption String to be displayed as caption in add dialog
         */
        void setCaption (const QString & caption);

        /*!
          \brief Read the string to be displayed as caption in add dialog
          \return String to be displayed as caption in the add dialog
         */
        const QString & caption () const;

        /*!
          \brief Set the string to be shown as tooltip in add dialog
          \param tooltip String to be displayed as tooltip in add dialog
         */
        void setTooltip (const QString & tooltip);

        /*!
          \brief Read the string to be displayed as tooltip in add dialog
          \return String to be displayed as tooltip in add dialog
         */
        const QString & tooltip () const;

        /*!
          \brief Set the string to be shown as note in the add dialog
          \param note String to be displayed as note in add dialog
         */
        void setNote (const QString & note);

        /*!
          \brief Read the string to be displayed as note in add dialog
          \return String to be displayed
         */
        const QString & note () const;

        /*!
          \brief Set the maximum input length for the text widget in the add
                 dialog
          \param maxLength Maximum input length for the text widget
         */
        void setMaxLength (int maxLength);

        /*!
          \brief Get the maximum length of string that the user can enter in
                 the add dialog's text widget. If this value is 0 (or
                 negative), then it means that there is no length restriction
                 on the input string
          \return Input length restriction on the new value to be added
         */
        int maxLength () const;

        /*!
          \brief Set the string to be shown in the add dialog's button
          \param addButton String to be displayed in the button of the add
                    dialog, clicking on which triggers the adding process
         */
        void setAddButton (const QString & addButton);

        /*!
          \brief Read the string to be displayed on the add button of the add
                 dialog
          \return String to be displayed
         */
        const QString & addButton () const;

    private:
        AddValueSettingsPrivate * const d_ptr;
    };
}

#endif