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

#ifndef _DUMMY_POST_H_
#define _DUMMY_POST_H_

#include <WebUpload/PostSimpleHttp>


/*!
  \class DummyPost
  \brief Dummy post class to go ahead with testing the PostBase, PostSimpleHttp
         classes
  \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class DummyPost : public WebUpload::PostSimpleHttp {
    Q_OBJECT
    
public:
    /*!
      \brief Constructor
      \param parent QObject parent
     */
    DummyPost (QObject *parent = 0);
    
    ~DummyPost ();  

protected:

    //! \brief Implementation for WebUpload::PostBase::getAuthPtr
    WebUpload::AuthBase * getAuthPtr ();

    /*!
      \brief Implementation for WebUpload::PostSimpleHttp::generateRequest
     */
    virtual QNetworkReply * generateRequest (WebUpload::Media * media);
    
    //! \brief Implementation for WebUpload::PostSimpleHttp::handleResponse
    virtual void handleResponse (QNetworkReply * response); 
    
};
#endif
