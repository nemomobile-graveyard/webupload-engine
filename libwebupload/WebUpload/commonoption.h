 
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

#ifndef _WEBUPLOAD_COMMON_OPTION_H_
#define _WEBUPLOAD_COMMON_OPTION_H_

#include "WebUpload/export.h"
#include "WebUpload/PostOption"

namespace WebUpload {

    // Forward declaration of required classes
    class CommonOptionPrivate;
    
    /*!
        \class CommonOption
        \brief Class representing the common post options provided.
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT CommonOption : public PostOption {    
        Q_OBJECT
        
    public:
        /*!
           \brief Constructor
           \param parent QObject parent
         */
        CommonOption (QObject * parent = 0);
        
        virtual ~CommonOption();
        
        //! \reimp
        virtual bool init (QDomElement & element);
        virtual bool init (PostOption::Type optionType, QString caption);
        virtual bool validForMedia (const Media * media) const;
        virtual bool validForEntry (const Entry * entry) const;
        //! \reimp_end
        
        /*!
          \brief
          \param element
          \param parent
          \return
         */
        static CommonOption * getCommonOption (QDomElement & element,
            QObject * parent = 0);

        /*!
          \brief
          \param optionType
          \param caption
          \param parent
          \return
         */
        static CommonOption * getCommonOption (PostOption::Type type,
            QString caption = QString (), QObject * parent = 0);

    protected:

        QString defaultCaption (PostOption::Type type) const;

    private:
    
        CommonOptionPrivate * const d_ptr; //!< Private data

#ifdef TESTING
        friend class LibWebUploadTests; //!< For unit tests
#endif
           
    };
}

#endif
