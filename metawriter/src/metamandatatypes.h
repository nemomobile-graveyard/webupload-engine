
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



 
#ifndef METAMAN_DATATYPES_H
#define METAMAN_DATATYPES_H

#include <QByteArray>

namespace Metaman {

    enum AtomType {
        ATOM_TYPE_UNKNOWN,
        ATOM_TYPE_DATA,
        ATOM_TYPE_CONTAINER,
        ATOM_TYPE_HYBRID,
        ATOM_TYPE_PSEUDO // for root level "fake" atom
    };

    enum AtomStorage {
        ATOM_STORAGE_MEMORY,
        ATOM_STORAGE_FILE
    };

    enum OperationResult {
        OPERATION_OK,
        OPERATION_GENERAL_ERROR
    };

    enum OperationMode {
        OPERATION_MODE_NONE      = 0x00,
        OPERATION_MODE_STANDARD  = 0x01,
        OPERATION_MODE_ITUNES    = 0x02,
        OPERATION_MODE_XMP       = 0x04,
        OPERATION_MODE_ALL       = OPERATION_MODE_STANDARD |
                                   OPERATION_MODE_ITUNES |
                                   OPERATION_MODE_XMP,
        // for compatibility
        OPERATION_MODE_DUAL      = OPERATION_MODE_ALL
    };

    struct AtomProperty {
        QByteArray  name;
        AtomType    type;
        AtomStorage storage;
    };
}

#endif
