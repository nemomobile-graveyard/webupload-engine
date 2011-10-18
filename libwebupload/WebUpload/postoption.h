 
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

#ifndef _WEBUPLOAD_POST_OPTION_H_
#define _WEBUPLOAD_POST_OPTION_H_

#include "WebUpload/export.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDomElement>

namespace WebUpload {

    // Forward declaration of required classes
    class PostOptionPrivate;
    class Account;
    class Media;
    class Entry;
    
    /*!
        \class PostOption
        \brief Class representing a single post option. This is a base
               class, and it is the derived classes that would actually be
               used.
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT PostOption : public QObject {    
        Q_OBJECT
        
    public:
        /*!
           \brief Constructor
           \param parent QObject parent
         */
        PostOption (QObject * parent = 0);
        
        virtual ~PostOption();
        
        /*! 
           \brief Init post option from dom element
           \param element Dom element where data is read
           \return <code>true</code> if init was success
         */
        virtual bool init (QDomElement & element) = 0;

        // Enumeration describing the type of supported options
        enum Type {
            OPTION_TYPE_TITLE = 0,      //!< common option: Title
            OPTION_TYPE_DESC,           //!< common option: Description
            OPTION_TYPE_TAGS,           //!< common option: Tags
            OPTION_TYPE_METADATA,       //!< common option: Metadata filtering
            OPTION_TYPE_IMAGE_RESIZE,   //!< common option: Image resize
            OPTION_TYPE_VIDEO_RESIZE,   //!< common option: Video resize
            OPTION_TYPE_FACE_TAGS,      //!< common option: Face tags

            OPTION_TYPE_COMMON_TYPES_N, //!< No more common options
            OPTION_TYPE_SERVICE = OPTION_TYPE_COMMON_TYPES_N, //!< Service option

            OPTION_TYPE_N               //!< Max value. DO NOT USE
        };


        /*!
          \brief Init the standard upload option with an enum describing what
                 type of option it is. This function is called only if there
                 was no xml information for the option in the service file.
                 This initializes the given option with default values
          \param optionType Type of option being created
          \param caption String used to present the option to the user
         */
        virtual bool init (enum Type optionType, QString caption = QString());
        
        /*!
          \brief Check if the option is valid for a single item or not
                 If this returns <code>false</code>, then this option will not
                 be shown in the editing page for single item in the multiple
                 item upload scenario. 
                 For instance, for service specific options, resize and
                 metadata options, this function would typically return
                 <code>false</code>.
          \param media Pointer to media being shown in the editing page
          \return <code>true</code> if the option can be shown in the editing
                 page for a single item in the multiple item upload scenario
         */
        virtual bool validForMedia (const Media * media) const = 0;

        /*!
          \brief Check if the option is valid for the transfer or not.
                 If this function returns <code>false</code> then this option
                 will not be shown in the publish page in the multiple item
                 upload scenario. 
                 For instance, for title and description options, this function
                 will return <code>false</code>
          \param entry Pointer to entry representing the transfer
          \return <code>true</code> if the option can be shown in the publish
                  page in the multiple item upload scenario
         */
        virtual bool validForEntry (const Entry * entry) const = 0;
        
        /*! 
          \brief Get caption text for option. This is the caption shown in the
                 UI
          \return Option caption
         */
        QString caption () const;

        /*!
          \brief Get the type of the option.
          \return Type of the option
         */
        Type type () const;

        /*!
          \brief Get the display order of the option
          \return Integer value indicating the display order of the option
         */
        int order () const;

        /*!
          \brief Get pointer to account this option is associated with
          \return Pointer to associated account
         */
        Account * account () const;

    protected:

        /*!
          \brief Check if option is valid for given mime type
          \param mimeType Mime type
          \return <code>true</code> if this upload option should be shown
         */
        bool validForMimeType (const QString &mimeType) const;

        /*!
          \brief Protected function to allow setting the type of the option
          \param type Option type
         */
        void setType (enum Type type);

        /*!
          \brief Protected function to allow setting the caption
          \param caption Caption of the upload option
         */
        void setCaption (const QString caption);

        /*!
          \brief Protected function to allow setting the list of mimetypes this
                 option will work with. The mimetype in the list can be a
                 regular expression. 
                 This function should be called only if there are mime type
                 restrictions for the option. By default, the mime type list
                 will be empty, indicating that the option will work for any
                 mimetype.
          \param mimeList QStringList of supported mimetype
         */
        void setSupportedMimeTypes (const QStringList mimeList);

        /*!
          \brief Protected function to allow the setting of the order of the
                 option. The default order for an option would be max integer
                 value - in which case, we do not state in what order the
                 option would be shown.
          \param order Integer value specifying the display order of the option
         */
        void setOrder (int order);

    private:
    
        PostOptionPrivate * const d_ptr; //!< Private data
           
    };
}

#endif
