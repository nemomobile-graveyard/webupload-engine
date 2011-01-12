
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



 
#ifndef METAAPPLICATION_H
#define METAAPPLICATION_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include "metamandatatypes.h"

namespace Metaman {
    class BackendInterface;

    /*!
        \class MetaApplication
        \brief The main interface to the metadata handler
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class MetaApplication 
    {
    public:
        enum FileFormat {
            FORMAT_UNKNOWN,
            FORMAT_ISO_BASE_MEDIA, // MPEG-4 Part 12
            FORMAT_MP4,            // MPEG-4 Part 14
            FORMAT_3GPP,
            FORMAT_3GPP2
        };

        /**
         * \brief Constructor
         */
        MetaApplication();

        /**
         * \brief Constructor
         * @param operationMode Operation mode of the application
         */
        MetaApplication(OperationMode operationMode);

        /**
         * \brief Destructor
         */
        ~MetaApplication();

        /**
         * \brief Sets output file
         * @param outputFilePath Full path of the ouput file
         * @return Operation result
         */
        OperationResult setOutputFile(const QString& outputFilePath);

        /**
         * \brief Sets input file
         * @param inputFilePath Full path of the input file
         * @return Operation result
         */
        OperationResult setInputFile(const QString& inputFilePath);

        /**
         * \brief Writes the processed data to the output file
         * @return Operation result
         */
        OperationResult writeFile();

        /**
         * \brief Reads the input file
         * @return Operation result
         */
        OperationResult readFile();

        /**
         * \brief Evaluates if given input file can be processed
         * @return True if able to process, false if not
         */
        bool ableToProcess();

        /**
         * \brief Erase all metadata
         * @return Operation result
         */
        OperationResult eraseMetaData();

        /**
         * \brief Erase author and location information from metadata
         * @return Operation result
         */
        OperationResult eraseAuthorAndGps();

        /**
         * \brief Sets description to metadata
         * @param description Description to be set
         * @return Operation result
         */
        OperationResult setDescription(const QByteArray& description);

        /**
         * \brief Sets title to metadata
         * @param title Title to be set
         * @return Operation result
         */
        OperationResult setTitle(const QByteArray& title);
        
        /**
         * \brief Add keywords to metadata
         * @param keywords List of keywords
         * @return Operation result
         */
        OperationResult addKeywords(const QList<QByteArray>& keywords);
        

        /**
         * \brief Set geotag to metadata
         * @param geotag A complete geotag
         * @return Operation result
         */
        OperationResult setGeotag(const QByteArray& geotag);
    private:
        /**
         * \brief Check if the file is a Mpeg4 type of file
         * @param File to be evaluated
         * @return True if yes, false if not
         */
        bool hasFtypAtom(QFile& file);
        
    private:

        /// Full path of input file
        QString m_inputFilePath;

        /// Full path of output file
        QString m_outputFilePath;

        /// Pointer to the file handler
        BackendInterface* m_backend;

        /// Input file object
        QFile m_inputFile;

        /// Operation mode
        OperationMode m_operationMode;
    };

}

#endif