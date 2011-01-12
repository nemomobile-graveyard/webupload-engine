
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




#include <QDebug>
#include "WebUpload/System"
#include "uploadengine.h" 
#include <cstdlib>
#include <iostream>
#include "recovery.h"

bool entryAgeLessThan (QSharedPointer<WebUpload::Entry> & e1,
    QSharedPointer<WebUpload::Entry> & e2) {
    
    qDebug() << "Comparing entry values" << e1->created() << "and"
        << e2->created();
    
     return e1->created() < e2->created();
}

Recovery::Recovery (bool clean, QObject *parent) : QObject (parent),
    m_clean (clean) {

}

Recovery::~Recovery () {
    m_entries.clear(); 
}

void Recovery::recover () {
    
    WebUpload::System system;
    m_entries = system.outboxEntries();
    
    if (m_entries.count() == 0) {
        qDebug() << "No undone entries found.";
        Q_EMIT (done ());
        return;
    }
    
    qDebug() << m_entries.count() << "undone entries loaded.";
    
    if (m_clean) {
        cleanEntries ();
        Q_EMIT (done ());
        return;
    }
    
    //Order entries
    qSort (m_entries.begin(), m_entries.end(), entryAgeLessThan);
    
    //TODO: check state of transfers???
    QStringList paths;
    for (int i = 0; i < m_entries.count(); ++i) {
        paths << m_entries.at(i)->serializedTo();
    }

    UploadEngine interface ("com.meego.sharing.webuploadengine");
    for (int i = 0; i < paths.count(); ++i) {
        bool ret = interface.newUpload (paths.at(i));
        if (ret == false) {
            qWarning() << "Failed to send entry" << paths.at(i) << 
                "to webupload engine. Cancelling it ...";
            m_entries.at(i)->cancel ();
        }
    }    

    Q_EMIT (done ());
    return;
}


void Recovery::cleanEntries () {
    std::cout << "Cleaning mode enabled";
    for (int i = 0; i < m_entries.count(); ++i) {
        std::cout << i + 1 << ": Cancel entry";
        m_entries.at(i)->cancel();
    }
}