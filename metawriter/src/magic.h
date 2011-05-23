
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



 
#ifndef METAMAN_MAGIC_H
#define METAMAN_MAGIC_H

#include "metamandatatypes.h"
#include <QByteArray>

namespace Metaman {

    const int MAX_ATOMS               = 1024;
    const int DEFAULT_DATA_CHUNK_SIZE = 4096;

    //
    // Magic values for MP4 / Quicktime / MPEG-4 Part 12 formats
    //

    const QByteArray ATOM_PATH_SEPARATOR = ".";
    
    // Brands
    const QByteArray BRAND_3GP4 = "3gp4";
    const QByteArray BRAND_MP42 = "mp42";

    // Box sizes and known locations
    const int ATOM_SIZE_PREAMBLE_LENGTH     = 4;
    const int ATOM_NAME_PREAMBLE_LENGTH     = 4;
    const int ATOM_DATA_DEFINITION_LENGTH   = 4;
    const int ATOM_NAME_FTYP_OFFSET         = 4;
    const int FILE_BRAND_OFFSET             = ATOM_NAME_FTYP_OFFSET +
                                              ATOM_NAME_PREAMBLE_LENGTH;

    // Atom classes
    const qint32 ATOM_CLASS_TEXT(0x00000001);


    // Atom names
    const QByteArray ATOM_NAME_DATA         = "data";
    const QByteArray ATOM_NAME_FTYP         = "ftyp";
    const QByteArray ATOM_NAME_TITLE        = "titl";
    const QByteArray ATOM_NAME_FREE         = "free";
    const QByteArray ATOM_NAME_GEOTAG       = "loci";
    const QByteArray ATOM_NAME_MDAT         = "mdat";
    const QByteArray ATOM_NAME_META         = "meta";
    const QByteArray ATOM_NAME_DESCRIPTION  = "dscp";
    const QByteArray ATOM_NAME_AUTH         = "auth";
    const QByteArray ATOM_NAME_USERDATA     = "udta";
    const QByteArray ATOM_NAME_MOOV         = "moov";
    const QByteArray ATOM_NAME_XMP          = "XMP_";
    const QByteArray ATOM_NAME_TRAK         = "trak";
    const QByteArray ATOM_NAME_MVHD         = "mvhd";
    const QByteArray ATOM_NAME_HDLR         = "hdlr";
    const QByteArray ATOM_NAME_ILST         = "ilst";
    const QByteArray ATOM_NAME_cART         = QByteArray("\xA9") +
                                              QByteArray("ART");
    const QByteArray ATOM_NAME_cCMT         = QByteArray("\xA9") +
                                              QByteArray("cmt");
    const QByteArray ATOM_NAME_cNAM         = QByteArray("\xA9") +
                                              QByteArray("nam");
    const QByteArray ATOM_NAME_UUID         = "uuid";


    // Properties of known atoms

    const AtomProperty atomProperties[] = {
        // Top level atoms
        {ATOM_NAME_FTYP,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_FREE,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_MOOV,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_MDAT,        ATOM_TYPE_DATA,         ATOM_STORAGE_FILE},

        // Lower level atoms
        {ATOM_NAME_DATA,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_USERDATA,    ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_TITLE,       ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_GEOTAG,      ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_META,        ATOM_TYPE_HYBRID,       ATOM_STORAGE_MEMORY},
        {ATOM_NAME_DESCRIPTION, ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_AUTH,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_XMP,         ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_TRAK,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_MVHD,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},

        {ATOM_NAME_HDLR,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY},
        {ATOM_NAME_ILST,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_cART,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_cCMT,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        {ATOM_NAME_cNAM,        ATOM_TYPE_CONTAINER,    ATOM_STORAGE_MEMORY},
        
        {ATOM_NAME_UUID,        ATOM_TYPE_DATA,         ATOM_STORAGE_MEMORY}
    };

    // Standard ISO/3GPP style tags
    const QByteArray ATOM_PATH_FILETYPE           = "ftyp";
    const QByteArray ATOM_PATH_FREE_TOPLEVEL      = "free";
    const QByteArray ATOM_PATH_MEDIADATA          = "mdat";
    const QByteArray ATOM_PATH_ALLMETADATA        = "moov"; // don't mix up with udta!
    const QByteArray ATOM_PATH_USERDATA           = "moov.udta";
    const QByteArray ATOM_PATH_AUTHOR             = "moov.udta.meta.auth";
    const QByteArray ATOM_PATH_TITLE              = "moov.udta.meta.titl";
    const QByteArray ATOM_PATH_TITLEDATA          = "moov.udta.meta.titl.data";
    const QByteArray ATOM_PATH_DESCRIPTION        = "moov.udta.meta.dscp";
    const QByteArray ATOM_PATH_DESCRIPTIONDATA    = "moov.udta.meta.dscp.data";
    const QByteArray ATOM_PATH_GEOTAG             = "moov.udta.meta.loci";
    const QByteArray ATOM_PATH_XMPDATA            = "moov.udta.XMP_";
    const QByteArray ATOM_PATH_AUTHOR_ALTERNATIVE = "moov.udta.auth";
    const QByteArray ATOM_PATH_GEOTAG_ALTERNATIVE = "moov.udta.loci";
    const QByteArray ATOM_PATH_UUID               = "uuid";

    //iTunes style tags
    const QByteArray ATOM_PATH_ITUNESDATA =
            "moov.udta.meta.ilst";
    const QByteArray ATOM_PATH_ITUNES_AUTHOR =
            QByteArray("moov.udta.meta.ilst.\xA9") + QByteArray("ART");
    const QByteArray ATOM_PATH_ITUNES_TITLE =
            "moov.udta.meta.ilst.\xA9nam";
    const QByteArray ATOM_PATH_ITUNES_TITLEDATA =
            "moov.udta.meta.ilst.\xA9nam.data";
    const QByteArray ATOM_PATH_ITUNES_DESCRIPTION =
            QByteArray("moov.udta.meta.ilst.\xA9") + QByteArray("cmt");
    const QByteArray ATOM_PATH_ITUNES_DESCRIPTIONDATA =
            QByteArray("moov.udta.meta.ilst.\xA9") + QByteArray("cmt.data");



    // XMP magic

    const QByteArray XMP_PROPERTY_TITLE            = "Title";
    const QByteArray XMP_PROPERTY_DESCRIPTION      = "Description";
    const QByteArray XMP_PROPERTY_AUTHOR           = "creator";
    const QByteArray XMP_PROPERTY_GEOTAG_COUNTRY   = "Country";
    const QByteArray XMP_PROPERTY_GEOTAG_CITY      = "City";
    const QByteArray XMP_PROPERTY_GEOTAG_DISTRICT  = "District";
    const QByteArray XMP_PROPERTY_GPS_LATITUDE     = "GPSLatitude";
    const QByteArray XMP_PROPERTY_GPS_LONGITUDE    = "GPSLongitude";
    const QByteArray XMP_PROPERTY_GPS_ALTITUDE     = "GPSAltitude";
    const QByteArray XMP_PROPERTY_GPS_ALTITUDE_REF = "GPSAltitudeRef";
    const QByteArray XMP_PROPERTY_KEYWORDS         = "subject";

    const QByteArray XMP_LANGUAGE_DEFAULT          = "x-default";
    const QByteArray XMP_LANGUAGE_NONE             = "";

}


#endif
