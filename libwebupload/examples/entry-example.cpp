 
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



 
/*!
 * \example entry-example.cpp
 * This is an example of how to build new entry and sent to be uploaded via
 * webupload engine. This is needed if you want to build up transfers in your
 * own application and still use Webupload Engine's framework to upload files.
 */

#include <QObject>
#include <QString>
#include <WebUpload/System>
#include <WebUpload/Account>
#include <WebUpload/Entry>
#include <WebUpload/Media>
#include <QDebug>

class EntryBuilder : public QObject {
    Q_OBJECT
public:
    EntryBuilder (QObject * parent = 0);
    virtual ~EntryBuilder();

public Q_SLOTS:

    //constructs entry with one file and sents it to the upload engine
    bool generateUpload (const QString & fileTrackerIri);
};

EntryBuilder::EntryBuilder (QObject * parent) : QObject (parent) {

}

EntryBuilder::~EntryBuilder () {
}

bool EntryBuilder::generateUpload (const QString & fileTrackerIri) {

    //Entry built
    WebUpload::Entry entry;

    // Load webupload suitable accounts from the system
    QList <QSharedPointer<WebUpload::Account> > * accounts = 
        WebUpload::System::accounts();
    
    if (accounts->isEmpty()) {
        qWarning() << "No suitable accounts found";
        delete accounts;
        return false;
    }
    
    // Select first account from the list and set it to the entry
    QSharedPointer<WebUpload::Account> account = accounts->at(0);
    delete accounts;
    entry.setAccount (account.data());
    
    // Initilize media from file Tracker IRI
    WebUpload::Media * media = new WebUpload::Media ();
    if (media->init (fileTrackerIri) == false) {
        qCritical() << "Failed to initialize media from file Tracker Iri";
        delete media;
        return false;
    }
    
    // Set metadata for the media
    media->setTitle ("Hello World");
    media->setDescription ("Lorem Ipsum");
    media->clearTags ();
    media->appendTag ("Foo");
    media->appendTag ("Bar");
    
    // Append media created to entry.
    // Note: Ownership of media is now moved to entry
    entry.appendMedia (media);
    
    // Ask engine to sent entry (this function will also serialize entry)
    WebUpload::System::EngineResponse res =
        WebUpload::System::sendEntryToUploadEngine (&entry);
    
    // Handle response received from the engine  
    switch (res) {
    
        case WebUpload::System::ENGINE_RESPONSE_OK:
            return true;
        
        default:
            qWarning() << "Failure:" << res;
            return false;
    }    
}
