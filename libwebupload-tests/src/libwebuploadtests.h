 
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

#ifndef _LIB_WEB_UPLOAD_TESTS_H_
#define _LIB_WEB_UPLOAD_TESTS_H_

#include <QObject>
#include <QUrl>
#include <QStringList>
#include <manager.h>

#include "WebUpload/Media"
#include "WebUpload/Error"

#include <QtSparql>

class LibWebUploadTests : public QObject {
    Q_OBJECT

	private slots:
        void initTestCase();
        void cleanupTestCase();
		
        // Try to create option classes
		void createOptions();
		
        // Load entry and check values read
        void loadEntry();

        // Load enties with errors
        void loadEntryInvalid();

        // Test the geotaginfo class
        void testGeotagInfo ();
        
        // Test system class
        void systemChecks();
        
        // Change media metadata values
        void modifyMediaFields();
        
        // Check entry metadata handling
        void entryMetadataHandling();
        
        // Check completed state changes
        void checkCompleted();
        
        // Check cancelled state changes
        void checkCancelled1();        
        void checkCancelled2();        

        // Check the resize options
        void checkImageResizeOptionOriginal();
        void checkImageResizeOptionMedium();
        void checkImageResizeOptionSmall();
        
        // Account private tests
        void checkAccountPrivate ();
        
        // Test service private class
        void checkServicePrivate();

		void testHttpMultiContentIO();
		
		void testError();

        void testErrorSerialization ();
		
		void testXmlHelper();

        void testProcessExchangeData ();

        void testPost ();

    private:

        inline void createAccounts ();
        inline void cleanupAccounts ();
        inline QSparqlResult * blockingSparqlQuery (const QSparqlQuery &query,
            bool singleResponse = false);

        /*!
          \brief Create entry and serialize it
          \param path Where entry is serialized
          \return true if entry was successfully created
         */        
        inline void createEntry (const QString & path);

        /*!
          \brief Create media and initialize it with an image file
          \param path Where image file is present
          \param file Name of image file
          \param media Pointer to the WebUpload::Media * that needs to be
                 initialized
         */        
        inline void createMedia (const QString & path, 
                const QString &file, WebUpload::Media **media );

        /*!
          \brief  Initialize resizeFiles string list according to the
                  enumeration and the resize array
         */
        inline void initResizeFields 
            (WebUpload::ImageResizeOption resizeOption);
    
        //! List containing tracker transfer uris to be cleared
        QStringList cleanTrackerUris;
        
        //! List of filepaths that shouldn't exist after tests
        QStringList checkFilePaths;

        QSparqlConnection * m_sparqlConnection;

        //! Enumeration of the test cases for resizing
        enum ResizeTestCases {
            HT_GT_WT_GT_1280, // Height > Width and Height > 1280
            WT_GT_HT_GT_1280, // Width > Height and Width > 1280 
            HT_GT_WT_EQ_1280, // Height > Width, Height == 1280 
            WT_GT_HT_EQ_1280, // Width > Height and Width == 1280 
            HT_GT_WT_LT_1280_GT_640, // Height > Width and 1280 > Height > 640
            WT_GT_HT_LT_1280_GT_640, // Width > Height and 1280 > Width > 640
            HT_GT_WT_EQ_640, // Height > Width, Height == 640 
            WT_GT_HT_EQ_640, // Width > Height and Width == 640 
            HT_GT_WT_LT_640, // Height > Width and Height < 640
            WT_GT_HT_LT_640, // Width > Height and Width < 640 

            RESIZE_CASES_MAX
        };

        // List of files used for resizing. The files are added so that the
        // enumeration matches the file in the appropriate index
        QStringList resizeFiles;

        // Boolean array. Initialized per test case with the expected return
        // value
        bool resize[RESIZE_CASES_MAX];

        Accounts::Manager accMgr;
        Accounts::AccountIdList accountList;
};

#endif 
