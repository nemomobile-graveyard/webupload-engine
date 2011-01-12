 
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
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

#ifndef _WEBUPLOAD_PROCESS_EXCHANGE_DATA_PRIVATE_H_
#define _WEBUPLOAD_PROCESS_EXCHANGE_DATA_PRIVATE_H_

#include <QDataStream>
#include <QByteArray>

namespace WebUpload {

    class ProcessExchangeData;

    /*!
        \class ProcessExchangeDataPrivate
        \brief Private implementation class for ProcessExchangeData
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
    */
    class ProcessExchangeDataPrivate
    {
    public:
        ProcessExchangeDataPrivate(ProcessExchangeData *publicObject);

        ~ProcessExchangeDataPrivate();

        void processByteArray(const QByteArray &recvdInfo);

        /*!
          \brief Prefix the size to the recieved array as a quint32 and return
                 the resultant array
          \param inS Input array
          \return A new QByteArray which is formed by prefixing the size of the
                    input array to the input array
         */
        static QByteArray wrapSize (const QByteArray &inS);

        /*!
          \brief Parses the data stream which contains the data for a
                 startUpload request and emits the startUploadSignal with the
                 parsed data
          \param ds Datastream containing the parameters for the
                    startUploadSignal
         */
        void parseStartUploadRequest (QDataStream & ds);

        /*!
          \brief Parses the data stream which contains the data for a
                 uploadFailed request and emits the uploadFailedSignal with the
                 parsed data
          \param ds Datastream containing the parameters for the
                    uploadFailedSignal
         */
        void parseUploadFailedRequest (QDataStream & ds);

        /*!
          \brief Parses the data stream which contains the data for a
                 uploadWarning request
          \param ds Datastream containing the parameters for the
                    uploadWarningSignal
         */
        void parseUploadWarningRequest (QDataStream & ds);

        /*!
          \brief Parses the data stream which contains the data for a
                 updateFailed request and emits the updateFailedSignal with the
                 parsed data
          \param ds Datastream containing the parameters for the
                    updateFailedSignal
         */
        void parseUpdateFailedRequest (QDataStream & ds);

    private:

        enum RequestEnums {
            CODE_REQUEST_START_UPLOAD = 0,
            CODE_REQUEST_UPDATE_ALL,
            CODE_REQUEST_UPDATE,
            CODE_REQUEST_ADD_VALUE,
            CODE_REQUEST_STOP,
            CODE_REQUEST_SENDING_MEDIA,
            CODE_REQUEST_PROGRESS,
            CODE_REQUEST_DONE,
            CODE_REQUEST_STOPPED,
            CODE_REQUEST_UPLOAD_FAILED,
            CODE_REQUEST_UPDATE_FAILED,
            #ifdef WARNINGS_ENABLED
            CODE_REQUEST_UPLOAD_WARNING,
            #endif
            CODE_REQUEST_CUSTOM,

            CODE_REQUEST_MAX_REQUEST = CODE_REQUEST_CUSTOM
        };

        ProcessExchangeData *q_ptr;

        char * m_alreadyReadData;

        int m_alreadyReadAmount;

        int m_sizeReqd;

        friend class ProcessExchangeData;
    };
}

#endif