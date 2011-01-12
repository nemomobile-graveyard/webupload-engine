
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



 
#ifndef MPEG4ATOMUTILITY_H
#define MPEG4ATOMUTILITY_H

#include "metamandatatypes.h"
#include <QByteArray>
#include <QFile>
#include <QList>

namespace Metaman {
    class Atom;
}

namespace Mpeg4AtomUtility {
    /**
     * \brief Find the full path to the given atom
     * @param atom Atom for which to find the path
     * @return Full path to the atom
     */
    QByteArray findAtomPath(Metaman::Atom* atom);

    /**
     * \brief Read information about an atom directly from a file
     * @param inputFile Input file
     * @param index Offset to the atom beginning
     * @param atomName Name of the atom (out)
     * @param atomSize Size of the atom (out)
     * @return True on success, false on failure
     */
    bool readAtomInfoFromFile(QFile& inputFile,
                              qint32 index,
                              QByteArray& atomName,
                              qint32& atomSize);
    
    /**
     * \brief Check if the file has large (64 bit) atoms
     * @param inputFile The file to check
     * @return True if the file has large atoms, false if not
     */
    bool has64BitAtoms(QFile& inputFile);

    /**
     * \brief Read atom data and allocate an atom object
     * @param in Input data stream
     * @param atomParent Parent atom
     * @return A pointer to the created atom
     */
    Metaman::Atom* readAtom(QDataStream& in,
                            Metaman::Atom* atomParent);

    Metaman::OperationResult writeAtom(QDataStream& out, Metaman::Atom* atom);
    
    /**
     * \brief Find an atom from atom tree
     * @param atoms Full path to the atom as a list of atoms
     * @param rootAtom Root atom (to search from)
     * @return A pointer to the atom or null if not found
     */
    Metaman::Atom* findAtom(QList<QByteArray> atoms, Metaman::Atom* rootAtom);
    
    /**
     * \brief Find an atom from atom tree
     * @param path Full path to the atom 
     * @param rootAtom Root atom (to search from)
     * @return A pointer to the atom or null if not found
     */
    Metaman::Atom* findAtom(const QByteArray& path, Metaman::Atom* rootAtom);
    
    /**
     * \brief Delete an atom
     * @param path Full path to the atom 
     * @param rootAtom Root atom of the given atom tree (path)
     * @return Operation result
     */
    Metaman::OperationResult deleteAtom(const QByteArray& path, Metaman::Atom* rootAtom);
    
    /**
     * \brief Create a new atom 
     * @param name Atom name
     * @param type Atom type
     * @param contents Atom contents
     * @param parent Parent atom
     * @return A pointer to the atom or null if allocation failed
     */
    Metaman::Atom* createAtom(const QByteArray& name,
                              Metaman::AtomType type,
                              const QByteArray& contents,
                              Metaman::Atom* parent);

    /**
     * \brief Make sure that the specified atom hierarchy exists.
     * The atom tree is created if needed.
     * @param path Atom path to assure
     * @param rootAtom A pointer to the top level atom of the hierarchy
     * @return A pointer to the lowest atom in the tree
     */
    Metaman::Atom* ensureAtomHierarchy(const QByteArray& path, Metaman::Atom* rootAtom);
    
    /**
     * \brief Find the type of given atom based on its name
     * @param atomName Atom name
     * @return Atom type
     */
    Metaman::AtomType resolveAtomType(const QByteArray& atomName);

    /**
     * \brief Find the storage location of the given atom based on its name
     * @param atomName Atom name
     * @return Atom storage location
     */
    Metaman::AtomStorage resolveAtomStorage(const QByteArray& atomName);

    /**
     * \brief Check if the given atom name is a known atom 
     * @param atomName Atom name
     * @return True if a known atom, false if not
     */
    bool isKnownAtom(const QByteArray& atomName);

    /**
     * \brief Adjust atom "free" to compensate structural changes
     * @param freeAtom A pointer to the free atom
     * @param sizeDelta The amount of bytes to enlarge/shrink
     * @return Operarion result
     */
    Metaman::OperationResult adjustFreeSpace(Metaman::Atom* freeAtom, qint32 sizeDelta);

    /**
     * \brief Drop last atom from given atom path
     * Useful for getting the path of a parent atom
     * @param path Path to be truncated
     * @return Truncated path
     */
    QByteArray truncateAtomPathTail(const QByteArray& path);

}

#endif // MPEG4ATOMUTILITY_H