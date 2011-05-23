 
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




#ifndef _RECOVERY_H_
#define _RECOVERY_H_

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "WebUpload/entry.h"

/*!
   \class  Recovery
   \brief  This class provides the recovery functionality for the
           webupload-recovery application
   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class Recovery : public QObject {

    Q_OBJECT

public:

    /*!
      \brief Constructor
      \param clean : If <code>true</code> all left over entry xmls and the
                media copies are deleted. If <code>false</code>, this class
                will send each entry xml to webupload-engine
      \param parent : Pointer to the QObject parent of this class (in this
                case, it will be a pointer to the UploadEngine instance)
     */
    Recovery (bool clean=false, QObject *parent = 0);

    //! \brief Destructor 
    virtual ~Recovery ();

Q_SIGNALS:

    /*!
      \brief Signal emitted when recovery functionality is completed
     */
    void done ();


public Q_SLOTS:

    /*!
      \brief Slot which does the actual recovery functionality
     */
    void recover ();
   
private:

    void cleanEntries ();

    bool m_clean; 
    QList <QSharedPointer <WebUpload::Entry> > m_entries;
};

#endif // _RECOVERY_H_
