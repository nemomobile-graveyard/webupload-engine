 
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

#ifndef _WEBUPLOAD_SERVICE_OPTION_H_
#define _WEBUPLOAD_SERVICE_OPTION_H_

#include "WebUpload/export.h"
#include "WebUpload/PostOption"

namespace WebUpload {

    class ServiceOptionPrivate;
    class ServiceOptionValue;
    class UpdateValueSettings;
    class AddValueSettings;
    
    /*!
        \class ServiceOption
        \brief Class presents one service option used with service
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT ServiceOption : public PostOption {    
        Q_OBJECT
        
    public:
        /*!
           \brief Constructor
           \param parent QObject parent
         */
        ServiceOption (QObject * parent = 0);
        
        virtual ~ServiceOption();

        /*! \reimp */
        virtual bool init (QDomElement & element);
        virtual bool init (enum PostOption::Type optionType, QString caption);
        virtual bool validForMedia (const Media * media) const;
        virtual bool validForEntry (const Entry * entry) const;
        /*! \reimp_end */
        
        /*!
          \brief Get the id for the option
          \return ID for the option
         */
        QString id () const;
        
        /*! 
           \brief Get current active value id
           \return ID of active value of service option
         */
        QString activeValueId () const;            
        
        /*!
          \brief Get active value index
          \return Index of active value. -1 means that there isn't active
                  value.
         */
        int activeValueIndex() const;            
        
        /*!
          \brief Get default value id
          \return ID of default value or empty string
         */
        QString defaultValueId () const;          
        
        /*! 
          \brief If option is marked as updatable
          \return <code>true</code> if values can changes and update option
                  should be shown.
         */
        bool isUpdatable() const;

        /*!
          \brief Returns pointer to the structure containing the update
                 settings. This pointer should not be deleted by the calling
                 function - ownership of the instance pointed to by that
                 pointer still is with this class.
          \return Pointer to structure containing the update settings, or NULL
                  pointer if the option is not updatable
         */
        const UpdateValueSettings *getUpdateSettings () const;
        
        /*!
          \brief If user can add new values
          \return <code>true</code> if new values can be added
         */
        bool isChangeable () const;

        /*!
          \brief Returns pointer to the structure containing the add value
                 settings. This pointer should not be deleted by the calling
                 function - ownership of the instance pointed to by that
                 pointer still is with this class.
          \return Pointer to structure containing the add value settings, or
                 NULL pointer if the option is not updatable
         */
        const AddValueSettings *getAddValueSettings () const;
        
        /*! 
           \brief Get values of option
           \return List of values under service option
         */
        QList<ServiceOptionValue> values() const;            
        
        /*!
          \brief Get value at index
          \param index of value requested
          \return Value or empty value if index is out of range
         */
        ServiceOptionValue valueAt (int index) const;            
        
        /*!
          \brief How many values there is define in option
          \return Number of value items
         */
        int valueCount() const;

        /*!
          \brief Set updated list of values
          \param values New set of values
          \param storeToAccount Store changes to account. If <code>false</code>
                                change will be stored only to the object and so
                                so lost when reinitialized.
         */
        void updateValues (const QList<ServiceOptionValue> & values,
            bool storeToAccount = true);

        /*!
          \brief Refresh values from the storage. This is useful if values have
                 be changed by another process.
          \brief sync If true then sync to DB is done before reading again. This
                      is needed if sync is not done before and values might be
                      changed by another process.
         */
        void refresh (bool sync = true);
        
    Q_SIGNALS:
    
        /*!
          \brief Signal emitted when active value changes.
          \param optionId Id of the option
          \param optionValueId Id of the option value
         */
        void activeValueChanged (const QString & optionId,
            const QString & optionValueId); 

    public Q_SLOTS:
    
        /*! 
           \brief Set active value id
           \param id ID of activated value
           \return true if active value was changed
         */            
        bool setActiveValueId (const QString & id);          
                    
    private:
    
        ServiceOptionPrivate * const d_ptr; //!< Private data
           
    };
}

#endif