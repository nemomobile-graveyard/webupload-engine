
/*
 * This file is part of Web Upload Engine for MeeGo social networking uploads
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,     
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS  
 * IN THE SOFTWARE. 
 */



 
#ifndef XMPHANDLER_H
#define XMPHANDLER_H

#include <exempi/xmp.h>
#include "metamandatatypes.h"
#include <QByteArray>

namespace Metaman {
    
/*!
    \class XmpHandler
    \brief Handler for XMP data
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class XmpHandler
{
public:
    /**
     * \brief Constructor
     */
    XmpHandler();
    
    /**
     * \brief Destructor
     */
    ~XmpHandler();
    
    /**
     * \brief Get processed XMP data
     * @return Processed XMP data
     */
    QByteArray getProcessedData();
    
    /**
     * \brief Remove description from XMP data
     * @return Operation result
     */
    OperationResult removeDescription();
    
    /**
     * \brief Remove title from XMP data
     * @return Operation result
     */
    OperationResult removeTitle();
    
    /**
     * \brief Set description to the XMP data
     * @param description Description
     * @return Operation result
     */
    OperationResult setDescription(const QByteArray& description);
    
    /**
     * Set title to the XMP data
     * @param title Title
     * @return Operation result
     */
    OperationResult setTitle(const QByteArray& title);

    /**
     * Set author to the XMP data
     * @param author Author
     * @return Operation result
     */
    OperationResult setAuthor(const QByteArray& author);
    
    /**
     * \brief Set XMP data to be processed
     * @param xmpData XMP data to be processed
     * @return Operation result
     */
    OperationResult setXmpDataToBeProcessed(const QByteArray& xmpData);
    
    /**
     * \brief Remove author from the XMP data
     * @return Operation result
     */
    OperationResult removeAuthor();
    
    /**
     * \brief Remove location information from the XMP data
     * @return Operation result
     */
    OperationResult removeLocationInfo();
    
    /**
     * \brief Remove GPS string from XMP data.
     * @return Operation result
     */
    OperationResult removeGpsString();

    /**
     * \brief Remove all XMP metadata
     * @return Operation result
     */
    OperationResult removeAllXmpData();

    
    OperationResult setGpsString(const QByteArray& GPSLatitude,
                                 const QByteArray& GPSLongitude,
                                 const QByteArray& GPSAltitude,
                                 const QByteArray& GPSAltitudeRef);
    
    /**
     * \brief Set keywords to the XMP metadata
     * @param keyword List of keywords
     * @return Operation result
     */
    OperationResult setKeywords(const QList<QByteArray>& keywords);

    /**
     * \brief Set geotag to the XMP metadata
     * @param geotag A complete geotag
     * @return Operation result
     */
    OperationResult setGeotag(const QByteArray& geotag);

private:
    
    /**
     * \brief Create or modify specified XMP property
     * @param nameSpace Schema namespace
     * @param propertyName Property name
     * @param value Property value
     * @return Operation result
     */
    OperationResult setProperty(const char* nameSpace,
                                const QByteArray& propertyName,
                                const QByteArray& value);
    
    /**
     * \brief Create or modify specified XMP property with language option
     * @param nameSpace Schema namespace
     * @param propertyName Property name
     * @param value Property value
     * @param genericLanguage Generic language
     * @param specificLanguage Specified language
     * @return Operation result
     */
    OperationResult setLocalizedProperty(const char* nameSpace,
                                         const QByteArray& propertyName,
                                         const QByteArray& value,
                                         const QByteArray& genericLanguage,
                                         const QByteArray& specificLanguage);
    
    /**
     * \brief Delete specified XMP property
     * @param nameSpace Schema namespace
     * @param propertyName Property name
     * @return Operation result
     */
    OperationResult deleteProperty(const char* nameSpace,
                                   const QByteArray& propertyName);
    
    /**
     * \brief Register XMP namespace
     * @param nameSpaceURI Namespace URI
     * @param suggestedPrefix Suggested prefix 
     * @return Operation result
     */
    OperationResult registerNamespace(const char* nameSpaceURI,
                                      const char* suggestedPrefix);

private:
    /// Tells if XMP initialization is done successfully or not
    bool m_initDone;

    /// XMP packet
    XmpPtr m_xmpPacket;

};

};
#endif