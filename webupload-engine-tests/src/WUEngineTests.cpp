 
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




#include "WUEngineTests.h"
#include "uploaditem.h"
#include "uploadqueue.h"
#include "uploadstatistics.h"
#include <QtTest/QtTest>
#include <QFile>

#include <QSignalSpy>

#include "WebUpload/Entry"
#include "WebUpload/Media"

#include <QtSparql>
#include <unistd.h>

using namespace WebUpload;

void WUEngineTests::setupSharingEntry(QString xmlPath) {
    Entry *entry = 0;
    Media *media = 0;
    QString testImgPath = QDir::homePath() + "/MyDocs/.images/";
    
    entry = new Entry();
    entry->setAccountId("test-plugin");
    
    media = new Media();
    QString imgFile1 = testImgPath + "webupload-engine-test1.jpg";
    if(!media->initFromTrackerIri (imgFile1)) {
        QFAIL("Could not init media with webupload-engine-test1.jpg");
    }
    media->setTitle("Picture 1");
    media->setDescription("Showing picture 1");
    // TODO: media->appendTag("Tag1");
    entry->appendMedia(media);

    media = new Media();
    QString imgFile2 = testImgPath + "webupload-engine-test2.jpg";
    if(!media->initFromTrackerIri (imgFile1)) {
        QFAIL("Could not init media with webupload-engine-test2.jpg");
    }

    media->setTitle("Picture 2");
    // TODO: media->appendTag("Tag2");
    entry->appendMedia(media);
    
    // Serialize
    if(!entry->serialize(xmlPath)) {
        qDebug() << "Tried to serialize to " << xmlPath;
        QFAIL("Serialization failed");
    }
    entry->reSerialize();
    
    delete entry;
}

void WUEngineTests::testUploadItem() {
    UploadItem *item = new UploadItem();
    QString xmlPath = QDir::homePath();

    // Passing an invalid path
    QVERIFY(!item->init(xmlPath + "/no_such_entrentry.xml"));
    QVERIFY(!item->getEntry());

    setupSharingEntry(xmlPath + "/entry.xml");
    QVERIFY(item->init(xmlPath + "/entry.xml"));

    Entry *entry = item->getEntry();
    QVERIFY(entry != NULL);
    // No upload has started yet, so there should not be any error.
    WebUpload::Error error = item->getError();

    QVERIFY(error.code() == WebUpload::Error::CODE_NO_ERROR);
    QVERIFY(!item->isCancelled());

    item->markFailed(WebUpload::Error::invalidFileType());
    error = item->getError();
    QVERIFY(error.code() == WebUpload::Error::CODE_INV_FILE_TYPE);

    // Reinit should not work
    QVERIFY(!item->init(xmlPath + "/entry.xml"));
    // Since the init failed, the error should still be there
    error = item->getError();
    QVERIFY(error.code() == WebUpload::Error::CODE_INV_FILE_TYPE);

    // Write another error. See if it overwrites the previous one, or gives
    // error
    item->markFailed(WebUpload::Error::connectFailure());
    error = item->getError();
    QVERIFY(error.code() != WebUpload::Error::CODE_INV_FILE_TYPE);
    QVERIFY(error.code() == WebUpload::Error::CODE_CONNECT_FAILURE);

    QVERIFY(QFile::remove(xmlPath + "/entry.xml"));

    QSparqlQuery rem ("DELETE { ?:te a rdf:Resource . } WHERE "
        "{ ?:te a rdf:Resource . }", QSparqlQuery::DeleteStatement);
    rem.bindValue ("te", QUrl (item->getEntry()->trackerIRI()));
    // Not checking for errors here - does not help
    QSparqlConnection connection ("QTRACKER");
    if (connection.isValid()) {
        QSparqlResult * result = connection.exec (rem);
        result->waitForFinished ();
        delete result;
    }
    delete item;
}

void WUEngineTests::testUploadQueue() {
    qRegisterMetaType<UploadItem *>("UploadItem *");
    UploadQueue *queue = new UploadQueue;
    QSignalSpy spyTopItem(queue, SIGNAL(topItem(UploadItem *)));
    QSignalSpy spyReplacedItem(queue, SIGNAL(replacedTopItem(UploadItem *, 
                                                             UploadItem *)));
    QSignalSpy spyDone(queue, SIGNAL(done()));
    QList<QVariant> spyArgs;
    QString xmlPath = QDir::homePath();

    UploadItem *item1 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry1.xml");
    item1->init(xmlPath + "/entry1.xml");

    UploadItem *item2 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry2.xml");
    item2->init(xmlPath + "/entry2.xml");

    UploadItem *item3 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry3.xml");
    item3->init(xmlPath + "/entry3.xml");

    QVERIFY(queue->size() == 0);

    QVERIFY(!queue->push(NULL));
    queue->removeItem(item1);
    QVERIFY(queue->size() == 0);

    QVERIFY(queue->push(item1));
    QVERIFY(queue->size() == 1);
    QCOMPARE(spyTopItem.count(), 1);
    spyArgs = spyTopItem.takeFirst();
    QVERIFY(spyArgs.count() == 1);
    QVERIFY(!spyArgs.at(0).isNull());
    // Need to figure out how to get the argument and compare and confirm that
    // the value is 1.

    QVERIFY(queue->push(item2));
    QVERIFY(queue->size() == 2);
    // Confirming that no more topItem signals have been emitted yet. takeFirst
    // removes the element.
    QCOMPARE(spyTopItem.count(), 0);

    QVERIFY(queue->push(item3));
    QVERIFY(queue->size() == 3);
    QCOMPARE(spyTopItem.count(), 0);

    QVERIFY(queue->getTop() == item1);

    queue->riseItem(item3);
    QVERIFY(queue->getTop() == item3);
    QCOMPARE(spyTopItem.count(), 0);
    QCOMPARE(spyReplacedItem.count(), 1);
    spyArgs = spyReplacedItem.takeFirst();
    QVERIFY(spyArgs.count() == 2);
    QVERIFY(!spyArgs.at(0).isNull());
    QVERIFY(!spyArgs.at(1).isNull());

    QSparqlQuery rem ("DELETE { ?:te a rdf:Resource . } WHERE "
        "{ ?:te a rdf:Resource . }", QSparqlQuery::DeleteStatement);
    rem.bindValue ("te", QUrl (item3->getEntry()->trackerIRI()));
    // Not checking for errors here - does not help
    QSparqlConnection connection ("QTRACKER");
    if (connection.isValid()) {
        QSparqlResult * result = connection.exec (rem);
        result->waitForFinished ();
        delete result;
    }
    queue->removeItem(item3);
    QVERIFY(queue->size() == 2);
    QVERIFY(queue->getTop() == item1);
    QCOMPARE(spyTopItem.count(), 1);
    spyArgs = spyTopItem.takeFirst();
    QVERIFY(spyArgs.count() == 1);
    QVERIFY(!spyArgs.at(0).isNull());

    // try to rise a removed item. nothing should happen
    queue->riseItem(item3);
    QVERIFY(queue->size() == 2);
    QVERIFY(queue->getTop() == item1);
    QCOMPARE(spyTopItem.count(), 0);
    QCOMPARE(spyReplacedItem.count(), 0);

    // try to rise first item. nothing should happen
    queue->riseItem(item1);
    QVERIFY(queue->size() == 2);
    QVERIFY(queue->getTop() == item1);
    QCOMPARE(spyTopItem.count(), 0);
    QCOMPARE(spyReplacedItem.count(), 0);

    // Remove same item again. Should not make any difference
    queue->removeItem(item3);
    QVERIFY(queue->size() == 2);
    QVERIFY(queue->getTop() == item1);

    rem.unbindValues ();
    rem.bindValue ("te", QUrl (item2->getEntry()->trackerIRI()));
    // Not checking for errors here - does not help
    if (connection.isValid()) {
        QSparqlResult * result = connection.exec (rem);
        result->waitForFinished ();
        delete result;
    }
    queue->removeItem(item2);
    QVERIFY(queue->size() == 1);
    QVERIFY(queue->getTop() == item1);
    QCOMPARE(spyTopItem.count(), 0);

    rem.unbindValues ();
    rem.bindValue ("te", QUrl (item1->getEntry()->trackerIRI()));
    // Not checking for errors here - does not help
    if (connection.isValid()) {
        QSparqlResult * result = connection.exec (rem);
        result->waitForFinished ();
        delete result;
    }
    queue->removeItem(item1);
    QVERIFY(queue->size() == 0);
    QVERIFY(queue->getTop() == NULL);
    QCOMPARE(spyTopItem.count(), 0);
    QCOMPARE(spyDone.count(), 1);
    spyArgs = spyDone.takeFirst();
    QVERIFY(spyArgs.count() == 0);

    item1 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry1.xml");
    item1->init(xmlPath + "/entry1.xml");
    QVERIFY(queue->push(item1));

    item2 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry2.xml");
    item2->init(xmlPath + "/entry2.xml");
    QVERIFY(queue->push(item2));

    item3 = new UploadItem(queue);
    setupSharingEntry(xmlPath + "/entry3.xml");
    item3->init(xmlPath + "/entry3.xml");
    QVERIFY(queue->push(item3));

    QSparqlQuery remAll ("DELETE { ?:t1 a rdf:Resource . ?:t2 a rdf:Resource . "
        "?:t3 a rdf:Resource . } WHERE { ?:t1 a rdf:Resource . "
        "?:t2 a rdf:Resource . ?:t3 a rdf:Resource . }",
        QSparqlQuery::DeleteStatement);
    remAll.bindValue ("t1", QUrl (item1->getEntry()->trackerIRI()));
    remAll.bindValue ("t2", QUrl (item2->getEntry()->trackerIRI()));
    remAll.bindValue ("t3", QUrl (item3->getEntry()->trackerIRI()));
    // Not checking for errors here - does not help
    if (connection.isValid()) {
        QSparqlResult * result = connection.exec (remAll);
        result->waitForFinished ();
        delete result;
    }
    delete queue;

    QVERIFY(QFile::remove(xmlPath + "/entry1.xml"));
    QVERIFY(QFile::remove(xmlPath + "/entry2.xml"));
    QVERIFY(QFile::remove(xmlPath + "/entry3.xml"));
}

void WUEngineTests::testStatistics() {
    UploadStatistics stat;
    
    QCOMPARE (stat.seconds(), -1);
    
    stat.setSize (100000);
    
    qDebug() << "Setting nowDone to 0.0";
    stat.nowDone (0.0);
    usleep (100000);
    qDebug() << "Setting nowDone to 0.1";
    stat.nowDone (0.1);

    usleep (1230000);
    qDebug() << "Setting nowDone to 0.15";
    stat.nowDone (0.15);
    QCoreApplication::processEvents();
    qDebug() << stat.seconds();
    QVERIFY (stat.seconds() > 13); // Should be ~19    

    usleep (2000000);
    stat.nowDone (0.2);
    qDebug() << stat.seconds();
    QCoreApplication::processEvents();
    QVERIFY (stat.seconds() < 40); // Should be ~29    

    usleep (2000001);
    stat.nowDone (0.4);
    qDebug() << stat.seconds();
    QCoreApplication::processEvents();
    qDebug() << stat.seconds();
    QVERIFY (stat.seconds() > 8); // Should be ~12
    
    usleep (3000);
    stat.nowDone (0.5);
    usleep (1000000);
    
    stat.nowDone (0.99);
    qDebug() << stat.seconds();
    QCoreApplication::processEvents();
    QVERIFY (stat.seconds() < 7); // Should be ~4
}



QTEST_MAIN(WUEngineTests)