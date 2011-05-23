
/*
 * This file is part of Web Upload Engine for MeeGo social networking uploads
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Contact: Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
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



 
#ifndef BACKENDINTERFACE_H
#define BACKENDINTERFACE_H

#include "metamandatatypes.h"

class QByteArray;
class QFile;

namespace Metaman {

/*!
    \class BackendInterface
    \brief An interface class for file format handler classes
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class BackendInterface
{
public:

    /**
     * \brief Read file contents
     * @param inputFile The file to read from
     * @return Operation result
     */
    virtual OperationResult readFile(QFile& inputFile) =0;

    /**
     * \brief Write atom contents to a file
     * @param inputFile The file to write to
     * @return Operation result
     */
    virtual OperationResult writeFile(QFile& outputFile) =0;

    /**
     * \brief Remove all metadata
     * @return Operation result
     */
    virtual OperationResult removeAllMetaData() =0;

    /**
     * \brief Remove geotag from metadata
     * @return Operation result
     */
    virtual OperationResult removeGeoTag() = 0;

    /**
     * \brief Remove author from metadata
     * @return Operation result
     */
    virtual OperationResult removeAuthor() = 0;

    /**
     * \brief Remove description from metadata
     * @return Operation result
     */
    virtual OperationResult removeDescription() = 0;

    /**
     * \brief Remove GPS string from metadata
     * @return Operation result
     */
    virtual OperationResult removeGpsString() = 0;

    /**
     * \brief Remove title from metadata
     * @return Operation result
     */
    virtual OperationResult removeTitle() = 0;

    /**
     * \brief Set description to metadata
     * @return Operation result
     */
    virtual OperationResult setDescription(const QByteArray& description) = 0;

    /**
     * \brief Set title to metadata
     * @return Operation result
     */
    virtual OperationResult setTitle(const QByteArray& title) = 0;

    /**
     * \brief Set author to metadata
     * @return Operation result
     */
    virtual OperationResult setAuthor(const QByteArray& author) = 0;

    /**
     * \brief Set GPS string to metadata
     * @return Operation result
     */
    virtual OperationResult setGpsString(const QByteArray& GPSLatitude,
                                         const QByteArray& GPSLongitude,
                                         const QByteArray& GPSAltitude,
                                         const QByteArray& GPSAltitudeRef) = 0;

    /**
     * \brief Set keywords to the metadata
     * @param keywords List of keywords
     * @return Operation result
     */
    virtual OperationResult setKeywords(const QList<QByteArray>& keywords) = 0;


    /**
     * \brief Set geotag to the metadata
     * @param geotag A complete geotag 
     * @return Operation result
     */
    virtual OperationResult setGeotag(const QByteArray& geotag) = 0;


    /**
     * \brief Check if the handler is able to process the input file
     * @param inputFile File to evaluate
     * @return True if able to process, false if not
     */
    virtual bool ableToProcess(QFile& inputFile) =0;

};

}

#endif
