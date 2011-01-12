 
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

#ifndef _WEBUPLOAD_SERVICE_OPTION_VALUE_H_
#define _WEBUPLOAD_SERVICE_OPTION_VALUE_H_

#include <WebUpload/export.h>
#include <QString>

namespace WebUpload {

    class ServiceOptionValuePrivate;

    /*!
        \class ServiceOptionValue
        \brief Value of ServiceOption
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT ServiceOptionValue {

    public:

        static const QChar FIELD_SEPARATOR;

        /*!
          \brief Contructor
         */
        ServiceOptionValue();
        
        /*! 
           \brief Constructor
           \param id ID of value
           \param name Name of value               
         */
        ServiceOptionValue(const QString & id, const QString & name);
        
        /*!
          \brief Copy constructor
         */
        ServiceOptionValue (const ServiceOptionValue & src);
        
        /*!
          \brief Assignment operator
         */
        ServiceOptionValue &operator= (const ServiceOptionValue & src);        
        
        virtual ~ServiceOptionValue();
        
        /*! 
           \brief Get ID of value
           \return ID of value
         */
        const QString & id() const;

        /*! 
           \brief Get presentable name of value
           \return Name of value
         */
        const QString & name() const;

        /*! 
           \brief Set ID of value
           \param id New ID for value
         */
        void setId (const QString & id);

        /*! 
           \brief Set presentable name of value
           \param name New name for value
         */
        void setName (const QString & name);
        
        /*!
          \brief Check if value is empty without id or name (not valid)
          \return <code>true</code> if value is empty
         */
        bool isEmpty() const;
    
    private:
    
        ServiceOptionValuePrivate * const d_ptr; //!< private data
    };
}

#endif