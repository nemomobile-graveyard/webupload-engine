 
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

#ifndef _WEBUPLOAD_COMMON_TEXT_OPTION_H_
#define _WEBUPLOAD_COMMON_TEXT_OPTION_H_

#include <WebUpload/export.h>
#include <WebUpload/CommonOption>

namespace WebUpload {

    class CommonTextOptionPrivate;

    /*!
        \class CommonTextOption
        \brief Class representing the common text post options provided (i.e
               title and description)
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT CommonTextOption : public CommonOption {    
        Q_OBJECT
        
    public:
        /*!
           \brief Constructor
           \param parent QObject parent
         */
        CommonTextOption (QObject * parent = 0);
        
        virtual ~CommonTextOption();
        
        /*! \reimp */
        virtual bool init (QDomElement & element);
        virtual bool init (PostOption::Type optionType, QString caption);
        /*! \reimp_end */

        /*!
          \brief Function to check whether the text entry widget should be
                 multiline or not
          \return <code>true</code> if the text entry widget should be
                  multiline else <code>false</code>
         */
        bool isMultiline () const;

        /*!
          \brief Gets the localized tooltip for the text widget
          \return Localized tooltip string
         */
        QString tooltip () const;
        
        /*!
          \brief If UI should prefill option for user
          \return <code>true</code> if value should be autofilled
         */
        bool prefill() const;
        
    private:
    
        CommonTextOptionPrivate * const d_ptr; //!< private data
           
    };
}

#endif
