
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



 
#include "mpeg4atomutility.h"
#include "magic.h"
#include "atom.h"
#include "metamandatatypes.h"
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QList>
#include <QDebug>

QByteArray Mpeg4AtomUtility::findAtomPath(Metaman::Atom* atom)
{
    QByteArray path;
    Metaman::Atom* currentAtom = atom;

    while (currentAtom != 0) {
        path.prepend(currentAtom->name());
        Metaman::Atom* parent = currentAtom->parentAtom();

        if (parent != 0) {
            path.prepend(Metaman::ATOM_PATH_SEPARATOR);
        }

        currentAtom = parent;
    }

    return path;
}



Metaman::Atom* Mpeg4AtomUtility::readAtom(QDataStream& in,
                                          Metaman::Atom* atomParent)
{
    qDebug() << "readAtom()";
    qint32 locationInFile = -1;

    if (in.device() != 0) {
        locationInFile = (qint32)in.device()->pos();
    }

    QFile* inputFile = qobject_cast<QFile*>(in.device());
    if (inputFile == 0) {
        qCritical () << "Could not cast input data stream to file";
        return 0;
    }
    qint32 sizePreambleLength = Metaman::ATOM_SIZE_PREAMBLE_LENGTH;
    
    QByteArray atomSizeBytes = inputFile->read(sizePreambleLength);
    qint32 atomSize = atomSizeBytes.toHex().toInt(0,16);
    QByteArray atomName = inputFile->read(Metaman::ATOM_NAME_PREAMBLE_LENGTH);
    
    // Size and name are included also here because of how expand() and 
    // collapse() work.
    QByteArray atomData = atomSizeBytes + atomName;

    Metaman::AtomType atomType = resolveAtomType(atomName);
    Metaman::AtomStorage atomStorage = resolveAtomStorage(atomName);

    if (atomStorage == Metaman::ATOM_STORAGE_MEMORY) {
        atomData += inputFile->read(atomSize -
                                    Metaman::ATOM_SIZE_PREAMBLE_LENGTH -
                                    Metaman::ATOM_NAME_PREAMBLE_LENGTH);
        qDebug() << "read " << atomData.size() << " bytes of data";
    }
    else {
        qint32 currentPos = inputFile->pos();
        inputFile->seek(currentPos +
                        atomSize -
                        Metaman::ATOM_SIZE_PREAMBLE_LENGTH -
                        Metaman::ATOM_NAME_PREAMBLE_LENGTH);
    }

    bool expandable = true;

    Metaman::Atom* atom = new Metaman::Atom(atomName,
                                            atomType,
                                            atomStorage,
                                            atomSize,
                                            atomData,
                                            locationInFile,
                                            expandable,
                                            atomParent,
                                            inputFile);

    return atom;
}




Metaman::OperationResult Mpeg4AtomUtility::writeAtom(QDataStream& out, Metaman::Atom* atom)
{
    Metaman::OperationResult operationResult = Metaman::OPERATION_OK;

    if (atom == 0) {
        // Something has gone wrong somewhere -> abort
        operationResult = Metaman::OPERATION_GENERAL_ERROR;

        // If this happens somewhere in the middle of the file,
        // some part of the output file has already been written
        // and we need to get rid of the partial file before quitting.
        QFile* outputFile = qobject_cast<QFile*>(out.device());
        
        if (outputFile != 0) {
            outputFile->remove();
        }

        return operationResult;
    }

    if (atom->storage() == Metaman::ATOM_STORAGE_MEMORY) {
        QByteArray data = atom->collapse();
        qDebug() << "WRITING ATOM SIZE " << data.size() << "/" << atom->size();
        out.writeRawData(data, data.size());
    }
    else {
        qint32 atomSize = atom->originalSize();
        qint32 remaining = atomSize;
        qint32 chunkSize = Metaman::DEFAULT_DATA_CHUNK_SIZE;

        while (remaining > 0) {

            if (remaining < chunkSize) {
                chunkSize = remaining;
            }

            qint32 dataOffset    = atomSize - remaining;
            QByteArray dataChunk = atom->dataChunk(dataOffset, chunkSize);
            qint32 bytesRead     = dataChunk.size();
            out.writeRawData(dataChunk, bytesRead);
            remaining -= bytesRead;
        }

    }

    return operationResult;
}



bool Mpeg4AtomUtility::has64BitAtoms(QFile& inputFile)
{
    bool hasLargeAtoms = false;

    if (inputFile.isOpen()) {
        inputFile.seek(0);
        QByteArray atomLength = inputFile.read(Metaman::ATOM_SIZE_PREAMBLE_LENGTH);

        if (atomLength.toInt() == 1) {
            hasLargeAtoms = true;
        }
    }
    else {
        // This should never happen, but if the file cannot be read,
        // mark this test as failed to be safe.
        hasLargeAtoms = true;
    }

    return hasLargeAtoms;
}



bool Mpeg4AtomUtility::readAtomInfoFromFile(QFile& inputFile,
                                            qint32 index,
                                            QByteArray& atomName,
                                            qint32& atomSize)
{
    bool success = false;

    if (inputFile.isOpen()) {
        inputFile.seek(index);
        QByteArray size = inputFile.read(Metaman::ATOM_SIZE_PREAMBLE_LENGTH);
        atomSize = size.toHex().toInt(0,16);
        atomName = inputFile.read(Metaman::ATOM_NAME_PREAMBLE_LENGTH);
        success = true;
    }

    return success;
}



Metaman::Atom* Mpeg4AtomUtility::findAtom(QList<QByteArray> atoms,
                                          Metaman::Atom* rootAtom)
{
    // A recursive function that searches for the last item of "atoms" from
    // the hierarchy described by "atoms", the current highest level atom
    // being rootAtom.

    QByteArray tryingtofind = atoms.first();
    qDebug() << "findAtom(): trying to find " << tryingtofind;
    Metaman::Atom* foundAtom = 0;
    Metaman::Atom* nextAtom = 0;

    if (rootAtom != 0) {
        if (rootAtom->isExpandable()) {
            rootAtom->expand();
        }

        nextAtom = rootAtom->hasChild(atoms.takeFirst());
        
        if (nextAtom != 0) {
            if (!atoms.isEmpty()) {
                foundAtom = findAtom(atoms, nextAtom);
            }
            else {
                foundAtom = nextAtom;
            }
        }
    }

    if (foundAtom !=0) {
        qDebug() << "Atom  " << foundAtom->name() << " was found";
    }
    else {
        qDebug() << "Atom was not found";
    }

    return foundAtom;
}



Metaman::Atom* Mpeg4AtomUtility::findAtom(const QByteArray& path,
                                          Metaman::Atom* rootAtom)
{
    // This is an overloaded convenience function
    qDebug() << "findAtom() " << path;
    char pathSeparator = Metaman::ATOM_PATH_SEPARATOR.at(0);
    QList<QByteArray> atoms = path.split(pathSeparator);
    return findAtom(atoms, rootAtom);
}



Metaman::OperationResult Mpeg4AtomUtility::deleteAtom(const QByteArray& path,
                                                      Metaman::Atom* rootAtom)
{
    qDebug() << "deleteAtom() : " << path;

    Metaman::OperationResult operationResult = Metaman::OPERATION_OK;
    char pathSeparator = Metaman::ATOM_PATH_SEPARATOR.at(0);
    QList<QByteArray> atoms = path.split(pathSeparator);

    if (!atoms.isEmpty()) {

        // The parent atom of the atom to be deleted
        Metaman::Atom* atomToBeDeleted = findAtom(atoms, rootAtom);
        Metaman::Atom* parent = 0;

        if (atomToBeDeleted != 0) {
            parent = atomToBeDeleted->parentAtom();
        }

        if (parent != 0 && atomToBeDeleted != 0) {
            parent->removeChild(atomToBeDeleted);
            operationResult = Metaman::OPERATION_OK;
        }
        else {
            qDebug() << "could not delete atom";
        }
    }

    return operationResult;
}



Metaman::Atom* Mpeg4AtomUtility::createAtom(const QByteArray& name,
                                            Metaman::AtomType type,
                                            const QByteArray& contents,
                                            Metaman::Atom* parent)
{
    qDebug() << "createAtom(): " << name;

    if (name.size() != Metaman::ATOM_NAME_PREAMBLE_LENGTH) {
        qWarning() << "Unexpected atom name length";
        return 0;
    }

    if (parent == 0) {
        qWarning() << "cannot add child to a null parent atom";
        return 0;
    }

    Metaman::Atom* newAtom = 0;
       
    if (parent->isExpandable()) {
        parent->expand();
    }

    qint32 originalSize = contents.size() +
                          Metaman::ATOM_SIZE_PREAMBLE_LENGTH +
                          Metaman::ATOM_NAME_PREAMBLE_LENGTH;
    qint32 locationInFile = -1;

    // The atom is not expandable (i.e analyzed) because we already know what's
    // in it and we don't add any unanalyzed data to it.
    bool expandable = false;

    newAtom = new Metaman::Atom(name,
                                type,
                                Metaman::ATOM_STORAGE_MEMORY,
                                originalSize,
                                contents,
                                locationInFile,
                                expandable,
                                parent);

    if (newAtom != 0) {
        qDebug() << "adding atom to parent's child list: "<< newAtom;
        parent->addChild(newAtom);
    }
    else {
        qWarning() << "failed to create a new atom";
    }
    
    return newAtom;
}



Metaman::Atom* Mpeg4AtomUtility::ensureAtomHierarchy(const QByteArray& path,
                                                     Metaman::Atom* rootAtom)
{
    qDebug() << "ensureAtomHierarchy() : " << path;

    char pathSeparator = Metaman::ATOM_PATH_SEPARATOR.at(0);
    QList<QByteArray> atoms = path.split(pathSeparator);
    Metaman::Atom* parent = rootAtom;
    QByteArray currentDepth;

    foreach (const QByteArray atomName, atoms) {
        qDebug() << "ensuring that " << atomName << " exists.";

        currentDepth.append(atomName);
        Metaman::Atom* currentAtom = findAtom(currentDepth, rootAtom);

        if (currentAtom == 0) {
            Metaman::AtomType atomType = Metaman::ATOM_TYPE_UNKNOWN;
            QByteArray emptyContents;

            if (atomName == Metaman::ATOM_NAME_META) {
                qDebug() << "doing some magic for meta atom";
                emptyContents.resize(4);
                emptyContents.fill(0);
            }

            currentAtom = createAtom(atomName, atomType, emptyContents, parent);
        }

        parent = currentAtom;
        currentDepth.append(pathSeparator);
    }

    return parent;
}



Metaman::AtomType Mpeg4AtomUtility::resolveAtomType(const QByteArray& atomName)
{
    qDebug() << "resolveAtomType() :" << atomName;

    Metaman::AtomType atomType = Metaman::ATOM_TYPE_UNKNOWN;
    const int numOfKnownAtoms = sizeof(Metaman::atomProperties) /
                                sizeof(Metaman::AtomProperty);

     for (int i=0; i < numOfKnownAtoms; i++) {
         if (Metaman::atomProperties[i].name == atomName) {
             atomType = Metaman::atomProperties[i].type;
         }
     }

    qDebug() << "resolveAtomType() :" << atomName.data() << ": " << atomType;
    return atomType;
}



Metaman::AtomStorage Mpeg4AtomUtility::resolveAtomStorage(
    const QByteArray& atomName)
{
    Metaman::AtomStorage storage = Metaman::ATOM_STORAGE_MEMORY;

    if (atomName == Metaman::ATOM_NAME_MDAT) {
        storage = Metaman::ATOM_STORAGE_FILE;
    }

    return storage;
}



bool Mpeg4AtomUtility::isKnownAtom(const QByteArray& atomName)
{
    return (resolveAtomType(atomName) != Metaman::ATOM_TYPE_UNKNOWN);
}



Metaman::OperationResult Mpeg4AtomUtility::adjustFreeSpace(Metaman::Atom* freeAtom, qint32 sizeDelta)
{
    bool result = false;

    if (freeAtom != 0 && freeAtom->name() == Metaman::ATOM_NAME_FREE) {
        qint32 currentSize = freeAtom->size();
        qint32 newSize = currentSize + sizeDelta;

        if (newSize > 0) {
            QByteArray atomData = freeAtom->data();
            atomData.fill(0, newSize);
            freeAtom->setData(atomData);
            result = true;
        }
        else {
            qWarning() << "Not enough free space";
        }
    }
    else {
        qWarning() << "Invalid atom";
    }

    Metaman::OperationResult operationResult = Metaman::OPERATION_OK;

    if (!result) {
        operationResult = Metaman::OPERATION_GENERAL_ERROR;
    }

    return operationResult;
}


QByteArray Mpeg4AtomUtility::truncateAtomPathTail(const QByteArray& path)
{
    QList<QByteArray> atomHierarchy = path.split(*Metaman::ATOM_PATH_SEPARATOR.data());
    atomHierarchy.removeLast();
    QByteArray truncatedPath;

    foreach(QByteArray atom, atomHierarchy){
        truncatedPath.append(atom);

        if (atom != atomHierarchy.last()) { 
            truncatedPath.append(Metaman::ATOM_PATH_SEPARATOR);
        }
    }

    return truncatedPath;
}