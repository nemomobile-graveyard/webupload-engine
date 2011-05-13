 
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

#include <QtTest/QtTest>
#include <QFileInfo>
#include <QSharedPointer>

#include <QtSparql>

#include <QImage>

#include <manager.h>
#include <QDebug>
#include <QUuid>
#include <QSignalSpy>

#include "libwebuploadtests.h"

#include "WebUpload/Entry"
#include "WebUpload/Media"
#include "WebUpload/ServiceOption"
#include "WebUpload/ServiceOptionValue"
#include "WebUpload/System"
#include "WebUpload/Service"
#include "serviceprivate.h"
#include "accountprivate.h"
#include "WebUpload/HttpMultiContentIO"
#include "WebUpload/processexchangedata.h"
#include "WebUpload/PluginInterface"
#include "WebUpload/Error"
#include "xmlhelper.h"
#include "WebUpload/CommonTextOption"
#include "commonoptionprivate.h"
#include "dummypost.h"

#define TEMP_ENTRY_PATH "/tmp/entry.xml"

using namespace WebUpload;

void LibWebUploadTests::initTestCase() {
    WebUpload::System::registerMetaTypes ();
    m_sparqlConnection = 0;
}

void LibWebUploadTests::cleanupTestCase() {

    cleanupAccounts ();

    // Clear test data in tracker database
    for (int i = 0; i < cleanTrackerUris.count(); ++i) {
        QSparqlQuery delQuery ("DELETE { ?:t a rdfs:Resource . "
            "?te a rdfs:Resource . } WHERE { ?:t mto:transferList ?te . }",
            QSparqlQuery::DeleteStatement);
        delQuery.bindValue ("t", QUrl (cleanTrackerUris[i]));

        QSparqlResult *result = blockingSparqlQuery (delQuery);
        if (result != 0) {
            delete result;
        }
    }
    
    int filesFound = 0;
    
    // Make sure tests did not leave files behind    
    for (int i = 0; i < checkFilePaths.size(); ++i) {    
        if (QFile::exists (checkFilePaths.at (i))) {
            qCritical() << "File left behind" << checkFilePaths.at (i);
            QFile::remove (checkFilePaths.at (i));
            ++filesFound;
        }
    }
        
    if (filesFound > 0) {
        QFAIL ("Files left behind");
    }

    if (m_sparqlConnection != 0) {
        delete m_sparqlConnection;
        m_sparqlConnection = 0;
    }
}

void LibWebUploadTests::createOptions() {

    ServiceOption * option = new ServiceOption (this);
    QVERIFY (option->id().isEmpty());    
    delete option;
    
    ServiceOptionValue * value = new ServiceOptionValue();
    QVERIFY (value->id().isEmpty());
    QVERIFY (value->name().isEmpty());
    QString id = "foobar";    
    value->setId (id);
    QCOMPARE (value->id(), id);
    value->setName (id);
    QCOMPARE (value->name(), id);
    delete value;
}

void LibWebUploadTests::modifyMediaFields() {
    createEntry (TEMP_ENTRY_PATH);

    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	QVERIFY (entry->init(TEMP_ENTRY_PATH));
    entry->setMetadataFilter(METADATA_FILTER_ALL);
    QVERIFY (entry->metadataFilterOption () == WebUpload::METADATA_FILTER_ALL);

	Media * media = entry->mediaAt (0);
	QVERIFY (media != 0);

    QVERIFY (media->title().isEmpty());
    entry->setMetadataFilter (WebUpload::METADATA_FILTER_NONE);
    QVERIFY (!media->title().isEmpty());
    QString title = media->title();
    QString newTitle = title;
    newTitle.append ("little more");
    media->setTitle (newTitle);
    QVERIFY (media->title() != title);
    entry->setMetadataFilter (WebUpload::METADATA_FILTER_ALL);
    QVERIFY (media->title().isEmpty());

    // Modify tags
    QVERIFY (media->tags().count() == 0);
    entry->setMetadataFilter (WebUpload::METADATA_FILTER_NONE);
    int numberOfTags = media->tags().count();
    QVERIFY (numberOfTags > 0);
    QString tag0 = media->tags().at(0);
    media->appendTag (tag0);
    QCOMPARE (media->tags().count(), numberOfTags);
    media->appendTag ("SomethingNew");
    QVERIFY (numberOfTags < media->tags().count());	
    numberOfTags = media->tags().count();
    media->appendTag ("SomethingNew");
    QVERIFY (numberOfTags == media->tags().count());
    media->removeTag ("SomethingNew");
    QVERIFY (numberOfTags - 1 == media->tags().count());
    media->removeTag ("SomethingNew");
    QVERIFY (numberOfTags - 1 == media->tags().count());
    media->clearTags ();
    QCOMPARE (media->tags().count(), 0);
    media->appendTag ("yes, box, holiday");
    QCOMPARE (media->tags().count(), 1);
    media->appendTag ("another tag");
    QCOMPARE (media->tags().count(), 2);
    media->clearTags ();
    QCOMPARE (media->tags().count(), 0);
    
    QString mediaPath = media->copyFilePath();    
    QVERIFY (mediaPath.isEmpty());
    QVERIFY (media->setCanceled());
    QVERIFY (entry->isCanceled());
    entry->reSerialize();
}

void LibWebUploadTests::entryMetadataHandling() {
    Entry * entry = new Entry();
    entry->setMetadataFilter(WebUpload::METADATA_FILTER_ALL);
    qDebug() << "Set metadata filter for entry";
    
    QList<QUrl> commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 0);
    qDebug() << "Checked for common tags, and got none";
    
    entry->removeTagFromAllMedia ("something");   
    entry->appendTagToAllMedia ("something");   
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 0);        
    qDebug() << "Removed/added tag \"something\" to all media";
    
    Media * media1 = new Media();
    media1->appendTag ("media1");
    media1->appendTag ("common");
    qDebug() << "Added tags \"media1\" and \"common\" to unintialized media";
    
    entry->appendMedia (media1);
    
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 0);        

    entry->setMetadataFilter(WebUpload::METADATA_FILTER_NONE);
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 2);
        
    entry->setMetadataFilter(WebUpload::METADATA_FILTER_ALL);
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 0);
        
    entry->setMetadataFilter(WebUpload::METADATA_FILTER_NONE);
    Media * media2 = new Media();
    media2->appendTag ("common");
    media2->appendTag ("media2");

    entry->appendMedia (media2);        
    
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 1);

    entry->appendTagToAllMedia ("extra");
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 2);  

    entry->removeTagFromAllMedia ("extra");
    commonTags = entry->commonTags();
    QCOMPARE (commonTags.size(), 1);  
    
    // There should not be any files written
    QVERIFY (entry->serializedTo().isEmpty());
    QVERIFY (media1->copyFilePath ().isEmpty());
    QVERIFY (media2->copyFilePath ().isEmpty());    
    
    delete entry;
}

void LibWebUploadTests::loadEntryInvalid() {
    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	
	QVectorIterator<Media *> mediaIter = entry->media();
	QVERIFY (!mediaIter.hasNext ());

    QString testPath = "/tmp/invalid_xml.xml";

    QVERIFY(!entry->init(testPath));
    checkFilePaths << testPath;

    delete entry;
}

void LibWebUploadTests::testGeotagInfo () {
    GeotagInfo *gInfo = new GeotagInfo ();

    QVERIFY (gInfo->isEmpty ());
    gInfo->setCountry (QLatin1String ("COUNTRY"));
    QVERIFY (!gInfo->isEmpty ());
    gInfo->clear ();
    QVERIFY (gInfo->isEmpty ());

    gInfo->setCountry (QLatin1String ("COUNTRY"));
    gInfo->setCity (QLatin1String ("CITY"));
    gInfo->setDistrict (QLatin1String ("DISTRICT"));
    QVERIFY (!gInfo->isEmpty ());
    QVERIFY (gInfo->country() == QLatin1String ("COUNTRY"));

    GeotagInfo gInfo1 (0, QLatin1String ("COUNTRY"), QLatin1String ("CITY"),
            QLatin1String ("DISTRICT"));
    QVERIFY (gInfo1 == *gInfo);
    delete gInfo;

    GeotagInfo gInfo2 (gInfo1);
    QVERIFY (!gInfo2.isEmpty ());
    QVERIFY (gInfo1 == gInfo2);

    GeotagInfo gInfo3 = gInfo1;
    QVERIFY (!gInfo3.isEmpty ());
    QVERIFY (gInfo3 == gInfo2);
}
	
void LibWebUploadTests::loadEntry() {
    createEntry (TEMP_ENTRY_PATH);

    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	
	QVERIFY (entry->init(TEMP_ENTRY_PATH));
    QVERIFY(entry->totalSize() == entry->unsentSize());
    entry->setMetadataFilter(METADATA_FILTER_ALL);

    Media* media;
	QVectorIterator<Media *> mediaIter = entry->media();
	mediaIter = entry->media();
    mediaIter.toFront();
	QVERIFY(mediaIter.hasNext()); 

    while (mediaIter.hasNext ()) {
        media = mediaIter.next ();
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);
        QVERIFY (QFile::exists(media->copyFilePath()));
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_ALREADY_COPIED);
        checkFilePaths << media->copyFilePath();
        QVERIFY(media->fileSize() > 0);
    }
    QVERIFY (entry->totalSize() > 0);

    mediaIter.toFront ();

    media = mediaIter.next();
    QVERIFY(media != NULL);
    QSparqlQuery query ("SELECT ?sTime WHERE { ?:te a mto:TransferElement . "
        "OPTIONAL { ?:te mto:startedTime ?sTime . } }");
    query.bindValue ("te", QUrl (media->trackerIri()));
    QSparqlResult * result = blockingSparqlQuery (query, true);
    QVERIFY (result != 0);
    QVERIFY (result->binding (0).value ().isValid () == false);
    delete result;

	QVERIFY(mediaIter.hasNext());  
    media = mediaIter.next();
	QVERIFY(!mediaIter.hasNext());  
    QVERIFY(media->setActive()); // Because only active transfers can fail
    QVERIFY(media->setFailed());
    QVERIFY(media->hasError());
    QVERIFY(entry->hasError());

    entry->reSerialize();
    QString afterErrorPath = media->copyFilePath();
    QVERIFY (!afterErrorPath.isEmpty());

    QVERIFY (QFile::exists (TEMP_ENTRY_PATH));
    QVERIFY (QFile::exists (afterErrorPath));

	delete entry;
	
    entry = new Entry();
    QVERIFY (entry->init(TEMP_ENTRY_PATH)); 
    QVERIFY(entry->isPending());
    Media * cancelMedia = entry->mediaAt (0);
    QVERIFY (cancelMedia != 0);
    QVERIFY(cancelMedia->setCanceled());
    QVERIFY(entry->isCanceled());
    delete entry; 

    // Media file shouldn't exists anymore
    QVERIFY (!QFile::exists (afterErrorPath));
}

void LibWebUploadTests::checkCompleted() {

    createEntry (TEMP_ENTRY_PATH);

    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	
    QVERIFY(entry->init("/tmp/entry.xml"));    
    QVERIFY(entry->totalSize() == entry->unsentSize());
    QVERIFY(entry->mediaCount() == 2);
    QVERIFY(entry->isPending());
    entry->checkShareFilter(METADATA_FILTER_ALL);
    
    Media* media;
	QVectorIterator<Media *> mediaIter = entry->media();
	mediaIter = entry->media();
    mediaIter.toFront();
	QVERIFY(mediaIter.hasNext());  

    while (mediaIter.hasNext ()) {
        media = mediaIter.next ();
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);
        QVERIFY (QFile::exists(media->copyFilePath()));
        checkFilePaths << media->copyFilePath();
    }

    mediaIter.toFront ();

    media = entry->mediaAt(0);
    QVERIFY(media == mediaIter.next());
    QSparqlQuery q1 ("SELECT ?sTime WHERE { ?:te a mto:TransferElement . "
        "OPTIONAL { ?:te  mto:startedTime ?sTime . } }");
    q1.bindValue ("te", QUrl (media->trackerIri()));
    QSparqlResult * result = blockingSparqlQuery (q1, true);
    QVERIFY (result != 0);
    QVERIFY (result->binding (0).value ().isValid () == false);
    delete result;

    QVERIFY(media->setActive());
    QVERIFY(entry->isActive());
    /***
     * TODO - Commenting for now - for some reason, this doesn't work
    QVERIFY(entry->reSerialize());
    QSparqlQuery q2 ("SELECT ?sTime ?eTime WHERE { "
        "?:te a mto:TransferElement ;  mto:startedTime ?sTime . "
        "OPTIONAL { ?:te mto:completedTime ?eTime . } }");
    q2.bindValue ("te", QUrl (media->trackerIri()));
    qDebug() << "Querying for" << media->trackerIri();
    result = blockingSparqlQuery (q2, true);
    QVERIFY (result != 0);
    QVariant sTime = result->binding(0).value();
    QVariant eTime = result->binding(1).value();
    qDebug () << sTime.isValid () << sTime.typeName();
    qDebug () << eTime.isValid ();
    delete result;
    QVERIFY (sTime.isValid ());
    QVERIFY (sTime.canConvert<QDateTime> ());
    QVERIFY (sTime.toDateTime ().isValid ());
    QVERIFY (eTime.isValid () == false);
     ***/
    QVERIFY(entry->reSerialize());
    
    delete entry;
    
    entry = new Entry();
    QVERIFY (entry->init(TEMP_ENTRY_PATH));
    QVERIFY(entry->isPending());

    media = entry->mediaAt(0);
    QVERIFY(media->setActive());
    QVERIFY(media->setCompleted("dummy://test.jpg"));
    QVERIFY(entry->isActive());
    QVERIFY(!QFile::exists(media->copyFilePath()));
    QVERIFY(entry->reSerialize());

    /***
     * TODO - Commenting for now - for some reason, this doesn't work
    QSparqlQuery q3 ("SELECT ?sTime ?cTime ?dest WHERE { "
        "?:te a mto:TransferElement ; mto:startedTime ?sTime ; "
        "mto:completedTime ?cTime ; mto:destination ?dest . }");
    q3.bindValue ("te", QUrl (media->trackerIri()));
    result = blockingSparqlQuery (q3, true);
    QVERIFY (result != 0);
    QVERIFY (result->binding (0).value ().isValid ());
    QVERIFY (result->binding (0).value ().canConvert<QDateTime> ());
    QVERIFY (result->binding (0).value ().toDateTime ().isValid ());

    QVERIFY (result->binding (1).value ().isValid ());
    QVERIFY (result->binding (1).value ().canConvert<QDateTime> ());
    QVERIFY (result->binding (1).value ().toDateTime ().isValid ());

    QVERIFY (result->binding (2).value ().isValid ());
    QVERIFY (result->binding (2).value ().canConvert<QString> ());
    QVERIFY (result->binding (2).value ().toString() == "dummy://test.jpg");
    delete result;
     ***/
    QVERIFY(entry->totalSize() > entry->unsentSize());
    QVERIFY(entry->reSerialize());

	delete entry;

    entry = new Entry();
    QVERIFY(entry->init(TEMP_ENTRY_PATH));

    QVERIFY(entry->isPending());

    QVERIFY(entry->mediaCount() == 2);
    media = entry->mediaAt(1);
    QVERIFY(media != NULL);
    QVERIFY(media->setActive());
    QVERIFY(entry->isActive());
    
    QString mediaPath = media->copyFilePath();
    QVERIFY (!mediaPath.isEmpty());
    QVERIFY (QFile::exists(mediaPath));  
    QVERIFY(entry->unsentSize() > 0);      
    
    QVERIFY(media->setCompleted(""));
    
    QVERIFY(entry->isSent());
    QVERIFY(media->isSent());
    
    QVERIFY(!QFile::exists(mediaPath));
    QVERIFY (entry->unsentSize() == 0);
    QVERIFY(entry->reSerialize());
	delete entry;

    entry = new Entry();
    QVERIFY(!entry->init(TEMP_ENTRY_PATH));
    delete entry;
}


void LibWebUploadTests::checkCancelled1() {

    createEntry (TEMP_ENTRY_PATH);
    
    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	
    QVERIFY(entry->init(TEMP_ENTRY_PATH));    
    QVERIFY(entry->isPending());
    entry->setMetadataFilter(METADATA_FILTER_ALL);
    
    Media* media;
	QVectorIterator<Media *> mediaIter = entry->media();
    mediaIter.toFront();
	QVERIFY(mediaIter.hasNext());  
		
    while (mediaIter.hasNext ()) {
        media = mediaIter.next ();
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);
        QVERIFY (QFile::exists(media->copyFilePath()));
        checkFilePaths << media->copyFilePath();
        
        QTemporaryFile temp ("/tmp/libwebupload-test-XXXXXX");
        temp.setAutoRemove (false);
        
        QVERIFY (temp.open ());

        QFileInfo fInfo (temp);
        QString tempPath = fInfo.absoluteFilePath();

        QVERIFY (temp.write ("foobar", 6) == 6);
        temp.close();

        qDebug() << "Checking if" << tempPath << "exists"; 
        QVERIFY (QFile::exists (tempPath));
        
        media->addToCleanUpList (tempPath);
        checkFilePaths << tempPath;
    }

    mediaIter.toFront ();

    media = mediaIter.next();
    QSparqlQuery q1 ("SELECT ?type WHERE { ?:te a mto:TransferElement ; "
        "rdf:type ?type . }");
    q1.bindValue ("te", QUrl (media->trackerIri()));
    QSparqlResult * result = blockingSparqlQuery (q1);
    QVERIFY (result != 0);
    delete result;
    QVERIFY(media->setActive());
    QVERIFY(entry->isActive());
    QVERIFY(entry->reSerialize());
    /*** TODO: Check why this does not work
    QSparqlQuery q2 ("SELECT ?sTime ?eTime WHERE { "
        "?:te a mto:TransferElement ;  mto:startedTime ?sTime . "
        "OPTIONAL { ?:te mto:completedTime ?eTime . } }");
    q2.bindValue ("te", QUrl (media->trackerIri()));
    result = blockingSparqlQuery (q2, true);
    QVERIFY (result != 0);
    QVERIFY (result->binding (0).value ().isValid ());
    QVERIFY (result->binding (0).value ().canConvert<QDateTime> ());
    QVERIFY (result->binding (0).value ().toDateTime ().isValid ());
    QVERIFY (result->binding (1).value ().isValid () == false);
    delete result;
     ***/
    
    QVERIFY(media->setCanceled());
    QVERIFY(entry->isCanceled());
    QVERIFY(!QFile::exists(media->copyFilePath()));
    // Now when the media state changes to cancel or done and the media's copy
    // is deleted, we do a reserialize then. If that means that the transfer
    // xml is deleted, then any reserialize after that will return false
    QVERIFY(entry->reSerialize() == false);
    /***
     * TODO - Check later why this does not work
    result = blockingSparqlQuery (q2, true);
    QVERIFY (result != 0);
    QVERIFY (result->binding (0).value ().isValid ());
    QVERIFY (result->binding (0).value ().canConvert<QDateTime> ());
    QVERIFY (result->binding (0).value ().toDateTime ().isValid ());
    QVERIFY (result->binding (1).value ().isValid () == false);
    delete result;
     ***/

    QVERIFY(media != NULL);
	QVERIFY(mediaIter.hasNext());  
    media = mediaIter.next();
    QVERIFY(entry->isCanceled());
    QVERIFY(!QFile::exists(media->copyFilePath()));

	QVERIFY(!mediaIter.hasNext());

	delete entry;
}


void LibWebUploadTests::checkCancelled2() {
    createEntry (TEMP_ENTRY_PATH);
    
    Entry * entry = new Entry ();
	QVERIFY(entry != 0);
	
    QVERIFY(entry->init(TEMP_ENTRY_PATH));    
    QVERIFY(entry->isPending());

    Media* media;
    QVectorIterator<Media *> mediaIter = entry->media();
    mediaIter.toFront();
	QVERIFY(mediaIter.hasNext());  
	
    while (mediaIter.hasNext ()) {
        media = mediaIter.next ();
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);
        QVERIFY (QFile::exists(media->copyFilePath()));
        checkFilePaths << media->copyFilePath();
    }
    QVERIFY(entry->reSerialize());

    qDebug() << "Calling cancel";
    entry->cancel();
    // Now when the media state changes to cancel or done and the media's copy
    // is deleted, we do a reserialize then. If that means that the transfer
    // xml is deleted, then any reserialize after that will return false
    QVERIFY(entry->reSerialize() == false);

    mediaIter.toFront ();
    while (mediaIter.hasNext ()) {
        media = mediaIter.next();
        QVERIFY(!QFile::exists(media->copyFilePath()));
    }

    QVERIFY(entry->isCanceled());
    delete entry;

    entry = new Entry();
    QVERIFY(!entry->init(TEMP_ENTRY_PATH));
    delete entry;
}

inline void LibWebUploadTests::initResizeFields (
        WebUpload::ImageResizeOption resizeOption) {
    resizeFiles.clear();

    // File names have heightXwidth
    resizeFiles << "libwebupload-test-1468X1330.jpg"
                << "libwebupload-test-1536X2048.jpg"
                << "libwebupload-test-1280X1160.jpg"
                << "libwebupload-test-960X1280.jpg"
                << "libwebupload-test-1200X1088.jpg"
                << "libwebupload-test-900X1200.jpg"
                << "libwebupload-test-640X580.jpg"
                << "libwebupload-test-480X640.jpg"
                << "libwebupload-test-500X453.jpg"
                << "libwebupload-test-450X600.jpg";

    if (resizeOption == WebUpload::IMAGE_RESIZE_NONE) {
        for (int i = 0; i < RESIZE_CASES_MAX; i++) {
            resize [i] = false;
        }
    } else {
        resize [HT_GT_WT_GT_1280] = resize [WT_GT_HT_GT_1280] = true;
        if (resizeOption == WebUpload::IMAGE_RESIZE_MEDIUM) {
            for (int i = HT_GT_WT_EQ_1280; i < HT_GT_WT_EQ_640; i++) {
                resize [i] = false;
            }
        } else {
            Q_ASSERT (resizeOption == WebUpload::IMAGE_RESIZE_SMALL);
            for (int i = HT_GT_WT_EQ_1280; i < HT_GT_WT_EQ_640; i++) {
                resize [i] = true;
            }
        }

        for (int i = HT_GT_WT_EQ_640; i < RESIZE_CASES_MAX; i++) {
            resize [i] = false;
        }
    }

}

void LibWebUploadTests::checkImageResizeOptionOriginal () {
    initResizeFields (WebUpload::IMAGE_RESIZE_NONE);

    // Check media files
    QString parentPath = QDir::homePath() + "/MyDocs/.images/";
    QDir dir (parentPath);
    QVERIFY (dir.exists ());

    WebUpload::Entry *entry = new WebUpload::Entry();
    entry->setImageResizeOption (WebUpload::IMAGE_RESIZE_NONE);
    QVERIFY (entry->imageResizeOption() == WebUpload::IMAGE_RESIZE_NONE);


    for (int i = 0; i < RESIZE_CASES_MAX; i++) {
        WebUpload::Media *media = 0;
        createMedia (parentPath, resizeFiles[i], &media);
        QVERIFY(media != 0);
        entry->appendMedia (media);
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);

        QUrl origPathUrl (media->origURI ());
        QImage original (origPathUrl.toLocalFile ());
        QImage copy (media->copyFilePath ());
        QSize origSize = original.size ();
        QSize copySize = copy.size ();

        qDebug () << "original's size is " << origSize;
        qDebug () << "copy's size is " << copySize;
        QVERIFY (origSize == copySize);
        QVERIFY(media->removeCopyFile());
    }

    delete entry;
    entry = 0;
}

void LibWebUploadTests::checkImageResizeOptionMedium () {
    initResizeFields (WebUpload::IMAGE_RESIZE_MEDIUM);

    // Check media files
    QString parentPath = QDir::homePath() + "/MyDocs/.images/";
    QDir dir (parentPath);
    QVERIFY (dir.exists ());

    int maxSize = 1280;

    WebUpload::Entry *entry = new WebUpload::Entry();
    entry->setImageResizeOption (WebUpload::IMAGE_RESIZE_MEDIUM);
    QVERIFY (entry->imageResizeOption() == WebUpload::IMAGE_RESIZE_MEDIUM);


    for (int i = 0; i < RESIZE_CASES_MAX; i++) {
        WebUpload::Media *media = NULL;
        createMedia (parentPath, resizeFiles[i], &media);
        QVERIFY (media != NULL);
        entry->appendMedia (media);
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);

        QImage original (media->srcFilePath ());
        QImage copy (media->copyFilePath ());
        QSize origSize = original.size ();
        QSize copySize = copy.size ();

        if (resize[i]) {
            QVERIFY(copySize.height() <= maxSize); 
            QVERIFY(copySize.width() <= maxSize); 
        } else {
            QVERIFY (origSize == copySize);
        }

        QVERIFY(media->removeCopyFile());
    }

    delete entry;
    entry = 0;
}

void LibWebUploadTests::checkImageResizeOptionSmall () {
    initResizeFields (WebUpload::IMAGE_RESIZE_SMALL);

    // Check media files
    QString parentPath = QDir::homePath() + "/MyDocs/.images/";
    QDir dir (parentPath);
    QVERIFY (dir.exists ());

    int maxSize = 640;

    WebUpload::Entry *entry = new WebUpload::Entry();
    entry->setImageResizeOption (WebUpload::IMAGE_RESIZE_SMALL);
    QVERIFY (entry->imageResizeOption() == WebUpload::IMAGE_RESIZE_SMALL);


    for (int i = 0; i < RESIZE_CASES_MAX; i++) {
        WebUpload::Media *media = NULL;
        createMedia (parentPath, resizeFiles[i], &media);
        QVERIFY (media != NULL);
        entry->appendMedia (media);
        QVERIFY(media->makeCopy () == WebUpload::Media::COPY_RESULT_SUCCESS);

        QUrl origPathUrl (media->origURI ());
        QImage original (origPathUrl.toLocalFile ());
        QImage copy (media->copyFilePath ());
        QSize origSize = original.size ();
        QSize copySize = copy.size ();

        if (resize[i]) {
            QVERIFY(copySize.height() <= maxSize); 
            QVERIFY(copySize.width() <= maxSize); 
        } else {
            QVERIFY (origSize == copySize);
        }

        QVERIFY(media->removeCopyFile());
    }

    delete entry;
    entry = 0;
}

void LibWebUploadTests::checkAccountPrivate () {
    AccountPrivate * priv = new AccountPrivate ();
    
    priv->m_accountId = 255;
    priv->m_serviceName = "Picasa";
    
    QString pres = priv->stringPresentation();
    QCOMPARE (pres, QString ("Accounts:ff:Picasa"));
    
    QVERIFY (priv->fromStringPresentation (pres));
    QCOMPARE (priv->m_accountId, (Accounts::AccountId)255);
    QCOMPARE (priv->m_serviceName, QString ("Picasa"));
    QString newPres = priv->stringPresentation();
    QCOMPARE (pres, newPres);
    
    priv->m_accountId = 423423;
    priv->m_serviceName = "Foo:Bar";
    pres = priv->stringPresentation();
    QCOMPARE (pres, QString ("Accounts:675ff:Foo:Bar"));

    QVERIFY (priv->fromStringPresentation (pres));
    QCOMPARE (priv->m_accountId, (Accounts::AccountId)423423);
    QCOMPARE (priv->m_serviceName, QString ("Foo:Bar"));
    newPres = priv->stringPresentation();
    QCOMPARE (pres, newPres);
    
    pres = "Invalid";
    QVERIFY (!(priv->fromStringPresentation (pres)));
    QVERIFY (!(priv->fromStringPresentation (QString())));
    
    delete priv;
}

#if 0
void LibWebUploadTests::systemChecks() {
    WebUpload::System system;
    
    QVERIFY (!system.getServiceDefinitionsPath().isEmpty());
    
    QList<QSharedPointer<WebUpload::Service> > * services = system.services();
    QVERIFY (services != 0);
    delete services;
    
    system.setServiceDefinitionsPath ("/usr/share/libwebupload-tests");
    QCOMPARE (system.getServiceDefinitionsPath(),
        QString ("/usr/share/libwebupload-tests"));
    services = system.services();
    QVERIFY (services != 0);
    QCOMPARE (services->size(), 5);
    
    QVERIFY (services->at(0) != 0);
    QVERIFY (services->at(0)->valueId ("sdfkewroweropfsdfods").isEmpty());
    QVERIFY (services->at(0)->getDefinitionPath().startsWith (
        "/usr/share/libwebupload-tests"));
    QVERIFY (!(services->at(0)->webUploadPluginName().isEmpty()));
    
    delete services;
    
    WebUpload::Service * service = system.service ("valid", this);
    QVERIFY (service != 0);    
    delete service; 
    
    QString entryPath = system.getEntryOutputPath ();
    QVERIFY (!entryPath.isEmpty());
    system.setEntryOutboxPath ("/tmp");
    QCOMPARE (system.getEntryOutputPath(), QString ("/tmp"));
    
    // System serialize
    Entry * entry = new Entry();
    QString outPath = system.serializeEntryToOutbox (entry);
    // With no medias this should fail!
    QVERIFY (outPath.isEmpty());
    QCOMPARE (outPath, entry->serializedTo());
    delete entry;
    
    system.setEntryOutboxPath (entryPath);
    
    // Try account loading
    createAccounts ();
    QList<QSharedPointer<WebUpload::Account> > * accounts = system.accounts();
    for (int i = 0; i < accounts->size(); i++) {
        Accounts::AccountId curr_id = accounts->at(i)->id();
        // Index will be 0 for first element and -1 if not found. 
        QVERIFY (accountList.indexOf (curr_id) <= 0);
    }
    delete accounts;
}
#endif

inline void LibWebUploadTests::createMedia (const QString & path, 
        const QString & file, WebUpload::Media **mediaP) {
    QVERIFY(mediaP);

    QString mediaPath = path;
    mediaPath.append (file);
    qDebug () << "Use media file" << mediaPath << "in test";
    QVERIFY (QFile::exists (mediaPath));

    Media * media = new Media();
    QVERIFY (media != 0);

    QVERIFY(!media->initFromTrackerIri(QString()));
    QVERIFY(media->initFromTrackerIri(mediaPath));
    QVERIFY (media->fileName() == file);

    QString uri = "file://";
    uri.append (mediaPath);

    QVERIFY (media->origURI() == uri);

    QVERIFY (media->mimeType() == "image/jpeg");

    *mediaP = media;
}

inline void LibWebUploadTests::createEntry (const QString & path) {

    // Check media files
    QString parentPath = QDir::homePath() + "/MyDocs/.images/";
    QDir dir (parentPath);
    QVERIFY (dir.exists ());
    
    // Create entry
    Entry * entry = new Entry();
    QVERIFY(entry != 0);

    //TODO: uses fake account
    entry->setAccountId("facebook");
    QVERIFY(entry->accountId() == "facebook");
    
    Media * media1 = NULL;
    createMedia (parentPath, "libwebupload-test1.jpg", &media1);
    QVERIFY(media1 != NULL);
    media1->setTitle("Picture1");
    QVERIFY(media1->title() == "Picture1");
    media1->setDescription ("1st picture in the transfer");
    QVERIFY(media1->description() == "1st picture in the transfer");
    QVERIFY(media1->tags().count() == 0);
    media1->appendTag("Tag1");
    media1->appendTag("Tag2");
    QVERIFY(media1->tags().count() == 2);
    QVERIFY(!media1->isSent());

    entry->appendMedia(media1);

    QVERIFY(!media1->title().isEmpty());
    QVERIFY(!media1->description().isEmpty());
    QVERIFY(!media1->tags().count() == 0);

    entry->setMetadataFilter(WebUpload::METADATA_FILTER_ALL);
    QVERIFY(entry->metadataFilterOption() != 0);
    QVERIFY(entry->checkShareFilter(WebUpload::METADATA_FILTER_ALL));

    entry->setMetadataFilter(WebUpload::METADATA_FILTER_NONE);
    QVERIFY(entry->metadataFilterOption() == 0);
    QVERIFY(entry->checkShareFilter(WebUpload::METADATA_FILTER_NONE));

    entry->setMetadataFilter(WebUpload::METADATA_FILTER_AUTHOR_LOCATION);
    QVERIFY(entry->metadataFilterOption() != 0);
    QVERIFY(entry->checkShareFilter(WebUpload::METADATA_FILTER_AUTHOR_LOCATION));

    QVERIFY(!media1->title().isEmpty());
    qDebug() << "Description is " << media1->description();
    QVERIFY(!media1->description().isEmpty());
    QVERIFY(media1->tags().count() == 2);
    
    Media * media2 = NULL;
    createMedia (parentPath, "libwebupload-test2.jpg", &media2);
    QVERIFY(media2 != NULL);
    media2->setTitle("Picture2");
    media2->setDescription("2nd picture in the transfer");
    QVERIFY(!media2->title().isEmpty());
    QVERIFY(!media2->description().isEmpty());
    entry->appendMedia(media2);
    QVERIFY(!media2->title().isEmpty());
    QVERIFY(!media2->description().isEmpty());

    entry->setMetadataFilter(WebUpload::METADATA_FILTER_NONE);
    
    QVERIFY (!entry->reSerialize());    
    
    QVERIFY (entry->serialize (path));
    checkFilePaths << path;    
    
    QString trackerUrl = entry->trackerIRI();
    qDebug() << "Tracker URI is " << trackerUrl;
    cleanTrackerUris << trackerUrl;
    delete entry;
}

inline QSparqlResult * LibWebUploadTests::blockingSparqlQuery (
    const QSparqlQuery &query, bool singleResponse) {

    if (m_sparqlConnection == 0) {
        m_sparqlConnection = new QSparqlConnection ("QTRACKER");
        if (!m_sparqlConnection->isValid ()) {
            qDebug() << "Could not create valid QSparqlConnection";
            delete m_sparqlConnection;
            m_sparqlConnection = 0;
            return 0;
        }
    }

    QSparqlResult * result = m_sparqlConnection->exec (query);
    result->waitForFinished ();

    if (result->hasError ()) {
        qDebug() << "Error with query" << query.preparedQueryText() << ":" << 
            result->lastError().message ();
        delete result;
        return 0;
    } else if ((singleResponse == true) && (result->size () != 1)) {
        // There should be exactly 1 row in the response if it was a valid
        // tracker iri
        qDebug() << query.preparedQueryText() << "returned" << 
            result->size() << "rows";
        delete result;
        return 0;
    }

    return result;
}


void LibWebUploadTests::checkServicePrivate() {
// TODO: NEED TO SEE HOW TO HANDLE THIS

    WebUpload::ServicePrivate * obj = new WebUpload::ServicePrivate (0);
    
    QCOMPARE (obj->m_name, QString());
    QVERIFY (obj->m_service == 0); 
    
    obj->clear();
    obj->clear();
    obj->clear();
    
    QVERIFY (!obj->validDataExists());
    QVERIFY (!obj->initFromDefinition (QDomElement()));
    QVERIFY (!obj->loadServiceOptions ());
    QVERIFY (obj->m_serviceOptionsLoaded == false);
    
    //Invalid xml
    QDomDocument dom ("test");
    QFile invalid_file ("/usr/share/libwebupload-tests/no_option_id.service.xml");
    QVERIFY (invalid_file.open(QIODevice::ReadOnly));
    QVERIFY (dom.setContent(&invalid_file));
    invalid_file.close();
    QVERIFY (obj->initFromDefinition (dom.documentElement ()));
    QCOMPARE (obj->m_uploadPlugin, QString ("plugin name"));
    QVERIFY (obj->m_serviceOptionsLoaded == false);
    QCOMPARE (obj->m_maxMedia, (unsigned int)0);
    //TODO: Option loading!
    delete obj;
    
    //Proper content
    obj = new WebUpload::ServicePrivate (0);
    QFile file("/usr/share/libwebupload-tests/valid.service.xml");
    QVERIFY (file.open(QIODevice::ReadOnly));
    QVERIFY (dom.setContent(&file));
    file.close();
    QVERIFY (obj->initFromDefinition (dom.documentElement ()));
    QVERIFY (obj->m_serviceOptionsLoaded == false);
    QCOMPARE (obj->m_maxMedia, (unsigned int)0);

    WebUpload::CommonOption * titleOpt = 0, * descOpt = 0, * tagsOpt = 0;
    QListIterator<PostOption *> iter (obj->m_postOptions);
    while (iter.hasNext ()) {
        PostOption * option = iter.next ();
        switch (option->type ()) {
            case PostOption::OPTION_TYPE_TITLE:
                titleOpt = qobject_cast <CommonTextOption *> (option);
                break;

            case PostOption::OPTION_TYPE_DESC:
                descOpt = qobject_cast <CommonTextOption *> (option);
                break;

            case PostOption::OPTION_TYPE_TAGS:
                tagsOpt = qobject_cast <CommonTextOption *> (option);
                break;

            default:
                break;
        }
    }

    QVERIFY (titleOpt);
    QVERIFY (titleOpt->caption() == WebUpload::CommonOptionPrivate::defaultCaption(titleOpt->type()));

    QVERIFY (descOpt);
    QVERIFY (descOpt->caption() == WebUpload::CommonOptionPrivate::defaultCaption(descOpt->type()));

    QVERIFY (tagsOpt == 0);
    
    QDomNodeList optionsNodeList = dom.elementsByTagName ("postOptions");
    for (int i = 0; i < optionsNodeList.count(); ++i) {
        QDomNode node = optionsNodeList.at (i);
        if (!node.isElement()) {
            continue;
        }
        QDomElement element = node.toElement();
        obj->initOptions (element);
        break;
    }
    
    // This will be false since accounts() is not defined
    //QVERIFY (obj->m_serviceOptionsLoaded == true);
    QCOMPARE (obj->m_postOptions.count(), (2+2));
    QCOMPARE (obj->m_uploadPlugin, QString ("plugin name"));    
    
    delete obj;

    // Another valid xml - this time with more options set
    obj = new WebUpload::ServicePrivate (0);
    file.setFileName("/usr/share/libwebupload-tests/another.valid.service.xml");
    QVERIFY (file.open(QIODevice::ReadOnly));
    QVERIFY (dom.setContent(&file));
    file.close();
    QVERIFY (obj->initFromDefinition (dom.documentElement ()));
    QVERIFY (obj->m_serviceOptionsLoaded == false);
    QCOMPARE (obj->m_maxMedia, (unsigned int)4);

    iter = QListIterator<PostOption *>(obj->m_postOptions);
    while (iter.hasNext ()) {
        PostOption * option = iter.next ();
        switch (option->type ()) {
            case PostOption::OPTION_TYPE_TITLE:
                titleOpt = qobject_cast <CommonTextOption *> (option);
                break;

            case PostOption::OPTION_TYPE_DESC:
                descOpt = qobject_cast <CommonTextOption *> (option);
                break;

            case PostOption::OPTION_TYPE_TAGS:
                tagsOpt = qobject_cast <CommonTextOption *> (option);
                break;

            case PostOption::OPTION_TYPE_METADATA:
            case PostOption::OPTION_TYPE_IMAGE_RESIZE:
            case PostOption::OPTION_TYPE_VIDEO_RESIZE:
                // These will not be valid for either media or entry since
                // the options are invalid
                QVERIFY (option->validForMedia (0) == false);
                QVERIFY (option->validForEntry (0) == false);
                break;

            case PostOption::OPTION_TYPE_SERVICE:
            {
                ServiceOption *sOpt = qobject_cast <ServiceOption*> (option);
                QVERIFY (sOpt->id () == "album");
                QVERIFY (sOpt->isUpdatable ());
                QVERIFY (sOpt->isChangeable ());
                break;
            }

            default:
                break;
        }
    }

    QVERIFY (titleOpt);
    QVERIFY (titleOpt->caption() == "Caption for images and videos");
    
    CommonTextOption * textOpt = qobject_cast <CommonTextOption*> (titleOpt);
    QVERIFY (textOpt);
    QVERIFY (textOpt->prefill() == true);

    QVERIFY (descOpt);
    QVERIFY (descOpt->caption() == "Description for links");
    
    textOpt = qobject_cast <CommonTextOption*> (descOpt);
    QVERIFY (textOpt);
    QVERIFY (textOpt->prefill() == false);

    QVERIFY (tagsOpt == 0);
    
    optionsNodeList = dom.elementsByTagName ("postOptions");
    for (int i = 0; i < optionsNodeList.count(); ++i) {
        QDomNode node = optionsNodeList.at (i);
        if (!node.isElement()) {
            continue;
        }
        QDomElement element = node.toElement();
        obj->initOptions (element);
        break;
    }
    
    // This will be false since accounts() is not defined
    //QVERIFY (obj->m_serviceOptionsLoaded == true);
    QCOMPARE (obj->m_postOptions.count(), (2+2));
    QCOMPARE (obj->m_uploadPlugin, QString ("plugin name"));    
    
    delete obj;

}

inline void LibWebUploadTests::createAccounts () {
    Accounts::Account *acc;

    // First create one account with sharing enabled. Then two with sharing
    // disabled
    for (int i = 0; i < 3; i++) {
        Accounts::ServiceList sList;
        acc = accMgr.createAccount ("google");
        QVERIFY (acc != NULL);
        acc->setDisplayName (QUuid::createUuid().toString());
        acc->setEnabled (true);
        sList = acc->services ();
        for (int j = 0; j < sList.size(); j++) {
            acc->selectService (sList[j]);
            if ((i == 0) && (sList[j]->serviceType () == "sharing")) {
                acc->setEnabled (true);
            } else {
                acc->setEnabled (false);
            }
        }

        acc->sync ();
        QVERIFY(acc->id() != 0);
        accountList << acc->id();
    }
}

inline void LibWebUploadTests::cleanupAccounts () {
    Accounts::Account *acc;

    for (int i = 0; i < accountList.size (); i++) {
        acc = accMgr.account (accountList[i]);
        acc->remove ();
        acc->sync ();
    }

    accountList.clear ();
}

/* TODO: Write proper test here. Make valid compination of file and strings.
   Then read it and validate that it's combined correctly */
void LibWebUploadTests::testHttpMultiContentIO() {
    HttpMultiContentIO * multi = 0;
    QString payload = "";
    
    multi = new HttpMultiContentIO();
    QVERIFY(multi->open(QIODevice::ReadWrite));
    
    QVERIFY(multi->size() == 0);
    QVERIFY(multi->bytesAvailable() == 0);

    // Testing template related functions and setting the template finally to
    // "Testing %1 %2 %1"
    QVERIFY(multi->setDefaultTemplate("Testing %1 %2 %1"));
    QVERIFY(multi->defaultTemplate() == "Testing %1 %2 %1");
    multi->clearDefaultTemplate();
    QVERIFY(multi->defaultTemplate().isEmpty());
    QVERIFY(multi->setDefaultTemplate("Testing %1 %2 %1"));
    QVERIFY(multi->bytesAvailable() == 0);

    // Testing boundaryString related functions
    // Confirming that the boundary string is set in the constructor
    QVERIFY(!multi->boundaryString().isEmpty());
    QVERIFY(multi->setBoundaryString("123"));
    QCOMPARE(multi->boundaryString(), QString("123"));
    QVERIFY(multi->bytesAvailable() == 0);

    // First error case: Add string with argument to prefix boundary set to
    // true while there is no boundary
    QVERIFY(multi->setBoundaryString(""));
    QVERIFY(multi->boundaryString().isEmpty());
    QVERIFY(!multi->addString("Testing...", true));

    QVERIFY(multi->addString("Testing...", false));
    payload += "Testing...\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());
    
    QVERIFY(multi->setBoundaryString("123"));
    QCOMPARE(multi->boundaryString(), QString("123"));

    QVERIFY(multi->addString("Testing...", false));
    payload += "Testing...\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    QVERIFY(multi->addString("Testing...", true));
    payload += "--123\r\nTesting...\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    QStringList args;
    args << "1" << "2";
    QVERIFY(multi->addString(args, "Testing"));
    payload += "--123\r\nTesting 1 2 1\r\n\r\nTesting\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    args.clear();
    args << "1";
    QVERIFY(multi->addString(args, "Testing"));
    payload += "--123\r\nTesting 1 %2 1\r\n\r\nTesting\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    args.clear();
    args << "1" << "2" << "3";
    QVERIFY(multi->addString(args, "Testing", "Testing %1 %2 %3"));
    payload += "--123\r\nTesting 1 2 3\r\n\r\nTesting\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    QString testFilePath = QDir::homePath ();
    testFilePath.append ("/MyDocs/test.txt");
    qDebug() << "Using file" << testFilePath;

    QVERIFY (QFile::exists (testFilePath));
    QFile file(testFilePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray arr = file.readAll();
    qDebug() << arr;
    file.close ();

    args.clear();
    args << "1" << "2" << "3";
    QVERIFY(multi->addFile(args, testFilePath));
    payload += "--123\r\nTesting 1 2 1\r\n\r\n";
    payload += QString(arr);
    payload += "\r\n--123--\r\n\r\n";
    QCOMPARE(multi->bytesAvailable(), (long long)payload.length());

    multi->allDataAdded();
    QVERIFY(multi->bytesAvailable() == payload.length());

    QCOMPARE(multi->size(), multi->bytesAvailable());
    QVERIFY(!multi->boundaryString().isNull());
    QVERIFY(!multi->boundaryString().isEmpty());    
    multi->close ();

    QVERIFY(multi->open(QIODevice::ReadOnly));
    QString readValue = ""; 
    int step = 0;
    for(step += 10; step < payload.length(); step += 10)
    {
        QByteArray temp = multi->read(10);
        readValue += QString(temp);
        QVERIFY(payload.startsWith(readValue));
    }

    QVERIFY(!multi->addString("Testing...", false));

    multi->close();
    delete multi;
}

void LibWebUploadTests::testError() {    
    WebUpload::Error error = WebUpload::Error::connectFailure();
    QVERIFY(!error.canContinue());
    // QVERIFY(error.repairable());
    QVERIFY(!error.title().isEmpty());
    QVERIFY(!error.description().isEmpty());
    /**
    error.setDescription("Hello World");
    QVERIFY(!error.description().isEmpty());
    QVERIFY(error.description() == "Hello World");
     **/
    QString desc = error.description(); 

    error = WebUpload::Error::invalidFileType();        
    QVERIFY (desc != error.description());
    QVERIFY(!error.repairable());
    QVERIFY(error.canContinue());
}

void LibWebUploadTests::testErrorSerialization () {
    // First check with simple error where no strings need to be set
    WebUpload::Error src = WebUpload::Error::noConnection ();
    src.setData ((QVariant)(1));
    src.setFailedCount (100);
    src.setTransferFileCount (102);

    QByteArray array = src.serialize ();

    WebUpload::Error dstSimple (array);
    QVERIFY (dstSimple.code() == WebUpload::Error::CODE_NO_CONNECTION);
    const QVariant dataSimple = dstSimple.data ();
    QVERIFY (dataSimple.canConvert<int>());
    QVERIFY (dataSimple.toInt() == 1);
    QVERIFY (dstSimple.repairable () == true);
    QVERIFY (dstSimple.canContinue () == false);
    QVERIFY (dstSimple.failedCount () == 100);
    QVERIFY (dstSimple.transferFileCount () == 102);
    QVERIFY (dstSimple.serviceErrorString ().isEmpty());

    // Now check with something like service error
    src = WebUpload::Error::serviceError ("Test Error String");
    src.setData ((QVariant)(5));
    src.setFailedCount (7);
    src.setTransferFileCount (10);

    array = src.serialize ();

    WebUpload::Error dstServiceError (array);
    QVERIFY (dstServiceError.code() == WebUpload::Error::CODE_SERVICE_ERROR);
    const QVariant dataServiceError = dstServiceError.data ();
    QVERIFY (dataServiceError.canConvert<int>());
    QVERIFY (dataServiceError.toInt() == 5);
    QVERIFY (dstServiceError.repairable () == false);
    QVERIFY (dstServiceError.canContinue () == false);
    QVERIFY (dstServiceError.failedCount () == 7);
    QVERIFY (dstServiceError.transferFileCount () == 10);
    QVERIFY (dstServiceError.serviceErrorString ().
        compare ("Test Error String") == 0);

    // Now check for custom error
    src = WebUpload::Error::custom ("Msg", "Desc", "Retry", true);

    array = src.serialize ();

    WebUpload::Error dstCustomError (array);
    QVERIFY (dstCustomError.code() == WebUpload::Error::CODE_CUSTOM);
    const QVariant dataCustomError = dstCustomError.data ();
    QVERIFY (dataCustomError.isNull());
    QVERIFY (dstCustomError.repairable () == true);
    QVERIFY (dstCustomError.canContinue () == true);
    QVERIFY (dstCustomError.failedCount () == 1);
    QVERIFY (dstCustomError.transferFileCount () == 0);
    QVERIFY (dstCustomError.title ().compare ("Msg") == 0);
    QVERIFY (dstCustomError.description ().compare ("Desc") == 0);
    QVERIFY (dstCustomError.recoverMsg ().compare ("Retry") == 0);
}

void LibWebUploadTests::testXmlHelper() {
    QDomDocument dom ("test");
    QFile testFile ("/usr/share/libwebupload-tests/mimetypes.xml");
    QVERIFY (testFile.open(QIODevice::ReadOnly));
    QVERIFY (dom.setContent (&testFile));
    testFile.close();

    QDomNodeList list = dom.elementsByTagName ("formats");
    QCOMPARE (list.count(), 8); //Update if you change xml
    for (int i = 0; i < list.count(); ++i) {
        QDomNode node = list.at (i);
        QVERIFY (node.isElement());
        QDomElement element = node.toElement();
        
        QStringList mimes = XmlHelper::readMimeTypes (element);
        QString type = element.attribute ("type");
        
        if (type.contains ("empty")) {
            QCOMPARE (mimes.count(), 0);
        } else {
            QVERIFY (mimes.count() > 0);
        }
        
        if (type.contains ("all") || type.contains ("empty") ||
            type.contains ("both")) {
            
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/jpeg", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/png", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/gif", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("video/mp4", mimes));
            
            bool acceptPDF = type.contains ("all") || type.contains ("empty");
            QVERIFY (XmlHelper::mimeTypeListCheck ("application/pdf", mimes)
                == acceptPDF);
                    
        } else if (type.contains ("image")) {
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/jpeg", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/png", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/gif", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("video/mp4", mimes));            
            QVERIFY (!XmlHelper::mimeTypeListCheck ("application/pdf", mimes));
                     
        } else if (type.contains ("video")) {
            QVERIFY (XmlHelper::mimeTypeListCheck ("video/mp4", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/jpeg", mimes));            
            QVERIFY (!XmlHelper::mimeTypeListCheck ("application/pdf", mimes));
            
        } else if (type.contains ("jpeg")) {
            QVERIFY (XmlHelper::mimeTypeListCheck ("image/jpeg", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/jpg", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/png", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("application/pdf", mimes));
                              
        } else if (type.contains ("pdf")) {
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/jpeg", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/png", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("image/gif", mimes));
            QVERIFY (!XmlHelper::mimeTypeListCheck ("video/mp4", mimes));
            QVERIFY (XmlHelper::mimeTypeListCheck ("application/pdf", mimes));
        }
    }
}

void LibWebUploadTests::testProcessExchangeData () {
    ProcessExchangeData pData;
    
    // No need to actually check all of them
    QSignalSpy feSpy(&pData, SIGNAL(startUploadSignal(QString,WebUpload::Error)));
    QSignalSpy uaSpy(&pData, SIGNAL(updateAllSignal(QString)));
    QSignalSpy avSpy(&pData, SIGNAL(addValueSignal(QString,QString,QString)));
    QSignalSpy smSpy(&pData, SIGNAL(sendingMediaSignal(quint32)));
    QSignalSpy uploadFailedSpy (&pData, SIGNAL(uploadFailedSignal(WebUpload::Error)));
    QSignalSpy progressSpy(&pData, SIGNAL(progressSignal(float)));
    QSignalSpy customSpy (&pData, SIGNAL(customRequestSignal(QByteArray)));

    QList<QVariant> spyArgs;
    QVariant firstArg, secondArg;

    QByteArray feArray = pData.startUpload ("/tmp/path", 
        WebUpload::Error::noConnection());
    quint32 feArraySize = feArray.size ();

    QByteArray uaArray = pData.updateAll ("ACCOUNT");
    QByteArray avArray = pData.addValue ("ACCOUNT", "optionId", "valueName");
    QByteArray smArray = pData.sendingMedia (5);
    QByteArray progressArray = pData.progress (0.765);
    QByteArray customArray = pData.customRequest (feArray);
    QByteArray ufArray = pData.uploadFailed (WebUpload::Error::targetDoesNotExist());

    QByteArray allRequests (uaArray);
    allRequests.append (avArray);
    allRequests.append (smArray);
    allRequests.append (progressArray);

    QByteArray inputArray (customArray);
    QByteArray restArray;
    int chopPoint = customArray.size() / 2;
    inputArray.chop (chopPoint);
    restArray = customArray.right (chopPoint);
    restArray.append (uaArray);

    pData.processByteArray (inputArray);
    QCOMPARE(uaSpy.count(), 0);
    QCOMPARE(feSpy.count(), 0);
    QCOMPARE(avSpy.count(), 0);
    QCOMPARE(smSpy.count(), 0);
    QCOMPARE(progressSpy.count(), 0);
    QCOMPARE(customSpy.count(), 0);

    pData.processByteArray (restArray);
    QCOMPARE(feSpy.count(), 0);
    QCOMPARE(uaSpy.count(), 1);
    QCOMPARE(avSpy.count(), 0);
    QCOMPARE(smSpy.count(), 0);
    QCOMPARE(progressSpy.count(), 0);
    QCOMPARE(customSpy.count(), 1);

    spyArgs = uaSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<QString>() == true);
    QVERIFY (firstArg.value<QString>().compare ("ACCOUNT") == 0);

    spyArgs = customSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert <QByteArray>() == true);
    QByteArray customArg = firstArg.toByteArray ();
    QCOMPARE ((quint32)customArg.size(), feArraySize);
    QVERIFY (customArg.contains (feArray));

    pData.processByteArray (customArg);
    QCOMPARE(feSpy.count(), 1);
    QCOMPARE(uaSpy.count(), 0);
    QCOMPARE(avSpy.count(), 0);
    QCOMPARE(smSpy.count(), 0);
    QCOMPARE(progressSpy.count(), 0);
    QCOMPARE(customSpy.count(), 0);

    spyArgs = feSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 2);
    firstArg = spyArgs[0];
    secondArg = spyArgs[1];
    QVERIFY (firstArg.canConvert<QString>() == true);
    QVERIFY (firstArg.value<QString>().compare ("/tmp/path") == 0);
    QVERIFY (secondArg.canConvert<WebUpload::Error>() == true);
    QVERIFY (secondArg.value<WebUpload::Error>().code() == WebUpload::Error::CODE_NO_CONNECTION);

    pData.processByteArray (allRequests);
    QCOMPARE(feSpy.count(), 0);
    QCOMPARE(uaSpy.count(), 1);
    QCOMPARE(avSpy.count(), 1);
    QCOMPARE(smSpy.count(), 1);
    QCOMPARE(progressSpy.count(), 1);
    QCOMPARE(customSpy.count(), 0);

    spyArgs = uaSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<QString>() == true);
    QVERIFY (firstArg.value<QString>().compare ("ACCOUNT") == 0);

    spyArgs = avSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 3);
    QVariant thirdArg;
    firstArg = spyArgs[0];
    secondArg = spyArgs[1];
    thirdArg = spyArgs [2];
    QVERIFY (firstArg.canConvert<QString>());
    QVERIFY (firstArg.value<QString>().compare ("ACCOUNT") == 0);
    QVERIFY (secondArg.canConvert<QString>());
    QVERIFY (secondArg.value<QString>().compare ("optionId") == 0);
    QVERIFY (thirdArg.canConvert<QString>());
    QVERIFY (thirdArg.value<QString>().compare ("valueName") == 0);

    spyArgs = smSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<int>());
    QCOMPARE (firstArg.value<int>(), 5);

    spyArgs = progressSpy.takeFirst ();
    QCOMPARE (spyArgs.count(), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<float>());
    qDebug() << firstArg.value<float>();
    QVERIFY (firstArg.value<float>() == (float)0.765);

    pData.processByteArray (ufArray);
    QCOMPARE(feSpy.count(), 0);
    QCOMPARE(uaSpy.count(), 0);
    QCOMPARE(avSpy.count(), 0);
    QCOMPARE(smSpy.count(), 0);
    QCOMPARE(progressSpy.count(), 0);
    QCOMPARE(customSpy.count(), 0);
    QCOMPARE(uploadFailedSpy.count(), 1);
    spyArgs = uploadFailedSpy.takeFirst ();
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<WebUpload::Error>() == true);
    QVERIFY (firstArg.value<WebUpload::Error>().code() == WebUpload::Error::CODE_TARGET_DOES_NOT_EXIST);
}


void LibWebUploadTests::testPost () {
    DummyPost postInst (0);
    WebUpload::Error wError;
    QSignalSpy errorSpy (&postInst, SIGNAL (error(WebUpload::Error)));
    QSignalSpy doneSpy (&postInst, SIGNAL (done()));
    QSignalSpy mediaStartedSpy (&postInst, 
        SIGNAL (mediaStarted(WebUpload::Media*)));
    QSignalSpy errorFixedSpy (&postInst, SIGNAL (errorFixed()));
    QSignalSpy errorFixFailedSpy (&postInst, 
        SIGNAL (errorFixFailed(WebUpload::Error)));
    QList<QVariant> spyArgs;
    QVariant firstArg;

    // Since entry has not been set yet
    QVERIFY (postInst.unsentCount() == 0);

    createEntry (TEMP_ENTRY_PATH);
    Entry * entry = new Entry ();

    postInst.upload (entry, wError);
    QCOMPARE (doneSpy.count (), 1);
    doneSpy.clear ();

    wError = WebUpload::Error::fileError ();
    postInst.upload (entry, wError);
    QCOMPARE (errorFixFailedSpy.count (), 1);
    spyArgs = errorFixFailedSpy.takeFirst ();
    QCOMPARE (spyArgs.count (), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<WebUpload::Error>() == true);
    WebUpload::Error errorGot = firstArg.value <WebUpload::Error>();
    QVERIFY (errorGot.code () == wError.code ());
    errorFixFailedSpy.clear ();

    wError = WebUpload::Error::dateTimeError ();
    postInst.upload (entry, wError);
    QCOMPARE (errorFixedSpy.count (), 1);
    errorFixedSpy.clear ();
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    QCOMPARE (doneSpy.count (), 1);
    doneSpy.clear ();

    errorSpy.clear ();
    entry->init (TEMP_ENTRY_PATH);
    wError.clearError ();
    postInst.upload (entry, wError);
    QCOMPARE (mediaStartedSpy.count (), 1);
    mediaStartedSpy.clear ();
    // Since auth ptr is NULL, auth will fail
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    QCOMPARE (errorSpy.count (), 1);
    spyArgs = errorSpy.takeFirst ();
    QCOMPARE (spyArgs.count (), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<WebUpload::Error>() == true);
    errorGot = firstArg.value <WebUpload::Error>();
    QVERIFY (errorGot.code() == WebUpload::Error::CODE_TRANSFER_FAILED);
    errorSpy.clear ();

    postInst.setEntry (entry);
    QVERIFY (postInst.unsentCount () == entry->mediaCount ());

    // Now checking the percent handling
    QSignalSpy progressSpy(&postInst, SIGNAL (progress(float)));
    postInst.nrUpProgress (-1, 1);
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    // We don't seem to emit progress of 0.0
    QCOMPARE (progressSpy.count (), 0);

    postInst.nrUpProgress (0, 1);
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    // We don't seem to emit progress of 0.0
    QCOMPARE (progressSpy.count (), 0);

    postInst.nrUpProgress (1, 2);
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    QCOMPARE (progressSpy.count (), 1);
    spyArgs = progressSpy.takeFirst ();
    QCOMPARE (spyArgs.count (), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<float>() == true);
    qDebug() << firstArg.toFloat ();
    QVERIFY (firstArg.toFloat () > 0.0);
    QVERIFY (firstArg.toFloat () <= 1.0);
    progressSpy.clear ();

    postInst.nrUpProgress (1, 1);
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    QCOMPARE (progressSpy.count (), 1);
    spyArgs = progressSpy.takeFirst ();
    QCOMPARE (spyArgs.count (), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<float>() == true);
    QVERIFY (firstArg.toFloat () > 0.0);
    QVERIFY (firstArg.toFloat () <= 1.0);
    progressSpy.clear ();

    postInst.nrUpProgress (2, 1);
    if (QCoreApplication::hasPendingEvents ()) {
        QCoreApplication::processEvents ();
    }
    QCOMPARE (progressSpy.count (), 1);
    spyArgs = progressSpy.takeFirst ();
    QCOMPARE (spyArgs.count (), 1);
    firstArg = spyArgs[0];
    QVERIFY (firstArg.canConvert<float>() == true);
    QVERIFY (firstArg.toFloat () > 0.0);
    QVERIFY (firstArg.toFloat () <= 1.0);
    progressSpy.clear ();

    entry->cancel ();
    delete entry;
}

QTEST_MAIN(LibWebUploadTests)
