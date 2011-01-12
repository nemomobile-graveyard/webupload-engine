
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
 * \example testupdate.h
 * Example class definition inheriting PostInterface and faking update process
 */ 

#ifndef _TEST_UPDATE_H_
#define _TEST_UPDATE_H_

#include <WebUpload/UpdateInterface>
#include <QTimer>

// Test update class is inherited from UpdateInterface. When developing plugins
// you might want to inherit your implementation from UpdateBase or
// UpdateSimpleHttp. These classes provide implementation for some basic
// functionality.

/*!
  \class TestUpdate
  \brief Test update class. Fake update process with timer events.
  \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>  
 */
class TestUpdate : public WebUpload::UpdateInterface {
    Q_OBJECT

public:

    TestUpdate (QObject *parent = 0);
    virtual ~TestUpdate();
    
    virtual void updateAll (WebUpload::Account * account);
        
    virtual void update (WebUpload::Account * account,
        WebUpload::ServiceOption * option);

    virtual void addValue (WebUpload::Account * account,
        WebUpload::ServiceOption * option, const QString &valueName);
        
    virtual void cancel();        
    
private:
    QTimer m_timer; //!< Used to fake process

};

#endif