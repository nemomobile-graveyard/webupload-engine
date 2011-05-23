 
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
 * \example exampleio.cpp
 * This is an example of how to use the HttpMultiContentIO to construct multi
 * content HTTP request body.
 */
 
 
class ServiceUpload : public QObject 
{
    Q_OBJECT
public:
    ServiceUpload();
    virtual ~ServiceUpload();
    
private:
    HttpMultiContentIO *contentIO;
};



ServiceUpload::ServiceUpload() : QObject(), contentIO (0) {
}

ServiceUpload::~ServiceUpload() {

}

void ServiceUpload::addStringWithBoundary() {
    //Example request 
    //--SOME_BOUNDARY_TEXT
    //Content-Disposition: form-data; name="caption"
    //code using multiContentIO
    QString dataString = "Content-Disposition: form-data; name=\"caption\"";
    contentIO->addString(dataString,true);
    
}

void ServiceUpload::addStringWithoutBoundary() {
    //Example request 
    //Content-Type: image/jpg 
    //code using multiContentIO
    QString dataString = "Content-Type: image/jpg";
    contentIO->addString(dataString,false);
}

void ServiceUpload::createAtomXmlRequest() {
    //Example request for application/atom+xml

    //Content-Type: application/atom+xml
    //<entry xmlns='http://www.w3.org/2005/Atom'>
    //  <title>sunflower.jpg</title>
    //  <summary>Beautiful flower, when no Sun.</summary>
    //</entry>
    //--SOME_BOUNDARY_TEXT
    
    // This part of the code generates data for content type application/atom + xml
    contentIO->setDefaultTemplate ("Content-Type: %1");
    QString entryAtom = "<entry xmlns='http://www.w3.org/2005/Atom'> \r\n
                        <title>sunflower.jpg</title> \r\n
                        <summary>Beautiful flower, when no Sun.</summary> \r\n
                        </entry>"

    argList << "application/atom+xml";
    if (!contentIO->addString (argList, entryAtom)) {
        qDebug() << "Failed to add metadata to request";
    }

}

void ServiceUpload::createFormDataRequest() {
    //Example for form-data request
    //--SOME_BOUNDARY_TEXT 
    //Content-Disposition: form-data; name="caption" 
    //Beautiful flower, when no Sun.
    //--SOME_BOUNDARY_TEXT 
    //Content-Disposition: form-data; name="title" 
    //sunflower.jpg
    
    // This part of code generates form data request
    contentIO->setDefaultTemplate ("Content-Disposition:form-data;name=\"%1\"");
    //add the caption
    contentIO->addString (QStringList() << "caption", "Beautiful flower, when no sun");
    //add the title
    contentIO->addString (QStringList() << "title", "sunflower.jpg");
}

void ServiceUpload::createIO() {
    //generates a boundry string which is used in the multipart IO request
     contentIO = new HttpMultiContentIO();
 

    if (!contentIO->open (QIODevice::ReadWrite)) {
        qDebug() << "Failed to open content buffer";
        return;
    }
    // Get the boundary String which is generated from HttpMultiContentIO
    QString boundaryString = contentIO->getBoundaryString();
}

QString ServiceUpload::getDefaultBoundaryString() const {

    return contentIO->getBoundaryString();
}

void ServiceUpload::setBoundaryString() {

    QString boundaryString = "SOME_BOUNDARY_TEXT";
    if(contentIO->setBoundaryString(boundaryString)!=true) {
          qWarning() << "Could not set boundary string";
    }
}

void ServiceUpload::uploadPhoto() {
    // Example request for media part
    //--SOME_BOUNDARY_TEXT
    //Content-Disposition: form-data; name="file"; filename="sunflower.jpg"
    //Content-Type: image/jpeg

    //...binary image data...
    //--SOME_BOUNDARY_TEXT--

    // Add file
    QStringList args;
    args << "Content-Disposition:form-data" << "photo"
        << "sunflower.jpg" << "image/jpeg";
    contentIO->addFile(args, "sunflower.jpg",
        "%1;name=\"%2\";filename=\"%3\"\r\nContent-Type:%4");
}

void ServiceUpload::uploadContentToService() {
    // No more data to add
    contentIO->allDataAdded();
    contentIO->close();
    
    if (!contentIO->open (QIODevice::ReadOnly)) {
        qWarning() << "Failed to open content buffer for reading";
        return 0;
    }
    
    //post data to the service
    networkManager->post(request, contentIO);
}
