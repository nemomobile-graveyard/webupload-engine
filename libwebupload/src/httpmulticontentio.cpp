 
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "WebUpload/HttpMultiContentIO"
#include "httpmulticontentioprivate.h"
#include <QDebug>
#include <QTime>
#include <QBuffer>
#include <QFile>

using namespace WebUpload;

#define IODEVICE_ERROR(a)  qWarning() << a;   \
                           delete buff;       \
                           return false;

HttpMultiContentIO::HttpMultiContentIO(QObject *parent) : QIODevice(parent),
    d_ptr(new HttpMultiContentIOPrivate) {
}

HttpMultiContentIO::~HttpMultiContentIO() {
    delete d_ptr;
}

bool HttpMultiContentIO::setDefaultTemplate(const QString &tmplt) {
    d_ptr->defaultTemplate = tmplt;
    return true;
}

const QString & HttpMultiContentIO::getDefaultTemplate() const {
    qCritical() << "Using deprecated function" << __FUNCTION__;
    return d_ptr->defaultTemplate;
}

QString HttpMultiContentIO::defaultTemplate() const {
    return d_ptr->defaultTemplate;
}

void HttpMultiContentIO::clearDefaultTemplate() {
    d_ptr->defaultTemplate = "";
}

bool HttpMultiContentIO::setBoundaryString(const QString &bdry) {
    d_ptr->boundaryString = bdry;
    return true;
}

const QString & HttpMultiContentIO::getBoundaryString() const {
    qCritical() << "Using deprecated function" << __FUNCTION__;
    return d_ptr->boundaryString;
}

QString HttpMultiContentIO::boundaryString() const {
    return d_ptr->boundaryString;
}

bool HttpMultiContentIO::addString(const QString &string, 
    bool prefixBoundary) {

    if (!d_ptr->isDeviceOpen) {
        qWarning() << "HttpMultiContentIO::" << __FUNCTION__ << 
            "called when device is not open yet";
        return false;
    }

    if(d_ptr->bytesSent > 0) {
        qWarning() << "Already sent " << d_ptr->bytesSent << " bytes";
        return false;
    }

    QBuffer *buff = new QBuffer(this);
    if (!buff->open (openMode())) {
        IODEVICE_ERROR("Could not open buffer in required mode");
    }

    if((prefixBoundary) && (!d_ptr->boundaryString.isEmpty())) {
        buff->write("--");
        if(buff->write(d_ptr->boundaryString.toUtf8()) == -1) {
            IODEVICE_ERROR("Write to buffer failed")
        }

        buff->write("\r\n");
    } else if(prefixBoundary) {
    // i.e d_ptr->boundaryString.isEmpty() == true 
        IODEVICE_ERROR ("No boundary string defined")
    }

    if(buff->write(string.toUtf8()) == -1) {
        IODEVICE_ERROR("Write to buffer failed")
    }

    if(buff->write("\r\n") == -1) {
        IODEVICE_ERROR("Write to buffer failed")
    }

    d_ptr->totalSizeBytes += buff->size();
    buff->close();
    d_ptr->dataList.append(buff);
    return true;
}

bool HttpMultiContentIO::addString(const QStringList &args, 
    const QString &string, const QString &tmplt) {

    QString finalString = d_ptr->defaultTemplate;
    
    if(!tmplt.isEmpty()) {
        finalString = tmplt;
    } 

    if(!string.isEmpty()) {
        finalString += "\r\n\r\n" + string;
    }

    for(int i = 0; i < args.count(); i++) {
        finalString = finalString.arg(args[i]);
    }

    return addString(finalString, true);
}

bool HttpMultiContentIO::addFile(const QStringList &args, 
    const QString &filePath, const QString &tmplt) {

    if (!d_ptr->isDeviceOpen) {
        qWarning() << "HttpMultiContentIO::" << __FUNCTION__ << 
            "called when device is not open yet";
        return false;
    }

    bool retVal = true;
    QString finalString = d_ptr->defaultTemplate;
    
    if(!tmplt.isEmpty()) {
        finalString = tmplt;
    } 

    for(int i = 0; i < args.count(); i++) {
        finalString = finalString.arg(args[i]);
    }

    finalString += "\r\n";
    retVal = addString(finalString, true);

    if(retVal) {
        QFile *file = new QFile(filePath, this);
        if (!file->open (QIODevice::ReadOnly)) {
            qWarning() << "Could not open file" << filePath << "in read mode";
            delete file;
            return false;
        }
        d_ptr->totalSizeBytes += file->size();
        d_ptr->dataList.append(file);

        QString  endString = "\r\n--" + d_ptr->boundaryString + "--\r\n";
        retVal = addString(endString, false);
    }

    return retVal;
}

void HttpMultiContentIO::allDataAdded() {
    // Does not seem to be anything required here currently.
}

bool HttpMultiContentIO::open(OpenMode openMode) {
    if (d_ptr->isDeviceOpen) {
        qWarning() << "HttpMultiContentIO::open - device already open";
        return false;
    }

    if ((!d_ptr->dataList.isEmpty()) && (openMode != QIODevice::ReadOnly)) {
        qDebug() << "Opening existing IODevices in read only mode, though"
            " open request is with" << openMode << "mode";
    }

    QVectorIterator<QIODevice *> iterDevice(d_ptr->dataList);
    while(iterDevice.hasNext()) {
        QIODevice *device = iterDevice.next();
        if(!device->open(QIODevice::ReadOnly)) {
            qWarning() << "Problem opening some IODevice which is part of"
                " this multi-content IODevice in the required mode. Closing"
                " the device";
            close();
            return false;
        }
    }

    d_ptr->currentDeviceIndex = 0;

    d_ptr->isDeviceOpen = QIODevice::open (openMode);

    if (!d_ptr->isDeviceOpen)
        close ();

    return d_ptr->isDeviceOpen;
}

void HttpMultiContentIO::close() {

    QVectorIterator<QIODevice *> iterDevice(d_ptr->dataList);
    while(iterDevice.hasNext() == true) {
        QIODevice *device = iterDevice.next();
        if(device->isOpen())
            device->close();
    }

    QIODevice::close();
    d_ptr->isDeviceOpen = false;
}

bool HttpMultiContentIO::isSequential() const {
    // As per bug 170498, comment #24, to handle large files, we need to make
    // HttpMultiContentIO a random access device. If we make it a sequential
    // device and set the QNetWrokRequest::DoNotBufferUploadDataAttribute, as
    // mentioned in that comment, then uploads fail with QNetworkReply having
    // error 205 - "the request needed to be sent again, but this failed for
    // example because the upload data could not be read a second time."
    return false;
}

bool HttpMultiContentIO::seek (qint64 pos) {
    if (d_ptr->isDeviceOpen == false) {
        qWarning() << "Trying to seek in a device not opened yet";
        return false;
    }

    if (pos > size()) {
        qWarning() << "Trying to seek beyond end of device";
        return false;
    }

    if (pos < 0) {
        qWarning() << "Negative seek";
        return false;
    }

    bool retVal = d_ptr->seek(pos);
    if (retVal) 
        retVal = QIODevice::seek (pos);

    return retVal;
}

qint64 HttpMultiContentIO::pos () const {
    if (d_ptr->isDeviceOpen == false) {
        qWarning() << "Trying to get position in a device not opened yet";
        return 0;
    }

    return d_ptr->pos ();
}

qint64 HttpMultiContentIO::size () const {
    if (d_ptr->isDeviceOpen == false) {
        qWarning() << "Trying to get position in a device not opened yet";
        return 0;
    }

    return d_ptr->totalSizeBytes;
}

bool HttpMultiContentIO::atEnd () const {
    if (d_ptr->isDeviceOpen == false) {
        qWarning() << "Check \"atEnd\" in a device not opened yet";
        return false;
    }

    if (d_ptr->currentDeviceIndex == d_ptr->dataList.size()) {
        return d_ptr->dataList[d_ptr->currentDeviceIndex]->atEnd ();
    } else {
        return false;
    }
}

void HttpMultiContentIO::clear() {
    close();
    d_ptr->clear();
}

qint64 HttpMultiContentIO::readData(char *data, qint64 maxlen) {
    if (!d_ptr->isDeviceOpen) {
        qWarning() << "Trying to read IODevice which has not been opened";
        return -1;
    }

    qint64 bytesRead = d_ptr->readData (data, maxlen);

    if (bytesRead == 0)
        bytesRead = -1;

    return bytesRead;
}

qint64 HttpMultiContentIO::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    qWarning() << "HttpMultiContentIO::writeData not implemented currently";
    return -1;
}

/****************************************************************************
 *              HttpMultiContentIOPrivate functions
 ****************************************************************************/

HttpMultiContentIOPrivate::HttpMultiContentIOPrivate() : isDeviceOpen (false),
    defaultTemplate(""), boundaryString("") {

    totalSizeBytes = 0;
    bytesSent      = 0;

    currentDeviceIndex = 0;

    generateBoundaryString();
}

HttpMultiContentIOPrivate::~HttpMultiContentIOPrivate() {
    clear ();
}

void HttpMultiContentIOPrivate::clear() {

    QVectorIterator<QIODevice *> iterDevice(dataList);
    while(iterDevice.hasNext()) {
        QIODevice * device = iterDevice.next();
        if(device->isOpen()) {
            device->close();
        }
        delete device;
    }
    dataList.clear();
    
    totalSizeBytes = 0;
    bytesSent = 0;
    currentDeviceIndex = 0;
}

bool HttpMultiContentIOPrivate::seek (qint64 pos) {
    bool retVal = true;
    qint64 sizeOfPastDevices = 0;

    for (currentDeviceIndex = 0; 
        (currentDeviceIndex < dataList.size ()) && (retVal == true);
        ++ currentDeviceIndex) {

        QIODevice * currDevice = dataList[currentDeviceIndex];

        if (currDevice->size() > (pos - sizeOfPastDevices)) {
            retVal = currDevice->seek (pos - sizeOfPastDevices);
            break;
        } 

        sizeOfPastDevices += currDevice->size ();
    }

    return retVal;
}

qint64 HttpMultiContentIOPrivate::pos () const {
    qint64 devicePos = 0;
    for (int i = 0; i < currentDeviceIndex; ++ i) {
        QIODevice * currDevice = dataList[i];
        devicePos += currDevice->size ();
    }

    devicePos += dataList[currentDeviceIndex]->pos ();
    return devicePos;
}

qint64 HttpMultiContentIOPrivate::readData (char *data, qint64 maxlen) {
    qint64 bytesRead = 0;
    int i;
    for (i = currentDeviceIndex; i < dataList.count(); i++) {
        qint64 currentBytesRead = 0;
        QIODevice *currDevice = dataList[i];

        currentBytesRead = 
            currDevice->read(data + bytesRead, maxlen - bytesRead);
        if(currentBytesRead == -1) {
            qWarning() << "Could not read from QIDevice at index " << i;
            break;
        }
        bytesRead += currentBytesRead;
        bytesSent += currentBytesRead;
        if(bytesRead >= maxlen)
            break;
    }
       
    currentDeviceIndex = i;

    return bytesRead;
}


void HttpMultiContentIOPrivate::generateBoundaryString() {
	int length = 0;
	int timeMSec = QTime::currentTime().msec();
	srand(timeMSec);

	while(length <= 40) {
		int iNumber = rand() % 124;
        /* Youtube does not support = in boundary string */
		if(((iNumber > 47) && (iNumber <=  57)) ||
           ((iNumber > 64) && (iNumber <=  90)) || 
           ((iNumber > 97) && (iNumber <= 123)) ||
           (iNumber == '=')) {

			boundaryString += QString(QChar(iNumber));
			length++;
		} else {
            switch(iNumber) {
                case 33:
                case 35:
                case 36:
                case 42:
                case 43:
                case 45:
                case 61:
                    boundaryString += QString(QChar(iNumber));
                    length++;
            }
        }
	}
}