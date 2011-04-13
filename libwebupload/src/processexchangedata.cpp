
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

#include "WebUpload/processexchangedata.h"
#include "processexchangedataprivate.h"
#include <QDataStream>
#include <QDebug>

using namespace WebUpload;

ProcessExchangeData::ProcessExchangeData (QObject * parent) : 
    QObject (parent),
    d_ptr(new ProcessExchangeDataPrivate(this))
{
}

ProcessExchangeData::~ProcessExchangeData()
{
    delete d_ptr;
    d_ptr = 0;
}

void ProcessExchangeData::clear () {
    if (d_ptr->m_alreadyReadData != 0) {
        delete [] d_ptr->m_alreadyReadData;
        d_ptr->m_alreadyReadData = 0;
        d_ptr->m_alreadyReadAmount = d_ptr->m_sizeReqd = 0;
    }
}

QByteArray ProcessExchangeData::startUpload (const QString & entryXmlPath,
    const Error & error) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_START_UPLOAD;
    ds << entryXmlPath;

    QByteArray errArray = error.serialize ();
    ds << (quint32)errArray.size ();
    ds.writeRawData (errArray.data(), errArray.size());

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::updateAll (const QString & accountStringId) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_UPDATE_ALL;
    ds << accountStringId;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::update (const QString & accountStringId,
    const QString & optionId) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_UPDATE;
    ds << accountStringId;
    ds << optionId;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::addValue (const QString & accountStringId,
    const QString & optionId, const QString & valueName) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_ADD_VALUE;
    ds << accountStringId;
    ds << optionId;
    ds << valueName;

    return ProcessExchangeDataPrivate::wrapSize (data);
}
    

QByteArray ProcessExchangeData::stop () {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_STOP;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::sendingMedia (quint32 index) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_SENDING_MEDIA;
    ds << index;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::progress (float pAmt) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_PROGRESS;
    ds << pAmt;

    return ProcessExchangeDataPrivate::wrapSize (data);
}

    
QByteArray ProcessExchangeData::done () {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_DONE;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::stopped () {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_STOPPED;

    return ProcessExchangeDataPrivate::wrapSize (data);
}
    
QByteArray ProcessExchangeData::uploadFailed (const Error & error) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_UPLOAD_FAILED;
    
    QByteArray errArray = error.serialize ();
    ds << (quint32)errArray.size ();
    ds.writeRawData (errArray.data(), errArray.size());

    return ProcessExchangeDataPrivate::wrapSize (data);
}

#ifdef WARNINGS_ENABLED            
QByteArray ProcessExchangeData::uploadWarning (const Error & warning) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_UPLOAD_WARNING;
    
    QByteArray warningArray = warning.serialize ();
    ds << (quint32)warningArray.size ();
    ds.writeRawData (warningArray.data(), warningArray.size());

    return ProcessExchangeDataPrivate::wrapSize (data);
}
#endif
    
QByteArray ProcessExchangeData::updateFailed (
    const Error::Code & errorId, const QStringList & failedIds) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_UPDATE_FAILED;
    ds << (qint32) errorId;
    ds << (quint32)failedIds.count ();
    for (int i = 0; i < failedIds.count (); i++) {
        ds << failedIds[i];
    }

    return ProcessExchangeDataPrivate::wrapSize (data);
}

QByteArray ProcessExchangeData::optionValueChanged (const QString & optionName,
    const QVariant & optionValue, int mediaIndex) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << 
        (qint32)ProcessExchangeDataPrivate::CODE_REQUEST_OPTION_VALUE_CHANGED;
    ds << optionName;
    ds << optionValue;
    ds << mediaIndex;

    return ProcessExchangeDataPrivate::wrapSize (data);
}


QByteArray ProcessExchangeData::customRequest (const QByteArray & inputStream) {

    QByteArray data;
    QDataStream ds (&data, QIODevice::WriteOnly);

    ds << (qint32) ProcessExchangeDataPrivate::CODE_REQUEST_CUSTOM;
    ds << inputStream.size ();
    ds.writeRawData (inputStream.data(), inputStream.size());

    return ProcessExchangeDataPrivate::wrapSize (data);
}

void ProcessExchangeData::processByteArray(const QByteArray &recvdInfo)
{
    d_ptr->processByteArray(recvdInfo);
}


// -- private class functions -----------------------------------------------

ProcessExchangeDataPrivate::ProcessExchangeDataPrivate(ProcessExchangeData *publicObject)
 :  q_ptr(publicObject),
    m_alreadyReadData(0),
    m_alreadyReadAmount(0),
    m_sizeReqd(0)
{

}

ProcessExchangeDataPrivate::~ProcessExchangeDataPrivate()
{
    if (m_alreadyReadData != 0) {
        delete [] m_alreadyReadData;
        m_alreadyReadData = 0;
    }
}
    
void ProcessExchangeDataPrivate::processByteArray (const QByteArray & recvdInfo) {
    QList<QByteArray> baList;
    QDataStream readStream (recvdInfo);

    int leftToRead = recvdInfo.size();
    while (readStream.atEnd () == false) {
        if (m_sizeReqd == 0) {
            readStream >> m_sizeReqd;
            leftToRead -= sizeof (quint32);
            if (m_sizeReqd <= 0) {
                qDebug() << "Recieved invalid sizeReqd " << m_sizeReqd;
                qDebug() << "Throw away rest of the stream and hope for the best";
                break;
            }

            m_alreadyReadData = new char [m_sizeReqd];
        } 

        int toRead = m_sizeReqd - m_alreadyReadAmount;
        
        if (toRead > leftToRead) {
            readStream.readRawData (m_alreadyReadData + m_alreadyReadAmount,
                leftToRead);
            m_alreadyReadAmount += leftToRead;
            leftToRead = 0;
            return;
        } else {
            readStream.readRawData (m_alreadyReadData + m_alreadyReadAmount,
                toRead);
            m_alreadyReadAmount = m_sizeReqd;
            leftToRead -= toRead;
        }

        QByteArray request (m_alreadyReadData, m_sizeReqd);
        baList.append (request);

        m_sizeReqd = m_alreadyReadAmount = 0;
        delete [] m_alreadyReadData;
        m_alreadyReadData = 0;
    }

    for (int i = 0; i < baList.size(); i++) {
        QDataStream requestStream (&(baList[i]), QIODevice::ReadOnly);

        qint32 requestId;
        requestStream >> requestId;

        switch ((RequestEnums)requestId) {
            case CODE_REQUEST_START_UPLOAD:
                parseStartUploadRequest (requestStream);
                break;

            case CODE_REQUEST_UPDATE_ALL:
            {
                QString accountId;
                requestStream >> accountId;
                qDebug() << "updateAllSignal";
                Q_EMIT (q_ptr->updateAllSignal (accountId));
                break;
            }

            case CODE_REQUEST_UPDATE:
            {
                QString accountId, optionId;
                requestStream >> accountId;
                requestStream >> optionId;
                qDebug() << "updateSignal";
                Q_EMIT (q_ptr->updateSignal (accountId, optionId));
                break;
            }

            case CODE_REQUEST_ADD_VALUE:
            {
                QString accountId, optionId, valueName;
                requestStream >> accountId;
                requestStream >> optionId;
                requestStream >> valueName;
                qDebug() << "addValueSignal";
                Q_EMIT (q_ptr->addValueSignal (accountId, optionId, valueName));
                break;
            }

            case CODE_REQUEST_STOP:
                qDebug() << "stopSignal";
                Q_EMIT (q_ptr->stopSignal ());
                break;

            case CODE_REQUEST_SENDING_MEDIA:
            {
                quint32 index;
                requestStream >> index;
                qDebug() << "sendingMediaSignal";
                Q_EMIT (q_ptr->sendingMediaSignal (index));
                break;
            }

            case CODE_REQUEST_PROGRESS:
            {
                float pAmt;
                requestStream >> pAmt;
                qDebug() << "progressSignal";
                Q_EMIT (q_ptr->progressSignal (pAmt));
                break;
            }

            case CODE_REQUEST_DONE:
                qDebug() << "doneSignal";
                Q_EMIT (q_ptr->doneSignal ());
                break;

            case CODE_REQUEST_STOPPED:
                qDebug() << "stoppedSignal";
                Q_EMIT (q_ptr->stoppedSignal ());
                break;

            case CODE_REQUEST_UPLOAD_FAILED:
                parseUploadFailedRequest (requestStream);
                break;

            #ifdef WARNINGS_ENABLED
            case CODE_REQUEST_UPLOAD_WARNING:
                parseUploadWarningRequest( requestStream);
                break;
            #endif

            case CODE_REQUEST_UPDATE_FAILED:
                parseUpdateFailedRequest (requestStream);
                break;

            case CODE_REQUEST_OPTION_VALUE_CHANGED:
            {
                QString optionName;
                QVariant optionValue;
                int mediaIndex;
                
                requestStream >> optionName;
                requestStream >> optionValue;
                requestStream >> mediaIndex;

                qDebug() << "optionValueChangedSignal";
                Q_EMIT (q_ptr->optionValueChangedSignal(optionName, optionValue,mediaIndex));
            }

            default:
            {
                quint32 sizeBuffer;
                requestStream >> sizeBuffer;

                char tmpBuffer [sizeBuffer];
                requestStream.readRawData (tmpBuffer, sizeBuffer);

                QByteArray customArray (tmpBuffer, sizeBuffer);
                qDebug() << "customRequestSignal";
                Q_EMIT (q_ptr->customRequestSignal (customArray));
                break;
            }
        }
    }
}

QByteArray ProcessExchangeDataPrivate::wrapSize (const QByteArray &inS) {
    QByteArray retVal;
    QDataStream retValStream (&retVal, QIODevice::WriteOnly);

    retValStream << (quint32) inS.size();
    retVal.append (inS);

    return retVal;
}

void ProcessExchangeDataPrivate::parseStartUploadRequest (QDataStream & ds) {
    QString entryXmlPath;
    ds >> entryXmlPath;

    quint32 errArraySize;
    ds >> errArraySize;

    char errArray [errArraySize];
    ds.readRawData (errArray, errArraySize);
    QByteArray errByteArray (errArray, errArraySize);

    Error error (errByteArray);

    qDebug() << "startUploadSignal";
    Q_EMIT (q_ptr->startUploadSignal (entryXmlPath, errByteArray));
}


void ProcessExchangeDataPrivate::parseUploadFailedRequest (QDataStream & ds) {
    quint32 errArraySize;
    ds >> errArraySize;

    char errArray [errArraySize];
    ds.readRawData (errArray, errArraySize);
    QByteArray errByteArray (errArray, errArraySize);

    Error error (errByteArray);

    qDebug() << "uploadFailedSignal";
    Q_EMIT (q_ptr->uploadFailedSignal (error));
}

#ifdef WARNINGS_ENABLED
void ProcessExchangeDataPrivate::parseUploadWarningRequest (QDataStream & ds) {
    quint32 errArraySize;
    ds >> errArraySize;

    char errArray [errArraySize];
    ds.readRawData (errArray, errArraySize);
    QByteArray errByteArray (errArray, errArraySize);

    Error error (errByteArray);

    qDebug() << "uploadFailedSignal";
    Q_EMIT (q_ptr->uploadWarningSignal (error));
}
#endif


void ProcessExchangeDataPrivate::parseUpdateFailedRequest (QDataStream & ds) {
    qint32 errCode;
    quint32 failedCount; 
    QStringList failedIds;

    ds >> errCode;
    ds >> failedCount;
    for (quint32 i = 0; i < failedCount; i++) {
        QString errString;
        ds >> errString;
        failedIds.append (errString);
    }

    qDebug() << "updateFailedSignal";
    Q_EMIT (q_ptr->updateFailedSignal ((Error::Code)errCode, failedIds));
}
