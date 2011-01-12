
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



 
#include "atom.h"
#include "magic.h"
#include "mpeg4atomutility.h"
#include <QDebug>
#include <QFile>
#include <QtEndian>
#include <QDataStream>
using namespace Metaman;

Atom::Atom(const QByteArray& name,
           AtomType type,
           AtomStorage storage,
           qint32 size,
           const QByteArray& contents,
           qint32 locationInFile,
           bool expandable,
           Atom* parent,
           QFile* sourceFile) :
    m_name(name),
    m_type(type),
    m_storage(storage),
    m_originalSize(size),
    m_locationInFile(locationInFile),
    m_data(contents),
    m_expandable(expandable),
    m_parent(parent),
    m_sourceFile(sourceFile)
{
    if (m_name.size() > ATOM_NAME_PREAMBLE_LENGTH) {
        m_name.truncate(ATOM_NAME_PREAMBLE_LENGTH);
    }
    else if (m_name.size() < ATOM_NAME_PREAMBLE_LENGTH) {
        // Any ascii filler character would be fine. We just need to make
        // sure it's not anything unexpected.
        char filler = '_'; 
        m_name.fill(filler);

        // This step is actually optional, because giving an atom name shorter
        // than expected length will invalidate the object anyway. However,
        // leaving the original bytes as they are will give a better 
        // understanding what's going on.
        m_name.replace(0, name.size(), name);
    }

    if (m_type == ATOM_TYPE_UNKNOWN) {
        m_type = Mpeg4AtomUtility::resolveAtomType(name);
    }

}


Atom::~Atom()
{
}



bool Atom::operator==(const Atom& other)
{
    return (other.name() == m_name);
}



bool Atom::operator!=(const Atom& other)
{
    return !(*this == other);
}



bool Atom::operator==(const QByteArray& other)
{
    return (other == m_name);
}



bool Atom::operator!=(const QByteArray& other)
{
    return !(*this == other);
}



Atom* Atom::parentAtom() const
{
    return m_parent;
}



AtomStorage Atom::storage() const
{
    return m_storage;
}



AtomType Atom::type() const
{
    return m_type;
}



QByteArray Atom::name() const
{
    return m_name;
}



QByteArray Atom::collapse() 
{
    qDebug() << "Atom::collapse() : "  << m_name << "(" << m_expandable << ")";

    QByteArray data;    

    // This check ensures size and name don't get included twice in the data.
    // for expandable atoms these are already included in m_data.
    if (!m_expandable) {
        qint32 atomSize = qToBigEndian(size());
        data.append((char*)(&atomSize), ATOM_SIZE_PREAMBLE_LENGTH);
        data.append(name());
    }

    qDebug() << "atom is expandable: " << m_expandable;
    qDebug() << "atom type: " << m_type;

    if (m_expandable ||
        m_type == ATOM_TYPE_DATA ||
        m_type == ATOM_TYPE_HYBRID ||
        m_type == ATOM_TYPE_UNKNOWN) {
        qDebug() << "appending data chunk for " << m_name << ", size " << m_data.size();
        data.append(m_data);
    }

    if (!m_expandable &&
        (m_type == ATOM_TYPE_CONTAINER ||
         m_type == ATOM_TYPE_HYBRID)) {

        qDebug() << "appending children (" << m_children.count() << ") for " << m_name;
        foreach (Atom* child, m_children) {
            data.append(child->collapse());
        }
    }

    m_data = data;
    qDeleteAll(m_children);
    m_children.clear();
    m_expandable = true;

    return data;
}



QByteArray Atom::dataChunk(qint32 offset, qint32 chunkSize) const
{
    QByteArray dataChunk;

    if (m_storage == ATOM_STORAGE_MEMORY) {
        dataChunk = m_data.mid(offset, chunkSize);
    }
    else if (m_storage == ATOM_STORAGE_FILE && m_sourceFile != 0) {

        if (m_sourceFile != 0 && m_sourceFile->isOpen()) {
            m_sourceFile->seek(locationInFile() + offset);
            dataChunk = m_sourceFile->read(chunkSize);
        }
    }
    else {
        qDebug() << "something is wrong, not copying any data";
    }

    return dataChunk;
}



void Atom::setData(const QByteArray& data)
{
    if (m_expandable) {
        expand();
    }

    m_data = data;
}



qint32 Atom::originalSize() const
{
    return m_originalSize;
}



qint32 Atom::size() const
{
    //qDebug() << "Atom::size() : " << m_name;
    qint32 size = 0;

    if (!m_expandable) {
	size += ATOM_SIZE_PREAMBLE_LENGTH + ATOM_NAME_PREAMBLE_LENGTH;
    }

    if (m_expandable ||
        m_type == ATOM_TYPE_DATA ||
        m_type == ATOM_TYPE_HYBRID ||
        m_type == ATOM_TYPE_UNKNOWN) {
        size += m_data.size();
    }

    if (!m_expandable &&
        (m_type == ATOM_TYPE_CONTAINER ||
         m_type == ATOM_TYPE_HYBRID)) {

        foreach (Atom* child, m_children) {
            size += child->size();
        }
    }
    
    return size;
}



QList<Atom*> Atom::children()
{
    if (isExpandable()) {
        expand();
    }
    
    return m_children;
}



bool Atom::isExpandable() const
{
    return m_expandable;
}



void Metaman::Atom::analyzeDataAtom()
{
    qDebug() << "m_type == ATOM_TYPE_DATA";

    int dataSize = m_data.size();

    if (ATOM_SIZE_PREAMBLE_LENGTH + ATOM_NAME_PREAMBLE_LENGTH + dataSize !=
        m_originalSize) {
        // Actual data size doesn't match what size header says
        m_expandable = true;
    }
    else {
        // Actual data size equals what size headers says. No need to
        // analyze the data further.
        m_expandable = false;
    }
}



void Metaman::Atom::analyzeContainerAtom()
{
    qDebug() << "analyzeContainerAtom()";

    qint32 sizeOfFirstChild =
            m_data.left(ATOM_SIZE_PREAMBLE_LENGTH).toHex().toInt(0,16);

    if (sizeOfFirstChild > m_data.size()) {
        // The atom claims to have more data than there actually is.
        // Something's wrong. Corrupted data or maybe this is a hybrid
        // atom, not a pure container.
        m_expandable = true;
    }
    else {
        // To make sure sure we don't create duplicate atoms, clear the
        // old list before populating it again.
        qDeleteAll(m_children);
        m_children.clear();
        m_children = extractChildren(m_data);
        m_expandable = false;
        m_data.clear(); // the data now exists as child atoms
    }
}



void Metaman::Atom::analyzeHybridAtom()
{
    qDebug() << "analyzeHybridAtom()";

    // This is where it gets challenging. Hybrid atoms can have (but not
    // always have) both data and child atoms, but we have no bulletproof
    // way to detect what comes after the atom name.

    int numOfKnownAtoms = sizeof(atomProperties) / sizeof(AtomProperty);
    qint32 offset = m_data.size(); // the index between atom data and children

    // This loop will find the index between atom data and its children by 
    // looking for known atom names from the data.
    // The lowest value found will be the known offset where data ends and
    // child data begins. It possible that the data part contains unknown
    // atoms, but that's OK as long as we don't need to handle those atoms.
    for (int i=0; i < numOfKnownAtoms; i++) {
        QByteArray candidate = atomProperties[i].name;

        // Note that we search for atom name, which is
        // ATOM_SIZE_PREAMBLE_LENGTH bytes after the real beginning in the
        // atom. Later we need to compensate this shift (*).
        qint32 index = m_data.indexOf(candidate);

        // @todo a safety check to filter out randon data that matches
        // the atom name by coincidence

        if (index >= 0 && index < offset) {
            offset = index;
        }
    }

    // (*) Related to the earlier comment, the offset is moved to the real
    // beginning of the atom.
    offset -= ATOM_SIZE_PREAMBLE_LENGTH;
    
    // Minimum size for an atom is ATOM_SIZE_PREAMBLE_LENGTH +
    // ATOM_NAME_PREAMBLE_LENGTH, so the last possible atom can be
    // ATOM_SIZE_PREAMBLE_LENGTH + ATOM_NAME_HEADER_LENGH before the end of the
    // data.
    const int lastPossibleAtomOffset =
            m_data.size() - ATOM_SIZE_PREAMBLE_LENGTH - ATOM_NAME_PREAMBLE_LENGTH;

    if (offset > lastPossibleAtomOffset) {
        // The loop iterated all the way to the end. No children were
        // found, although a hybrid atom should have some. No additional
        // information about the atom content was learned.
        m_expandable = true;
    }
    else {
        QByteArray children = m_data.right(m_data.size() - offset);
        m_data.truncate(offset);
        qDeleteAll(m_children);
        m_children = extractChildren(children);
        m_expandable = false;
    }
}



bool Metaman::Atom::expand()
{
    qDebug() << "Atom::expand() " << m_name;

    // Size and name are already stored in corresponding member variables,
    // so they can be dropped from the raw data block.
    m_data.remove(0, ATOM_SIZE_PREAMBLE_LENGTH + ATOM_NAME_PREAMBLE_LENGTH);

    if (m_expandable) {
        if (m_type == ATOM_TYPE_DATA) {
            analyzeDataAtom();
        }
        else if (m_type == ATOM_TYPE_CONTAINER) {
            analyzeContainerAtom();
        }
        else if (m_type == ATOM_TYPE_HYBRID) {
            analyzeHybridAtom();
        }
        else if (m_type == ATOM_TYPE_PSEUDO) {
            // Pseudo atom (in practise the fake atom that is used to store all
            // the real top level atoms) needs no actions. It can be directly
            // marked as already expanded.
            m_expandable = false;
        }
        else {
            // Not enough information to analyze the data
            qDebug() << "unknown atom type: " << m_type;
            m_expandable = true;
        }
    }

    qDebug() << "data of " << m_name << " was analyzed? " << !m_expandable;
    return m_expandable;
}



void Metaman::Atom::addChild(Atom* child, int index)
{
    if (isExpandable()) {
        expand();
    }
    
    if (child != 0) {
        m_children.insert(index, child);
    }
    else {
        qDebug() << "will not add null child";
    }
}



void Metaman::Atom::appendChild(Atom* child)
{
    if (isExpandable()) {
        expand();
    }
    
    addChild(child, m_children.size());
}



void Metaman::Atom::prependChild(Atom* child)
{
    if (isExpandable()) {
        expand();
    }
    
    addChild(child, 0);
}




void Metaman::Atom::removeChild(Atom* child)
{
    qDebug() << "Atom::removeChild()";

    // No need to check if the atom is analyzed or not. The child atom pointer
    // cannot exist if the data is not analyzed (i.e child atoms are created
    // when the data is analyzed).
    
    int entriesRemoved = m_children.removeAll(child);
    qDebug() << "children removed: " << entriesRemoved;

    if (child != 0) {
        delete child;
        child = 0;
        qDebug() << "child deleted";
    }
}



bool Metaman::Atom::hasChild(Atom* child) const
{
    qDebug() << "Atom::hasChild()";

    // No need to check if the atom is analyzed or not. The child atom pointer
    // cannot exist if the data is not analyzed (i.e child atoms are created
    // when the data is analyzed).
    
    return m_children.contains(child);
}



Atom* Metaman::Atom::hasChild(const QByteArray& name)
{
    qDebug() << "Atom::hasChild() : " << name;

    Atom* foundChild = 0;

    if (isExpandable()) {
        expand();
    }

    foreach(Atom* child, m_children) {
        if (child != 0 && child->name() == name) {
            foundChild = child;
            qDebug() << "found child " << foundChild->name();
            break;
        }
    }

    return foundChild;
}



qint32 Metaman::Atom::locationInFile() const
{
    return m_locationInFile;
}



QList<Atom*> Metaman::Atom::extractChildren(const QByteArray& data)
{
    qDebug() << "extractChildren()";

    // This function assumes that the data contains only child atoms
    // and no leading or trailing extra data.

    int index = 0;
    QList<Atom*> children;

    while (index < data.size()) {
        qint32 locationInSourceFile = locationInFile() + index;
        const QByteArray childSizeBytes = data.mid(index, ATOM_SIZE_PREAMBLE_LENGTH);
        const qint32 childSize = childSizeBytes.toHex().toInt(0, 16);

        if (index + ATOM_SIZE_PREAMBLE_LENGTH >= data.length()) {
            break;
        }
        else {
            index += ATOM_SIZE_PREAMBLE_LENGTH;
        }

        const QByteArray childName = data.mid(index,
                                              ATOM_NAME_PREAMBLE_LENGTH);

        if (index + ATOM_NAME_PREAMBLE_LENGTH >= data.length()) {
            break;
        }
        else {
            index += ATOM_NAME_PREAMBLE_LENGTH;
        }

        AtomType childType         = Mpeg4AtomUtility::resolveAtomType(childName);
        AtomStorage childStorage   = ATOM_STORAGE_MEMORY;
        Atom* parent               = this;
        const QByteArray payload  = data.mid(index, childSize -
                                             ATOM_SIZE_PREAMBLE_LENGTH -
                                             ATOM_NAME_PREAMBLE_LENGTH);
	const QByteArray rawAtomData = childSizeBytes + childName + payload;

        if (index + payload.size() > data.length()) {
            break;
        }
        else {
            index += payload.size();
        }

        bool expandable = true;
        Atom* childAtom = new Atom(childName,
                                   childType,
                                   childStorage,
                                   childSize,
                                   rawAtomData,
                                   locationInSourceFile,
                                   expandable,
                                   parent);

        if (childAtom != 0) {
            children << childAtom;
        }
    }

    return children;
}



QByteArray Metaman::Atom::data()
{
    return m_data;
}