
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




/*!
 * \example testupdate.cpp
 * Example class definition implementation PostInterface and faking update
 * process
 */ 

#include "testupdate.h"

TestUpdate::TestUpdate (QObject *parent) : WebUpload::UpdateInterface (parent) {
    m_timer.setSingleShot (true);
    connect(&m_timer, SIGNAL(timeout()), this, SIGNAL(done()));
}

TestUpdate::~TestUpdate() {
    m_timer.disconnect (this);
}

void TestUpdate::updateAll (WebUpload::Account * account) {
    
    // Just faking so we ignore account. In normal use cases you should check
    // updatable options of account and store updated values there. You
    // can also inherit from UpdateBase if you want to only implement update
    // function and let base class to check for which options it should be
    // called.
    Q_UNUSED (account);
    
    // Just faking update by using timer
    m_timer.start (3000);

}
        
void TestUpdate::update (WebUpload::Account * account,
    WebUpload::ServiceOption * option) {
    
    // Just faking update of option so ignoring input. This function is called
    // if single options are updated.
    Q_UNUSED (account);
    Q_UNUSED (option);
    
    // Just faking update by using timer
    m_timer.start (3000);
}

void TestUpdate::addValue (WebUpload::Account * account,
    WebUpload::ServiceOption * option, const QString &valueName) {
    
    // Just faking add value action so not adding valueName to option. This
    // function is called for example when user wants to add new photo album
    // to service.
    Q_UNUSED (account);
    Q_UNUSED (option);
    Q_UNUSED (valueName);
    
    // Just faking add by using timer
    m_timer.start (3000);
    
}

void TestUpdate::cancel() {
    // Stop timer can emit canceled signal to notify that action was canceled
    m_timer.stop();
    Q_EMIT (canceled());
}