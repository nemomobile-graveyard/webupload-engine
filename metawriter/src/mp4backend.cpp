
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



 
#include "mp4backend.h"
#include <QByteArray>
#include <QDataStream>
#include <QList>
#include "magic.h"
#include "atom.h"
#include <QDebug>
#include <QFile>
#include <QtEndian>
#include "xmphandler.h"
#include "mpeg4atomutility.h"

//be7acfcb-97a9-42e8-9c71-999491e3afac is UUID for XMP data:
//http://www.adobe.com/content/dam/Adobe/en/devnet/xmp/pdfs/XMPSpecificationPart3.pdf
#define XMP_UUID_HEX "be7acfcb97a942e89c71999491e3afac"

using namespace Metaman;

Mp4Backend::Mp4Backend(OperationMode operationMode)
 : m_rootAtom(0),
   m_sourceFile(0),
   m_operationMode(operationMode),
   m_xmpHandler(0),
   m_xmpReadFromUUID (false)
{
    qDebug() << "Using Mp4Backend, operation mode: " << m_operationMode;
}



Mp4Backend::~Mp4Backend()
{
    delete m_rootAtom;
    m_rootAtom = 0;

    delete m_xmpHandler;
    m_xmpHandler = 0;
}



OperationResult Mp4Backend::removeAllMetaData()
{
    qDebug() << "Mp4Backend::removeAllMetaData()";

    if (m_operationMode & OPERATION_MODE_XMP) {
        m_xmpHandler->removeAllXmpData();
    }

    OperationResult operationResult =
            Mpeg4AtomUtility::deleteAtom(ATOM_PATH_USERDATA, m_rootAtom);

    return operationResult;
}



OperationResult Mp4Backend::removeAuthor()
{
    qDebug() << "Mp4Backend::removeAuthor()";
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    if (m_operationMode & OPERATION_MODE_STANDARD) {
        operationResultStandard =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_AUTHOR, m_rootAtom);

        // This is for N900 format compatibility. For now, try to delete the atom
        // silently. Decided later how strictly this should be handled.
        Mpeg4AtomUtility::deleteAtom(ATOM_PATH_AUTHOR_ALTERNATIVE, m_rootAtom);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        operationResultItunes = 
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_ITUNES_AUTHOR,
                                             m_rootAtom);
    }
    if (m_operationMode & OPERATION_MODE_XMP) {
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->removeAuthor();
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }
    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::removeDescription()
{
    qDebug() << "Mp4Backend::removeDescription()";
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    if (m_operationMode & OPERATION_MODE_STANDARD) {
        operationResultStandard =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_DESCRIPTION,
                                             m_rootAtom);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        operationResultStandard =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_ITUNES_DESCRIPTION,
                                             m_rootAtom);
    }
    if (m_operationMode & OPERATION_MODE_XMP) {
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->removeDescription();
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }
    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::removeGpsString()
{
    qDebug() << "Mp4Backend::removeGpsString()";

    // As far as I know, there is no standard way to store GPS string on atom
    // level. That's why this only affects XMP data.
    
    OperationResult operationResultXmp = OPERATION_OK;
    
    if (m_operationMode & OPERATION_MODE_XMP) {
        operationResultXmp = m_xmpHandler->removeGpsString();
    }

    return operationResultXmp;
}



OperationResult Mp4Backend::removeGeoTag()
{
    qDebug() << "Mp4Backend::removeGeoTag()";
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;
    
    operationResultStandard =
            Mpeg4AtomUtility::deleteAtom(ATOM_PATH_GEOTAG, m_rootAtom);

    // This is for N900 format compatibility. For now, try to delete the atom
    // silently. Decided later how strictly this should be handled.
    Mpeg4AtomUtility::deleteAtom(ATOM_PATH_GEOTAG_ALTERNATIVE, m_rootAtom);

    if (m_operationMode & OPERATION_MODE_XMP) {
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->removeLocationInfo();
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }
    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }
    
    return operationResult;
}



OperationResult Mp4Backend::removeTitle()
{
    qDebug() << "Mp4Backend::removeTitle()";
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    if (m_operationMode & OPERATION_MODE_STANDARD) {
        operationResultStandard =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_TITLE, m_rootAtom);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        operationResultStandard =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_ITUNES_TITLE,
                                             m_rootAtom);
    }
    if (m_operationMode & OPERATION_MODE_XMP) {
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->removeTitle();
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }
    }
    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::removeXMPData()
{
    qDebug() << "Mp4Backend::removeXMPData()";
    OperationResult Result =
            Mpeg4AtomUtility::deleteAtom(ATOM_PATH_XMPDATA, m_rootAtom);
    return Result;
}



OperationResult Mp4Backend::removeItunesData()
{
    qDebug() << "Mp4Backend::removeItunesData()";
    OperationResult result =
            Mpeg4AtomUtility::deleteAtom(ATOM_PATH_ITUNESDATA, m_rootAtom);
    return result;
}



OperationResult Mp4Backend::setDescription(const QByteArray& description)
{
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    // The data atom that holds the actual title inside the titl atom needs
    // 8 extra bytes in addition to the actual title. The first 4 bytes
    // consist of the atom version and its class, which broadly defines the
    // type of the atom. After that, there are 4 null-bytes.
    const qint32 atomVersion = 0x00000000;
    const qint32 atomFlags = qToBigEndian(atomVersion | ATOM_CLASS_TEXT);
    
    if (m_operationMode & OPERATION_MODE_STANDARD) {
        operationResultStandard =
            setAtomData(description, ATOM_PATH_DESCRIPTION, atomFlags);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        operationResultStandard =
            setAtomData(description, ATOM_PATH_ITUNES_DESCRIPTION, atomFlags);
    }
    if (m_operationMode & OPERATION_MODE_XMP) {
        
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->setDescription(description);
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }

    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::setTitle(const QByteArray& title)
{
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    // The data atom that holds the actual title inside the titl atom needs
    // 8 extra bytes in addition to the actual title. The first 4 bytes
    // consist of the atom version and its class, which broadly defines the
    // type of the atom. After that, there are 4 null-bytes.
    const qint32 atomVersion = 0x00000000;
    const qint32 atomFlags = qToBigEndian(atomVersion | ATOM_CLASS_TEXT);

    if (m_operationMode & OPERATION_MODE_STANDARD) {
        qDebug() << "writing standard title tag";
        operationResultStandard =
            setAtomData(title, ATOM_PATH_TITLE, atomFlags);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        qDebug() << "writing itunes title tag: ";
        operationResultStandard =
            setAtomData(title, ATOM_PATH_ITUNES_TITLE, atomFlags);
    }

    if (m_operationMode & OPERATION_MODE_XMP) {
        
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->setTitle(title);
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }

    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::setAuthor(const QByteArray& author)
{
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultItunes   = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;



    if (m_operationMode & OPERATION_MODE_STANDARD) {
        qDebug() << "writing standard author tag";
        operationResultStandard =
                setAtomData(author, ATOM_PATH_AUTHOR);
    }

    if (m_operationMode & OPERATION_MODE_ITUNES) {
        qDebug() << "writing itunes author tag: ";
        operationResultStandard =
                setAtomData(author, ATOM_PATH_ITUNES_AUTHOR);
    }

    if (m_operationMode & OPERATION_MODE_XMP) {
        
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->setAuthor(author);
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }

    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultItunes   != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
        }

        return operationResult;
}




OperationResult Mp4Backend::setGeotag(const QByteArray& geotag)
{
    OperationResult operationResult         = OPERATION_OK;
    OperationResult operationResultStandard = OPERATION_OK;
    OperationResult operationResultXmp      = OPERATION_OK;

    if (m_operationMode & OPERATION_MODE_STANDARD) {
        qDebug() << "writing standard geotag";
        operationResultStandard =
                setAtomData(geotag, ATOM_PATH_GEOTAG);
        Metaman::Atom * atom = Mpeg4AtomUtility::findAtom(ATOM_PATH_GEOTAG, 
                                                          m_rootAtom);
        if (atom == 0) {
            qWarning () << "Could not find geotag atom";
        } else {
            qDebug() << "loci now: " << atom->size();
        }
    }

    if (m_operationMode & OPERATION_MODE_XMP) {
        
        if (m_xmpHandler != 0) {
            operationResultXmp = m_xmpHandler->setGeotag(geotag);
        }
        else {
            operationResultXmp = OPERATION_GENERAL_ERROR;
        }

    }

    if (operationResultStandard != OPERATION_OK ||
        operationResultXmp      != OPERATION_OK) {

        operationResult = OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}



OperationResult Mp4Backend::setAtomData(const QByteArray& content,
                                        const QByteArray& atomPath,
                                        const qint32 atomFlags)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;
    const QByteArray& dataAtomPath = atomPath +
                                     ATOM_PATH_SEPARATOR +
                                     ATOM_NAME_DATA;
    Atom* dataAtom = Mpeg4AtomUtility::findAtom(dataAtomPath, m_rootAtom);
    QByteArray atomDataDefinition((char*)(&atomFlags),
                                  ATOM_DATA_DEFINITION_LENGTH);
    const char fillCharacter = 0;
    QByteArray nullBytes(ATOM_DATA_DEFINITION_LENGTH, fillCharacter);
    QByteArray atomData = atomDataDefinition + nullBytes + content;

    if (dataAtom != 0) {
        // modify existing atom
        qDebug() << "atom found, modifyind its data";
        dataAtom->setData(atomData);
        operationResult = OPERATION_OK;
    }
    else {
        // create atom
        qDebug() << "atom not found, creating the atom (and its parents if needed)";
        Atom* atom =
            Mpeg4AtomUtility::ensureAtomHierarchy(atomPath, m_rootAtom);
        Atom* dataAtom = Mpeg4AtomUtility::createAtom(ATOM_NAME_DATA,
                                                      ATOM_TYPE_DATA,
                                                      atomData,
                                                      atom);

        if (dataAtom != 0) {
            operationResult = OPERATION_OK;
        }

    }

    return operationResult;
}



OperationResult Mp4Backend::setAtomData(const QByteArray& content,
                                        const QByteArray& atomPath)
{
    OperationResult operationResult = OPERATION_GENERAL_ERROR;
    Atom* atom = Mpeg4AtomUtility::findAtom(atomPath, m_rootAtom);

    if (atom != 0) {
        // modify existing atom
        qDebug() << "atom found, modifyind its data";
        atom->setData(content);
        operationResult = OPERATION_OK;
    }
    else {
        // create atom
        QByteArray parentPath = Mpeg4AtomUtility::truncateAtomPathTail(atomPath); 
        qDebug() << "atom not found, creating the atom (and its parents if needed)";
        Atom* parentAtom =
               Mpeg4AtomUtility::ensureAtomHierarchy(parentPath, m_rootAtom);
        QByteArray atomName = atomPath.split(*ATOM_PATH_SEPARATOR.data()).last();
        AtomType atomType = Mpeg4AtomUtility::resolveAtomType(atomName);
        atom = Mpeg4AtomUtility::createAtom(atomName,
                                            atomType,
                                            content,
                                            parentAtom);

        if (atom != 0) {
            operationResult = OPERATION_OK;
            qDebug() << "created atom " << atomName << ", size " 
                << atom->size();
        } else {
            qWarning() << "Mpeg4AtomUtility::createAtom returned NULL ptr";
        }
    }

    return operationResult;
}



OperationResult Mp4Backend::readFile(QFile& inputFile)
{
    qDebug() << "readFile()";
    OperationResult operationResult = OPERATION_GENERAL_ERROR;
    m_sourceFile = &inputFile;

    if (inputFile.isOpen()) {
        inputFile.seek(0);
        QDataStream in(&inputFile);

        // As the root atom is a fake atom, these properties are really not
        // meaningful.
        QByteArray rootAtomName("");
        QByteArray emptyContents;
        qint32 rootAtomOriginalSize = -1;
        qint32 locationOfRootAtomInFile = -1;
        Atom* parentForRootAtom = 0;

        m_rootAtom = new Atom(rootAtomName,
                              ATOM_TYPE_PSEUDO,
                              ATOM_STORAGE_MEMORY,
                              rootAtomOriginalSize,
                              emptyContents,
                              locationOfRootAtomInFile,
                              parentForRootAtom);

        // This is a failsafe guard that prevents an infinite loop
        int counter = 0; 

        while (!in.atEnd() && counter <= MAX_ATOMS) {
            Atom* atom = Mpeg4AtomUtility::readAtom(in, m_rootAtom);
            m_rootAtom->appendChild(atom);
            counter++;
        }

        if (m_operationMode & OPERATION_MODE_XMP) {
            operationResult = prepareXmpHandler();
        }
        else {
            operationResult = OPERATION_OK;
        }
    }

    return operationResult;
}



OperationResult Mp4Backend::writeFile(QFile& outputFile)
{
    qDebug() << "Mp4Backend::writeFile()";

    if (m_rootAtom == 0) {
        qWarning () << "m_rootAtom is a NULL ptr";
        return OPERATION_GENERAL_ERROR;
    }

    qDebug() << "top level children:";
    foreach (Atom* atom, m_rootAtom->children()) {
        qDebug() << atom->name();
    }

    OperationResult operationResult = dropUnhandledMetadata();

    if (m_operationMode & OPERATION_MODE_XMP) {
        // This will update the data of the xmp atom.
        operationResult = finalizeXmpProcessing();
    }
    
    if (outputFile.isOpen() &&
        m_rootAtom != 0     &&
        operationResult == OPERATION_OK) {

        outputFile.seek(0);
        QDataStream out(&outputFile);

        foreach(Atom* atom, m_rootAtom->children()) {
            OperationResult result = Mpeg4AtomUtility::writeAtom(out, atom);

            if (result != OPERATION_OK) {
                operationResult = result;
                break;
            }
        }

        qDebug() << "file written";
    }

    return operationResult;
}



bool Mp4Backend::ableToProcess(QFile& inputFile)
{
    qDebug() << "ableToProcess()";

    if (!inputFile.isOpen()) {
        return false;
    }

    if (Mpeg4AtomUtility::has64BitAtoms(inputFile)) {
        return false;
    }

    QByteArray brand = readBrand(inputFile);

    if (brand != BRAND_3GP4 && brand != BRAND_MP42) {
        // It is also possible that this handler works on other brands too,
        // but those haven't been tested.
        return false;
    }

    qint32 index = 0;
    QByteArray atomName;
    qint32 atomSize = 0;

    inputFile.seek(0);
    QList<QByteArray> atomNames;

    int failsafeCounter = 0;
    qDebug() << "file size " << inputFile.size();
    while(!inputFile.atEnd() && failsafeCounter <= MAX_ATOMS) {
        Mpeg4AtomUtility::readAtomInfoFromFile(inputFile,
                                               index,
                                               atomName,
                                               atomSize);
        qDebug() << "found atom" << atomName << atomSize << index;
       atomNames.append(atomName);
       index += atomSize;
       qDebug() << "index after round: " << index;

       failsafeCounter++;
    }

    if (failsafeCounter > MAX_ATOMS) {
        qDebug() << "failsafe guard aborted the processing";
        return false;
    }

    if (atomNames.indexOf(ATOM_NAME_MOOV) < atomNames.indexOf(ATOM_NAME_MDAT)){
        // It is extremely essential to maintain the mdat atom position in the
        // file because this backend doesn't currently implement for example
        // sample table chunk offset recalculation. This means that if mdat
        // position changes, the video file becomes unplayable.
        // If moov atom is placed after mdat atom, modifications in the
        // metadata don't affect mdat element position.
        return false;
    }

    inputFile.seek(index);

    if (!inputFile.atEnd()) {
        return false;
    }

    return true;
}



OperationResult Mp4Backend::prepareXmpHandler()
{
    qDebug() << "prepareXmpHandler()";
    
    int preambleLength = ATOM_SIZE_PREAMBLE_LENGTH + ATOM_NAME_PREAMBLE_LENGTH;
    
    m_xmpHandler = new XmpHandler;
    
    //Try first normal location of XMP data
    Atom* xmpAtom = Mpeg4AtomUtility::findAtom(ATOM_PATH_XMPDATA, m_rootAtom);
    
    //Try uuid alternative if not found
    if (xmpAtom == 0) {
        xmpAtom = Mpeg4AtomUtility::findAtom (ATOM_PATH_UUID, m_rootAtom);
        if (xmpAtom != 0) {
            QByteArray uuid = xmpAtom->collapse().mid (preambleLength, 16);
            if (uuid.toHex() == XMP_UUID_HEX) {
                m_xmpReadFromUUID = true;
            } else {
                xmpAtom = 0;
            }
        }
    } else {
        m_xmpReadFromUUID = false;
    }

    QByteArray xmpData;

    if (xmpAtom != 0) {
        // Atom::data() will return the complete atom data including name and
        // size preambles. However, those cannot be passed to the xmp handler
        // and they need to be dropped from the xmp data.
        qint32 xmpAtomSize = xmpAtom->size();
        
        if (m_xmpReadFromUUID == false) {
            xmpData = xmpAtom->collapse().right(xmpAtomSize - preambleLength);
        } else {
            xmpData = xmpAtom->collapse().right (
                xmpAtomSize - preambleLength - 16);
        }
    }

    OperationResult result = OPERATION_OK;
        
    if (m_xmpHandler != 0) {
        m_xmpHandler->setXmpDataToBeProcessed(xmpData);
    }
    else {
        qDebug() << "unable to prepare xmp processing";
        delete m_xmpHandler;
        m_xmpHandler = 0;
        result = OPERATION_GENERAL_ERROR;
    }

    return result;
}



OperationResult Mp4Backend::finalizeXmpProcessing()
{
    OperationResult result = OPERATION_OK;

    if (m_xmpHandler != 0) {
        QByteArray xmpData = m_xmpHandler->getProcessedData();
        Atom* xmpAtom = 0;
        
        if (m_xmpReadFromUUID == false) {
            xmpAtom = Mpeg4AtomUtility::findAtom(ATOM_PATH_XMPDATA, m_rootAtom);
        } else {
            xmpAtom = Mpeg4AtomUtility::findAtom (ATOM_PATH_UUID, m_rootAtom);
        }

        if (xmpAtom == 0) {
           Atom* parent =
                Mpeg4AtomUtility::ensureAtomHierarchy(ATOM_PATH_USERDATA,
                                                      m_rootAtom);
           AtomType atomType = Mpeg4AtomUtility::resolveAtomType(ATOM_NAME_XMP);
           Mpeg4AtomUtility::createAtom(ATOM_NAME_XMP,
                                        atomType,
                                        xmpData,
                                        parent);
        }
        else {
        
            if (m_xmpReadFromUUID == false) {
                xmpAtom->setData(xmpData);
            } else {
                QByteArray uuidContent = QByteArray::fromHex (XMP_UUID_HEX);
                uuidContent.append (xmpData);
                xmpAtom->setData (uuidContent);
            }
        }

        qDebug() << "output data: " << xmpData;
    }
    else {
        result = OPERATION_GENERAL_ERROR;
        qDebug() << "unable to finalize xmp processing";
    }

    return result;
}



QByteArray Mp4Backend::readBrand(QFile& inputFile)
{
    QByteArray brand;

    if (inputFile.isOpen()) {
        inputFile.seek(FILE_BRAND_OFFSET);
        QDataStream in(&inputFile);

        // first bytes of atom data should tell enough info
        brand = inputFile.read(ATOM_NAME_PREAMBLE_LENGTH);
    }

    return brand;
}



OperationResult Mp4Backend::dropUnhandledMetadata()
{
    qDebug() << "dropUnhandledMetadata";

    OperationResult result = OPERATION_OK;

    if (!(m_operationMode & OPERATION_MODE_ITUNES)) {
        OperationResult dropResult =
                Mpeg4AtomUtility::deleteAtom(ATOM_PATH_ITUNESDATA, m_rootAtom);

        if (dropResult != OPERATION_OK) {
            result = OPERATION_GENERAL_ERROR;
        }
    }

    if (!(m_operationMode & OPERATION_MODE_XMP)) {
        OperationResult dropResult =
            Mpeg4AtomUtility::deleteAtom(ATOM_PATH_XMPDATA, m_rootAtom);

        if (dropResult != OPERATION_OK) {
            result = OPERATION_GENERAL_ERROR;
        }
    }

    return result;
}



OperationResult Mp4Backend::setGpsString(const QByteArray& GPSLatitude,
                                         const QByteArray& GPSLongitude,
                                         const QByteArray& GPSAltitude,
                                         const QByteArray& GPSAltitudeRef)
{
    OperationResult operationResult = OPERATION_OK;

    // Using GPS data is only applicable with XMP metadata. There is no
    // standard way to write GPS string on atom level.
    if (m_operationMode & OPERATION_MODE_XMP) {
        operationResult = m_xmpHandler->setGpsString(GPSLatitude,
                                                     GPSLongitude,
                                                     GPSAltitude,
                                                     GPSAltitudeRef);
    }

    return operationResult;
}



OperationResult Mp4Backend::setKeywords(const QList<QByteArray>& keywords)
{
    OperationResult operationResult = OPERATION_OK;

    if (m_operationMode & OPERATION_MODE_XMP) {
        operationResult = m_xmpHandler->setKeywords(keywords);
    }
    
    return operationResult;
}
