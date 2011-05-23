
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



 
#include "metaapplication.h"
#include "backendinterface.h"
#include "magic.h"
#include "mp4backend.h"

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>

using namespace Metaman;

MetaApplication::MetaApplication():
    m_backend(0),
    m_operationMode(OPERATION_MODE_ALL)
{
}



MetaApplication::MetaApplication(OperationMode operationMode):
    m_backend(0),
    m_operationMode(operationMode)
{
    if (m_operationMode == OPERATION_MODE_NONE) {
        m_operationMode = OPERATION_MODE_ALL;
    }
}



MetaApplication::~MetaApplication()
{
    delete m_backend;
    m_backend = 0;
}



OperationResult MetaApplication::eraseMetaData()
{
    qDebug() << "MetaApplication::eraseMetaData()";

    OperationResult operationResult = OPERATION_GENERAL_ERROR;
    
    if (m_backend != 0) {
        operationResult = m_backend->removeAllMetaData();
    }

    return operationResult;
}



OperationResult MetaApplication::eraseAuthorAndGps()
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {
        OperationResult removeAuthorResult = m_backend->removeAuthor();
        OperationResult removeGpsStringResult = m_backend->removeGpsString();
        OperationResult removeGeoTagResult = m_backend->removeGeoTag();

        if (removeAuthorResult == OPERATION_OK &&
            removeGpsStringResult == OPERATION_OK &&
            removeGeoTagResult == OPERATION_OK) {

            operationResult = OPERATION_OK;
        }
    }

    return operationResult;
}



OperationResult MetaApplication::setDescription(const QByteArray& description)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {
        if (description.isEmpty()) {
            operationResult = m_backend->removeDescription();
        }
        else {
            operationResult = m_backend->setDescription(description);
        }
    }

    return operationResult;
}



OperationResult MetaApplication::setTitle(const QByteArray& title)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {
        if (title.isEmpty()) {
            operationResult = m_backend->removeTitle();
        }
        else {
            operationResult = m_backend->setTitle(title);
        }
    }

    return operationResult;
}



bool MetaApplication::ableToProcess()
{
    bool canProcess = false;
    QIODevice::OpenMode openMode;

    if (m_inputFilePath == m_outputFilePath) {
        openMode = QIODevice::ReadWrite;
    }
    else {
        openMode = QIODevice::ReadOnly;
    }

    m_inputFile.open(openMode);

    if (m_inputFile.isOpen()) {
        if (hasFtypAtom(m_inputFile)) {

            // Ftyp atom means that the file is based on QuickTime/MPEG-4 Part 12
            // format. Let the MP4 backend make some further test to see if the
            // file can be handled.
            delete m_backend; // Just to make sure this won't leak memory
            m_backend = new Mp4Backend(m_operationMode);

            if (m_backend != 0) {
                canProcess = m_backend->ableToProcess(m_inputFile);
            }
        }
        else {
            // Cannot handle this type of file; no suitable handler found
            canProcess = false;
        }

        if (!canProcess) {
             m_inputFile.close();
        }
    }

    return canProcess;
}



OperationResult MetaApplication::setOutputFile(const QString& outputFilePath)
{
    m_outputFilePath = outputFilePath;
    return OPERATION_OK;
}



OperationResult MetaApplication::setInputFile(const QString& inputFilePath)
{
    m_inputFile.setFileName(inputFilePath);    
    m_inputFilePath = inputFilePath;
    return OPERATION_OK;
}



OperationResult MetaApplication::writeFile()
{
    qDebug() << "writeFile()";
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {

        if (m_inputFilePath == m_outputFilePath) {
            // If writing to the input file, it should be already open.
            operationResult = m_backend->writeFile(m_inputFile);
        }
        else {
            QFile outputFile(m_outputFilePath);
            outputFile.open(QIODevice::WriteOnly);
            operationResult = m_backend->writeFile(outputFile);
            outputFile.close();
        }

        m_inputFile.close();
    }

    return operationResult;
}



OperationResult MetaApplication::readFile()
{
    // The input file should be open when this function gets called. If it's
    // not, something must have wrong earlier.

    OperationResult operationResult = OPERATION_GENERAL_ERROR;
    
    if (m_backend != 0 && m_inputFile.isOpen()) {
        operationResult = m_backend->readFile(m_inputFile);
    }

    return operationResult;
}



bool MetaApplication::hasFtypAtom(QFile& file)
{
    bool hasFtyp = false;
    qint32 rawData = 0;
    
    if (file.isOpen()) {
        QDataStream dataStream(&file);
        dataStream.skipRawData(ATOM_NAME_FTYP_OFFSET);
        dataStream >> rawData;
    }

    rawData = qToBigEndian(rawData);
    QByteArray readAtomName((char*)(&rawData), ATOM_NAME_PREAMBLE_LENGTH);

    if (readAtomName == ATOM_NAME_FTYP) {
        hasFtyp = true;
    }

    return hasFtyp;
}



OperationResult Metaman::MetaApplication::addKeywords(const QList<QByteArray>& keywords)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {
        operationResult = m_backend->setKeywords(keywords);
    }

    return operationResult;
}



OperationResult MetaApplication::setGeotag(const QByteArray& geotag)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;

    if (m_backend != 0) {
        if (geotag.isEmpty()) {
            operationResult = m_backend->removeGeoTag();
        }
        else {
            operationResult = m_backend->setGeotag(geotag);
        }
    }

    return operationResult;
}
