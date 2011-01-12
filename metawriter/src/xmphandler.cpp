
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



 
#include "xmphandler.h"
#include <exempi/xmp.h>
#include <exempi/xmpconsts.h>
#include <QByteArray>
#include <QDebug>
#include "magic.h"

using namespace Metaman;

XmpHandler::XmpHandler() :
    m_xmpPacket(0)
{
    qDebug() << "initing xmp";
    m_initDone = xmp_init();
    qDebug() << "init done";
}


XmpHandler::~XmpHandler()
{
    xmp_terminate();
    m_initDone = false;
}



QByteArray XmpHandler::getProcessedData()
{
    qDebug() << "getProcessedData()";
    XmpStringPtr buffer = xmp_string_new();
    uint32_t options = XMP_SERIAL_ENCODEUTF8 | XMP_SERIAL_READONLYPACKET;
    uint32_t padding = 0;
    bool resultOk = xmp_serialize(m_xmpPacket, buffer, options, padding);
    QByteArray xmlData;

    if (resultOk) {
        qDebug() << "getting buffer " << buffer;
        const char* stringbuffer = xmp_string_cstr(buffer);
        xmlData = QByteArray(stringbuffer);
    }
    else {
        qDebug() << "failed to serialize";
    }

    xmp_string_free(buffer);
    buffer = 0;
    xmp_free(m_xmpPacket);
    m_xmpPacket = 0;

    return xmlData;
}



OperationResult XmpHandler::removeDescription()
{
    const char* nameSpace = NS_DC;
    return deleteProperty(nameSpace, XMP_PROPERTY_DESCRIPTION);
}



OperationResult XmpHandler::removeTitle()
{
    const char* nameSpace = NS_DC;
    return  deleteProperty(nameSpace, XMP_PROPERTY_TITLE);
}



OperationResult XmpHandler::setDescription(const QByteArray& description)
{
    qDebug() << "setting description " << description;

    const char* nameSpace        = NS_DC;
    QByteArray noGenericLanguage = XMP_LANGUAGE_NONE;
    QByteArray specificLanguage  = XMP_LANGUAGE_DEFAULT;

    // Get rid of the old property first. This will effectively make sure there
    // are no conflicting language variants.
    deleteProperty(nameSpace, XMP_PROPERTY_DESCRIPTION);

    OperationResult operationResult =
            setLocalizedProperty(nameSpace,
                                 XMP_PROPERTY_DESCRIPTION,
                                 description,
                                 noGenericLanguage,
                                 specificLanguage);

    return operationResult;
}



OperationResult XmpHandler::setTitle(const QByteArray& title)
{
    qDebug() << "setting title " << title;

    const char* nameSpace = NS_DC;

    // Get rid of the old property first. This will effectively make sure there
    // are no conflicting language variants.
    deleteProperty(nameSpace, XMP_PROPERTY_TITLE);

    QByteArray noGenericLanguage = XMP_LANGUAGE_NONE;
    QByteArray specificLanguage = XMP_LANGUAGE_DEFAULT;
    OperationResult operationResult = setLocalizedProperty(nameSpace,
                                                           XMP_PROPERTY_TITLE,
                                                           title,
                                                           noGenericLanguage,
                                                           specificLanguage);

    return operationResult;
}



OperationResult XmpHandler::setAuthor(const QByteArray& author)
{
    qDebug() << "setting author " << author;

    const char* nameSpace = NS_DC;

    // Get rid of the old property first. This will effectively make sure there
    // are no conflicting language variants.
    deleteProperty(nameSpace, XMP_PROPERTY_AUTHOR);

    QByteArray noGenericLanguage = XMP_LANGUAGE_NONE;
    QByteArray specificLanguage = XMP_LANGUAGE_DEFAULT;
    OperationResult operationResult = setLocalizedProperty(nameSpace,
                                                           XMP_PROPERTY_AUTHOR,
                                                           author,
                                                           noGenericLanguage,
                                                           specificLanguage);

    return operationResult;
}



OperationResult XmpHandler::setGpsString(const QByteArray& GPSLatitude,
                                         const QByteArray& GPSLongitude,
                                         const QByteArray& GPSAltitude,
                                         const QByteArray& GPSAltitudeRef)
{
    qDebug() << "setting GPS string ";

    const char* nameSpace = NS_EXIF;

    OperationResult operationResult = OPERATION_OK;
    OperationResult latitudeResult = setProperty(nameSpace,
                                                 XMP_PROPERTY_GPS_LATITUDE,
                                                 GPSLatitude);
    OperationResult longitudeResult = setProperty(nameSpace,
                                                  XMP_PROPERTY_GPS_LONGITUDE,
                                                  GPSLongitude);

    OperationResult altitudeResult = setProperty(nameSpace,
                                                 XMP_PROPERTY_GPS_ALTITUDE,
                                                 GPSAltitude);

    OperationResult altitudeRefResult = setProperty(nameSpace,
                                                    XMP_PROPERTY_GPS_ALTITUDE_REF,
                                                    GPSAltitudeRef);

    if (latitudeResult != OPERATION_OK ||
        longitudeResult != OPERATION_OK ||
        altitudeRefResult != OPERATION_OK ||
        altitudeResult != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }
    

    return operationResult;
}



OperationResult XmpHandler::setXmpDataToBeProcessed(const QByteArray& xmpData)
{
    qDebug() << "setting data to process";

    OperationResult operationResult = OPERATION_OK;
    m_xmpPacket = xmp_new_empty();

    if (!xmpData.isEmpty()) {
        const char* buffer = xmpData.data();
        size_t bufferLength = xmpData.size();
        bool parsingOk = xmp_parse(m_xmpPacket, buffer, bufferLength);

        if (!parsingOk) {
            operationResult = OPERATION_GENERAL_ERROR;
        }
    }
    
    return operationResult;
}



OperationResult XmpHandler::setProperty(const char* nameSpace,
                                        const QByteArray& propertyName,
                                        const QByteArray& value)
{
    qDebug() << "setProperty()";

    OperationResult operationResult = OPERATION_OK;
    
    if (value.isEmpty()) {
        operationResult = deleteProperty(nameSpace, propertyName);
    }
    else {
        uint32_t options = 0;
        bool resultOk = false;

        if (m_xmpPacket != 0) {
            // The property will be created if it doesn't exist.
            resultOk = xmp_set_property(m_xmpPacket,
                                        nameSpace,
                                        propertyName.data(),
                                        value.data(),
                                        options);
        }

        if (!resultOk) {
            qDebug() << "xmp error: " << xmp_get_error();
            operationResult = OPERATION_GENERAL_ERROR;
        }
    }
    
    return operationResult;
}



OperationResult XmpHandler::setLocalizedProperty(const char* nameSpace,
                                                 const QByteArray& propertyName,
                                                 const QByteArray& value,
                                                 const QByteArray& genericLanguage,
                                                 const QByteArray& specificLanguage)
{
    qDebug() << "setLocalizedProperty()";

    OperationResult operationResult = OPERATION_OK;
    
    if (value.isEmpty()) {
        operationResult = deleteProperty(nameSpace, propertyName);
    }
    else {
        uint32_t propertyOptions =
                XMP_PROP_VALUE_IS_ARRAY | XMP_PROP_ARRAY_IS_ALT;
        bool resultOk = false;
        const char* noValue = "";

        if (m_xmpPacket != 0) {
            // The property will be created if it doesn't exist.
            resultOk = xmp_set_property(m_xmpPacket,
                                        nameSpace,
                                        propertyName.data(),
                                        noValue,
                                        propertyOptions);

            uint32_t qualifierOptions = XMP_PROP_IS_QUALIFIER |
                                        XMP_PROP_HAS_LANG;
            resultOk = xmp_set_localized_text(m_xmpPacket,
                                              nameSpace,
                                              propertyName.data(),
                                              genericLanguage.data(),
                                              specificLanguage.data(),
                                              value.data(),
                                              qualifierOptions);
        }
         
        if (!resultOk) {
            qDebug() << "xmp error: " << xmp_get_error();
            operationResult = OPERATION_GENERAL_ERROR;
        }
    }
    
    return operationResult;
}




OperationResult XmpHandler::deleteProperty(const char* nameSpace,
                                           const QByteArray& propertyName)
{
    qDebug() << "deleteProperty()";

    // No need to check if the property exists.
    OperationResult result = OPERATION_OK;

    if (m_xmpPacket != 0) {
         bool deleted = xmp_delete_property(m_xmpPacket,
                                            nameSpace,
                                            propertyName.data());
         if (!deleted) {
             result = OPERATION_GENERAL_ERROR;
         }

    }

    return result;
}



OperationResult XmpHandler::registerNamespace(const char* nameSpaceURI,
                                              const char* suggestedPrefix)
{
    qDebug() << "registering namespace " << nameSpaceURI;

    // Pass NULL prefix parameter because we are not really interested in this
    // value.
    XmpStringPtr prefix = 0; 
    bool alreadyRegistered = xmp_namespace_prefix(nameSpaceURI, prefix);
    OperationResult result = OPERATION_OK;
    
    if (!alreadyRegistered) {
        XmpStringPtr registeredPrefixString = xmp_string_new();
        bool registered = xmp_register_namespace(nameSpaceURI,
                                                 suggestedPrefix,
                                                 registeredPrefixString);
        xmp_string_free(registeredPrefixString);

        if (!registered) {
            result = OPERATION_GENERAL_ERROR;
        }
    }
    else {
            qDebug() << "Namespace already registered";
    }
    
    return result;
}



OperationResult XmpHandler::removeAuthor()
{
    const char* nameSpace = NS_DC;
    return deleteProperty(nameSpace, XMP_PROPERTY_AUTHOR);
}



OperationResult XmpHandler::removeLocationInfo()
{
    const char* nameSpace = NS_PHOTOSHOP;
    OperationResult resultDeleteCountry =
            deleteProperty(nameSpace, XMP_PROPERTY_GEOTAG_COUNTRY);
    OperationResult resultDeleteCity =
            deleteProperty(nameSpace, XMP_PROPERTY_GEOTAG_CITY);
    OperationResult resultDeleteDistrict =
            deleteProperty(nameSpace, XMP_PROPERTY_GEOTAG_DISTRICT);

    OperationResult operationResult = OPERATION_OK;

    if (resultDeleteCountry  != OPERATION_OK ||
        resultDeleteCity     != OPERATION_OK ||
        resultDeleteDistrict != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Metaman::XmpHandler::removeGpsString()
{
    const char* nameSpace = NS_EXIF;
    OperationResult latitudeResult =
            deleteProperty(nameSpace, XMP_PROPERTY_GPS_LATITUDE);
    OperationResult longitudeResult =
            deleteProperty(nameSpace, XMP_PROPERTY_GPS_LONGITUDE);
    OperationResult altitudeResult =
            deleteProperty(nameSpace, XMP_PROPERTY_GPS_ALTITUDE);
    OperationResult altitudeRefResult =
            deleteProperty(nameSpace, XMP_PROPERTY_GPS_ALTITUDE_REF);

    OperationResult operationResult = OPERATION_OK;

    if (latitudeResult != OPERATION_OK ||
        longitudeResult != OPERATION_OK ||
        altitudeRefResult != OPERATION_OK ||
        altitudeResult != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Metaman::XmpHandler::removeAllXmpData()
{
    xmp_free(m_xmpPacket);
    m_xmpPacket = xmp_new_empty();

    OperationResult operationResult = OPERATION_OK;

    if (m_xmpPacket == 0) {
        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Metaman::XmpHandler::setKeywords(const QList<QByteArray>& keywords)
{
    qDebug() << "setKeywords()";
    
    OperationResult operationResult = OPERATION_OK;

    if (m_xmpPacket != 0) {
        const char* nameSpace = NS_DC;
        uint32_t propertyOptions = XMP_PROP_VALUE_IS_ARRAY;
        uint32_t itemOptions = 0;
        const char* noValue = "";
        
        bool resultOk = xmp_set_property(m_xmpPacket,
                                        nameSpace,
                                        XMP_PROPERTY_KEYWORDS,
                                        noValue,
                                        propertyOptions);

        foreach (QByteArray keyword, keywords) {
            resultOk = xmp_append_array_item(m_xmpPacket,
                                             nameSpace,
                                             XMP_PROPERTY_KEYWORDS,
                                             propertyOptions,
                                             keyword.constData(),
                                             itemOptions);

            if (!resultOk) {
                // Let's be strict for now.
                qDebug() << "xmp error: " << xmp_get_error();
                operationResult = OPERATION_GENERAL_ERROR;
                break;
            }
        }
    }

    return operationResult;
}



OperationResult XmpHandler::setGeotag(const QByteArray& geotag)
{
    qDebug() << "setting geotag " << geotag;

    QList<QByteArray> geotagParts = geotag.split(',');
    QByteArray geotagCountry;
    QByteArray geotagCity;
    QByteArray geotagDistrict;

    if (!geotagParts.isEmpty()) {
        geotagCountry = geotagParts.first();
    }

    if (geotagParts.size() > 1) {
        geotagCity = geotagParts.at(1);
    }

    if (geotagParts.size() > 2) {
        geotagDistrict = geotagParts.at(2);
    }

    const char* nameSpace = NS_DC;
    OperationResult setCountryResult = 
        setProperty(nameSpace, XMP_PROPERTY_GEOTAG_COUNTRY, geotagCountry);
    OperationResult setCityResult = 
        setProperty(nameSpace, XMP_PROPERTY_GEOTAG_CITY, geotagCity);
    OperationResult setDistrictResult = 
        setProperty(nameSpace, XMP_PROPERTY_GEOTAG_DISTRICT, geotagDistrict);
    OperationResult operationResult = OPERATION_OK;

    if (setCountryResult  != OPERATION_OK ||
        setCityResult     != OPERATION_OK ||
        setDistrictResult != OPERATION_OK){
    
        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}