
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
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

#ifndef _WEBUPLOAD_ENUMS_H_
#define _WEBUPLOAD_ENUMS_H_

#include <QObject>
#include <QFlag>

Q_ENUMS(WebUpload::ImageResizeOption WebUpload::VideoResizeOption)

namespace WebUpload {

    // If the top bit is set to 1, then QString cannot convert back to integer.
    // Hence, filter-all value is set as 0x7fffffff.
    enum MetadataFilter {
        METADATA_FILTER_NONE = 0x0, // !< Keep all the metadata
        METADATA_FILTER_ALL = 0x7fffffff, // !< Remove all metadata
        METADATA_FILTER_REST = 0x80,// !< Remove Rest of metadata
        METADATA_FILTER_AUTHOR_LOCATION = 0x01, // !< Remove Author and location
        METADATA_FILTER_TITLE = 0x2, // !< Remove Title
        METADATA_FILTER_DESCRIPTION = 0x4, // !< Remove Description
        METADATA_FILTER_TAGS = 0x8 // !< Remove Tags
    };

    Q_DECLARE_FLAGS(MetadataFilters, MetadataFilter)
    Q_DECLARE_OPERATORS_FOR_FLAGS(MetadataFilters)    

    enum ImageResizeOption {
        IMAGE_RESIZE_NONE, //!< Do not resize - keep original size

        IMAGE_RESIZE_LARGE, //!< Long edge is of length 1920 pixels
        IMAGE_RESIZE_MEDIUM, //!< Long edge is of length 1280 pixels
        IMAGE_RESIZE_SMALL, //!< Long edge is of length 640 pixels
        IMAGE_RESIZE_SERVICE_DEFAULT, //!< Use service default image size

        IMAGE_RESIZE_N //!< Last value, do not use
    };

    enum VideoResizeOption {
        VIDEO_RESIZE_NONE, //!< Do not resize
        VIDEO_RESIZE_VGA_QVGA, //!< Short edge is 480
        VIDEO_RESIZE_QVGA_WQVGA, //!< Short edge is 240

        VIDEO_RESIZE_N //!< Last value, do not use
    };
}

#endif
