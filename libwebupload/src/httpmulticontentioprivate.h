 
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

#ifndef _WEBUPLOAD_HTTP_MULTI_CONTENT_IO_PRIVATE_H_
#define _WEBUPLOAD_HTTP_MULTI_CONTENT_IO_PRIVATE_H_

#include <QString>
#include <QIODevice>
#include <QVector>

namespace WebUpload {

    class HttpMultiContentIOPrivate {

    public:
        HttpMultiContentIOPrivate();
        virtual ~HttpMultiContentIOPrivate();
        
        void clear();
        bool seek (qint64 pos);
        qint64 pos () const;
        qint64 readData (char *data, qint64 maxlen);

        //! Captures whether the device is currently open or not
        bool isDeviceOpen;

        QString defaultTemplate;
        QString boundaryString;

        qint64  totalSizeBytes;
        qint64  bytesSent;

        QVector<QIODevice *> dataList;
        int currentDeviceIndex;

        /*!
           \brief  Generates a random boundary string which is 64 characters
                   long. This is called from the constructor.
         */
        void generateBoundaryString();
    };
}

#endif
