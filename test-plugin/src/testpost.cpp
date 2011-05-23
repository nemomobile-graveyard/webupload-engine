 
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
 * \example testpost.cpp
 * Example class implementation inheriting PostBase and faking post process
 */

#include "testpost.h"
#include <WebUpload/Entry>
#include <QDebug>

using namespace WebUpload;

TestPost::TestPost(QObject *parent) : WebUpload::PostBase (parent),
    m_auth(this), m_step(0) {
    
    // Connect timer event signal
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

TestPost::~TestPost() {
}

AuthBase * TestPost::getAuthPtr () {
    return &m_auth;
}

void TestPost::uploadMedia (WebUpload::Media *media) {
    Q_UNUSED (media); // We are not really uploading media, so ignoring
    
    m_step = 0; // make sure step counter is at zero
    m_timer.setSingleShot (false);
    m_timer.start (1000); // events each second
}


void TestPost::stopMediaUpload () {
    // As this is fake transfer we can just stop timer and emit stopped
    if (m_timer.isActive () == true) {
        m_timer.stop ();
        Q_EMIT (stopped ());
    }
}

void TestPost::timerTimeout() {

    // First then steps will update progress and then 11th step will emit done
    if (m_step < 10) {
        ++m_step;
        float fStep = m_step;
        float done = fStep * 0.1; // Calc fake progress
        qDebug() << "Emitting mediaProgress" << done;
        Q_EMIT (mediaProgress(done)); // Emit progress update
    } else {
        m_timer.stop(); // Last event
        qDebug() << "Emitting mediaDone";
        
        // With done signal tell where file was uploaded
        Q_EMIT (mediaDone ("dummy://file.jpg"));
    }
}
