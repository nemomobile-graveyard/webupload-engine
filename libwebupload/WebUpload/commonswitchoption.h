 
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

#ifndef _WEBUPLOAD_COMMON_SWITCH_OPTION_H_
#define _WEBUPLOAD_COMMON_SWITCH_OPTION_H_

#include "WebUpload/export.h"
#include "WebUpload/CommonOption"

namespace WebUpload {

// Forward declaration of required classes
class CommonSwitchOptionPrivate;

/*!
    \class CommonSwitchOption
    \brief Class representing the common on/off post options
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class WEBUPLOAD_EXPORT CommonSwitchOption : public CommonOption {
    Q_OBJECT

public:
    /*!
       \brief Constructor
       \param parent QObject parent
     */
    CommonSwitchOption (QObject * parent = 0);

    virtual ~CommonSwitchOption();

    /*! \reimp */
    virtual bool init (QDomElement & element);
    virtual bool init (PostOption::Type optionType, QString caption);
    /*! \reimp_end */

    /*!
      \brief Returns true if option is checked
      \return <code>true</code> if the option is checked
     */
    bool isChecked () const;

public Q_SLOTS:

    /*!
      \brief Sets the checked state of the option
      \param checked New checked state
     */
    void setChecked (bool checked);

private:

    CommonSwitchOptionPrivate * const d_ptr; //!< Private data

};

}

#endif
