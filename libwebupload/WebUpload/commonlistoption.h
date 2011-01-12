 
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

#ifndef _WEBUPLOAD_COMMON_LIST_OPTION_H_
#define _WEBUPLOAD_COMMON_LIST_OPTION_H_

#include "WebUpload/export.h"
#include "WebUpload/CommonOption"

namespace WebUpload {

    // Forward declaration of required classes
    class CommonListOptionPrivate;
    
    /*!
        \class CommonListOption
        \brief Class representing the common post options that have a fixed
               list of values
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT CommonListOption : public CommonOption {    
        Q_OBJECT
        
    public:
        /*!
           \brief Constructor
           \param parent QObject parent
         */
        CommonListOption (QObject * parent = 0);
        
        virtual ~CommonListOption();
        
        /*! \reimp */
        virtual bool init (QDomElement & element);
        virtual bool init (PostOption::Type optionType, QString caption);
        /*! \reimp_end */

        /*!
          \brief Function to get list of valid values for common options that
                 have fixed set of values. 
          \return Stringlist with localized strings for the values
         */
        QStringList valueList () const;

        /*!
          \brief Get the current value of the option
          \return Integer value of the enum representing the option value.
                  Enums can be found in WebUpload/enums.h
         */
        int currentValue () const;

        /*!
          \brief Get the index in the list for the current value
          \return Index of the current value in the list
         */
        int currentValueIndex () const;

        /*!
          \brief Set the index of the current value
          \param index Index of the new current value in the list
          \return <code>true</code> if the index was valid 
         */
        bool setCurrentValueIndex (int index);
        
    private:
    
        CommonListOptionPrivate * const d_ptr; //!< Private data
           
    };
}

#endif