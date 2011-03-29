
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



 
#ifndef MP4BACKEND_H
#define MP4BACKEND_H

#include <backendinterface.h>
#include "metamandatatypes.h"

class QByteArray;
class QDataStream;
class QFile;

namespace Metaman {
    class Atom;
    class XmpHandler;
    
/*!
    \class MetaApplication
    \brief The main interface to the metadata handler
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class Mp4Backend : public BackendInterface
{
public:
    /**
     * \brief Constructor
     * @param operationMode Operation mode
     */
    Mp4Backend(OperationMode operationMode = OPERATION_MODE_DUAL);
    
    /**
     * \brief Destructor
     */
    ~Mp4Backend();

    /// \reimp
    virtual OperationResult readFile(QFile& inputFile);
    virtual OperationResult writeFile(QFile& outputFile);
    virtual OperationResult removeAllMetaData();
    virtual OperationResult removeAuthor();
    virtual OperationResult removeDescription();
    virtual OperationResult removeGpsString();
    virtual OperationResult removeGeoTag();
    virtual OperationResult removeTitle();
    virtual OperationResult setDescription(const QByteArray& description);
    virtual OperationResult setTitle(const QByteArray& title);
    virtual OperationResult setAuthor(const QByteArray& author);
    virtual OperationResult setGpsString(const QByteArray& GPSLatitude,
                                         const QByteArray& GPSLongitude,
                                         const QByteArray& GPSAltitude,
                                         const QByteArray& GPSAltitudeRef);
    virtual OperationResult setKeywords(const QList<QByteArray>& keywords);
    virtual OperationResult setGeotag(const QByteArray& geotag);
    virtual bool ableToProcess(QFile& inputFile);
    /// \reimp_end

private:


    /**
     * \brief Remove XMP data
     * @return Operation result
     */
    OperationResult removeXMPData();

    /**
     * \brief Remove iTunes data
     * @return Operation result
     */
    OperationResult removeItunesData();

    /**
     * \brief Make sure that the specified atom hierarchy exists.
     * The atom tree is created if needed.
     * @param path Atom path to assure
     * @param rootAtom A pointer to the top level atom of the hierarchy
     * @return A pointer to the lowest atom in the tree
     */
    Atom* ensureAtomHierarchy(const QByteArray& path, Atom* rootAtom);
    
    /**
     * \brief Set atom data
     * This function modifies the data of an atom, not XMP data.
     * To be used for atom that wrap their contents in data-atom.
     * @param content Atom content
     * @param atomPath Path to the atom of which data to modify
     * @param atomFlags Atom flags
     * @return Operation result
     */
    OperationResult setAtomData(const QByteArray& content,
                                const QByteArray& atomPath,
                                const qint32 atomFlags);


    /**
     * \brief Set atom data
     * This function modifies the data of an atom, not XMP data.
     * To be used for atom that do *not* wrap their contents in data-atom.
     * @param content Atom content
     * @param atomPath Path to the atom of which data to modify
     * @return Operation result
     */
    OperationResult setAtomData(const QByteArray& content,
                                const QByteArray& atomPath);
    
    /**
     * \brief Does some prepation for XMP data handling
     */
    OperationResult prepareXmpHandler();
    
    /**
     * \brief Performs XMP processing finalization routines
     */
    OperationResult finalizeXmpProcessing();

    /**
     * \brief Read the file format brand from the input file
     * \param inputFile Input file
     * \return File format brand
     */
    QByteArray readBrand(QFile& inputFile);

    /**
     * \brief Drop metadata that is selected not to be handled
     * This is to ensure that there are no conflicting metadata.
     * Only iTunes style tags and XMP metadata can be dropped.
     * \return Operation status
     */
    OperationResult dropUnhandledMetadata();

private:

    /// A pointer to the root atom
    Atom* m_rootAtom;

    /// Source file
    QFile* m_sourceFile;

    /// Operation mode
    OperationMode m_operationMode;

    /// A pointer to the XMP handler
    XmpHandler* m_xmpHandler;
    
    // If true XMP was read from UUID
    bool m_xmpReadFromUUID;
    
};

}

#endif
