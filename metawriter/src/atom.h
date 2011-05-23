
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



 
#ifndef ATOM_H
#define ATOM_H

#include <QList>
#include <QByteArray>
#include "metamandatatypes.h"

class QFile;

namespace Metaman {
    
/*!
    \class Atom
    \brief Atom class represents an ISO base media file format atom
    \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class Atom
{
public:
       
    //static AtomStorage resolveAtomStorage(const QByteArray& atomName);
    
    /**
     * \brief Constructor
     * @param name Atom name
     * @param type Atom type
     * @param storage Atom storage (memory/file)
     * @param size Atom size
     * @param contents Atom contents (if data or hybrid atom)
     * @param locationInFile Original location in source file or 0 if N/A
     * @param analyzed True if the data block of the atom has been analyzed
     * @param parent Pointer to parent atom
     */
    Atom(const QByteArray& name,
         AtomType type,
         AtomStorage storage,
         qint32 size,
         const QByteArray& contents,
         qint32 locationInFile,
         bool expandable,
         Atom* parent = 0,
         QFile* sourceFile = 0);
    
    /**
     * \brief Destructor
     */
    ~Atom();

    /**
     * \brief Comparison operator
     * @param other Atom to be compared with
     * @return True if atoms are equal, false if not
     */
    bool operator==(const Atom& other);
    
    /**
     * \brief Comparison operator
     * @param other Atom to be compared with
     * @return True if atoms are *not* equal, false if they are
     */
    bool operator!=(const Atom& other);
    
    /**
     * \brief Comparison operator
     * @param other Atom data to be compared with
     * @return True if atoms are equal, false if not
     */
    bool operator==(const QByteArray& other);
    
    /**
     * \brief Comparison operator
     * @param other Atom data to be compared with
     * @return True if atoms are *not* equal, false if they are
     */
    bool operator!=(const QByteArray& other);
    
    /**
     * \brief Returns a pointer to parent atom
     * @return Pointer to parent atom
     */
    Atom* parentAtom() const;
    
    /**
     * \brief Returns the atom storage location (memory/file)
     * @return Atom storage location
     */
    AtomStorage storage() const;
    
    /**
     * \brief Returns the type of the atom 
     * @return Atom type
     */
    AtomType type() const;
    
    /**
     * \brief Return the name of the atom
     * @return Atom name
     */
    QByteArray name() const;
    
    /**
     * \brief Collect all atom data (including children) to one data chunk
     * Think collapsing an atom tree as closing a folder in the tree view of 
     * file managers: it doesn't change the contents, but only makes the view 
     * more compact. After collapsing, interaction with the contents cannot be
     * done before expanding the atom again.
     * @return Atom contents (including preambles and children)
     */
    QByteArray collapse();
    
    /**
     * \brief Returns a partial block of atom data
     * Used for mdat atom. Its data is not stored in memory but read directly
     * from the source file.
     * @param offset Offset from the beginning of the data
     * @param chunkSize Size of the chunk to be returned
     * @return Data chunk
     */
    QByteArray dataChunk(qint32 offset, qint32 chunkSize) const;
    
    /**
     * \brief Set the data of the atom
     * @param data Data to be set
     */
    void setData(const QByteArray& data);
    
    /**
     * \brief Returns the original size of the atom 
     * Only applicable for the atoms read from a source file
     * @return The original size of the atom
     */
    qint32 originalSize() const;
    
    /**
     * \brief Returns the location of the atom in the source file
     * Only applicable for the atoms read from a source file
     * @return Location of the atom as an offset from the beginning of the file
     */
    qint32 locationInFile() const;
    
    /**
     * \brief Returns the current size of the atom
     * Includes children and preambles
     * @return The size of the atom
     */
    qint32 size() const;
    
    /**
     * \brief Returns a list of the children of the atom
     * @return The children of the atom as a list of Atom pointers
     */
    QList<Atom*> children();
    
    /**
     * \brief Tells if the atom is analyzed or not
     * Unanalyzed data can contain both normal data and children. Handling the
     * contents of an atom requires that the data is analyzed.
     * @return True if analyzed, false if not
     */
    bool isExpandable() const;
    
    /**
     * \brief Removes a child atom
     * @param child The child atom the be removed
     */
    void removeChild(Atom* child);
    
    /**
     * \brief Adds a child atom to place denoted by index
     * @param child The child atom to be added
     * @param index The position for the child atom in the children list
     */
    void addChild(Atom* child, int index = 0); // Takes ownership

    /**
     * \brief Adds a child atom to the end of children list
     * @param child The child atom to be added
     */
    void appendChild(Atom* child); // Takes ownership

    /**
     * \brief Adds a child atom to the beginning of children list
     * @param child The child atom to be added
     */
    void prependChild(Atom* child); // Takes ownership
    
    /**
     * \brief Tells if the atom has the child given as parameter
     * @param child The atom to be checked
     * @return True is the atom is paren't child, false if not
     */
    bool hasChild(Atom* child) const;
    
    /**
     * \brief Tells if the atom has the child given as parameter
     * @param name Atom name to be checked
     * @param foundChild (out) Pointer to the child if it was found
     * @return True is the atom is paren't child, false if not
     */
    Atom* hasChild(const QByteArray& name);
    
    /**
     * \brief Analyzes the atom (raw) data and divides it into parts
     * Think expanding atoms as the tree view of file managers: interacting
     * with the contents of a folder requires opening the folder, i.e expanding
     * the view. Expanding doesn't change the contents, it just makes it more
     * visible.
     * @return True if analysis was successful, false if not
     */
    bool expand();


    /**
     * \brief Return the data block of the atom
     * This means data and only the data, not children - unless the atom is not
     * not analyzed. In that case the children are naturally included in the
     * data block.
     */
    QByteArray data();
    
private:

    /**
     * \brief Constructs a list of allocated child atoms from the given data
     * This assumes that the given data contains only child atoms
     * @param data Data block that contains the child atoms
     * @return The list of children
     */
    QList<Atom*> extractChildren(const QByteArray& data);
      
    /**
     * \brief A utility function that analyzes a data atom
     */
    void analyzeDataAtom();
    
    /**
     * \brief A utility function that analyzes a container atom
     */
    void analyzeContainerAtom();
    
    /**
     * \brief A utility function that analyzes a hybrid atom
     */
    void analyzeHybridAtom();

private:
    /// The name of the atom
    QByteArray      m_name;

    /// The type of the atom
    AtomType        m_type;

    /// The storage of the atom
    AtomStorage     m_storage;

    /// List of child atoms
    QList<Atom*>    m_children;

    /// Original size of the atom
    qint32          m_originalSize;

    /// Location of the atom in the source file
    qint32          m_locationInFile;

    /// Data of the atom. Doesn't include preambles
    QByteArray      m_data;

    /// Tells if the atom is expandable or not
    bool            m_expandable;

    /// A pointer to the parent atom
    Atom*           m_parent;

    /// A pointer to the file this atom was read from. Can be NULL.
    QFile*          m_sourceFile;
};

}

#endif
