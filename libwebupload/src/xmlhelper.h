 
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

#ifndef _WEBUPLOAD_XML_HELPER_H_
#define _WEBUPLOAD_XML_HELPER_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDomElement>

namespace WebUpload {

    /*!
       \class XmlHelper
       \brief Shared XML functionality for other classes
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class XmlHelper : public QObject {
    
        Q_OBJECT
        
    public:

        XmlHelper (QObject * parent = 0);
        virtual ~XmlHelper ();

        /*!
          \brief Will try to get localizated content text of element. Checks
                 key attribute of element if defined.
          \param element Element which is parsed
          \return Localizated text content of element
         */
        static QString getLocalizatedContent (const QDomElement & element);
        
         /*!
          \brief Function to receive boolean value from attribute
          \param elem Element used
          \param attName Attribute name used
          \param defValue Default value returned if attribute not found or
                          if it's value isn't valid.
          \return attributes value or default if value not defined
         */
        static bool attributeValueToBool (QDomElement & elem,
            const QString & attribute, bool defValue = false);
            
        /*!
          \brief Resolve mime types under given tag
          \param elem Element which has mime tags
          \return List of mime types (strings). Empty list if no mime tags found.
         */
        static QStringList readMimeTypes (const QDomElement & elem);
        
        /*!
          \brief Check if mime type is defined in given list
          \param mimeType Mime type of file
          \param mimeTypeList List of mime types accepted
          \return <code>true</code> if given mime type is accepted by the list.
                  Also <code>true</code> if given list is empty.
         */
        static bool mimeTypeListCheck (const QString & mimeType,
            const QStringList & mimeTypeList);
    
    };
}

#endif
