 
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
 * \example testpost.h
 * Example class definition inheriting PostBase and faking post process
 */
 
#ifndef _TEST_POSTBASE_H_
#define _TEST_POSTBASE_H_

#include <WebUpload/PostBase>
#include <WebUpload/AuthBase>
#include <QObject>
#include <QTimer>

// Test post class is inherited from PostBase. You can inherit your
// implementation directly from PostInterface or from PostSimpleHttp if you
// are using single HTTP request for each media upload.

class TestPost : public WebUpload::PostBase {

    Q_OBJECT
    public:
        TestPost(QObject *parent = 0);
        virtual ~TestPost();
        
    protected:
        /*!
          \brief Returns auth class for PostInterface
         */
        virtual WebUpload::AuthBase * getAuthPtr ();
        
        /*!
          \brief Implementation for stopping the upload
         */
        virtual void stopMediaUpload ();

    protected Q_SLOTS:
        /*!
          \brief Slot called by PostInterface for each media in entry
         */
        virtual void uploadMedia (WebUpload::Media * media);

    private Q_SLOTS:
        /*!
          \brief Handle timer events
         */
        void timerTimeout();
    
    private:
    
        //! We use default dummy implementation of auth
        WebUpload::AuthBase m_auth;
    
        QTimer m_timer; //!< Used to fake posting
        int m_step; //!< Used to move between fake states
};

#endif /* #ifndef _TEST_POSTBASE_H_ */