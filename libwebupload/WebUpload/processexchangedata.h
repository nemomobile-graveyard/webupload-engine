 
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

#ifndef _WEBUPLOAD_PROCESS_EXCHANGE_DATA_H_
#define _WEBUPLOAD_PROCESS_EXCHANGE_DATA_H_

#include <WebUpload/Error>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QByteArray>

namespace WebUpload {

    class ProcessExchangeDataPrivate;

    /*!
       \class  ProcessExchangeData
       \brief  This class facilitates the exchange of information between the
               webupload-engine/share-ui and the upload/update process. 

       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class ProcessExchangeData : public QObject 
    {
        Q_OBJECT 

    public:
        /*!
           \brief  Constructor
           \param  parent Pointer to the parent QObject instance
         */
        ProcessExchangeData (QObject * parent = 0);

        //! \brief  Destructor 
        virtual ~ProcessExchangeData ();

        /*! 
          \brief Clear any existing bytes that have not yet been processed and
                 converted to a signal
         */
        void clear ();

        //------- FUNCTIONS CALLED FROM WEBUPLOAD-ENGINE --------------------
        /*!
          \brief Function called by the webupload-engine when it wants the
                 upload process to fix an existing error in the upload and then
                 continue with the upload
          \param entryXmlPath Path of the entry xml file corresponding to the
                    upload request
          \param error Existing error in the upload
          \return QByteArray corresponding to the startUpload request
         */
        static QByteArray startUpload (const QString & entryXmlPath, 
            const WebUpload::Error & error);

        //--------FUNCTIONS CALLED FROM SHARE-UI ----------------------------
        /*!
          \brief Function called to update all updateable options
          \param accountStringId String representation of the account
          \return QByteArray corresponding to the update all request
         */
        static QByteArray updateAll (const QString & accountStringId);

        /*!
          \brief Function called to update a given option
          \param accountStringId String representation of the account
          \param optionId Id of the option to be updated
          \return QByteArray corresponding to the update request
         */
        static QByteArray update (const QString & accountStringId, 
            const QString & optionId);

        /*!
          \brief Function called to add a new value to a given option
          \param accountStringId String representation of the account
          \param optionId Id of the option to be updated
          \param valueName New value to be added
          \return QByteArray corresponding to the addValue request
         */
        static QByteArray addValue (const QString & accountStringId,
            const QString & optionId, const QString & valueName);
        
        //--------FUNCTIONS CALLED FROM SHARE-UI/WEBUPLOAD-ENGINE -----------
        /*!
          \brief Function called by the calling process when it wants the
                 ongoing request to be stopped. (Same as cancel when called for
                 update or add option requests)
          \return QByteArray corresponding to the stop request
         */
        static QByteArray stop ();

        //------- FUNCTIONS CALLED FROM HANDLER PROCESS ---------------------

        /*!
          \brief Function called by the handler process giving the index of the
                 media file being sent currently. This is emitted only when the
                 initial request was for upload, or fix error
          \param index Index of the media file being sent currently
          \return QByteArray corresponding to the progress request
         */
        static QByteArray sendingMedia (quint32 index);

        /*!
          \brief Function called by the upload process giving the current
                 progress
          \param pAmt Progress of the upload
          \return QByteArray corresponding to the progress request
         */
        static QByteArray progress (float pAmt);

        /*!
          \brief Function called by the process when the request has
                 successfully completed. The request could be for upload, fix
                 error, update or add new value. 
          \return QByteArray corresponding to the done request
         */
        static QByteArray done ();

        /*!
          \brief Function called by the upload process when the upload has been
                 stopped as per the request of the webupload-engine
          \return QByteArray corresponding to the stopped request
         */
        static QByteArray stopped ();

        /*!
          \brief Function called by the upload process when there is an error
                 in the upload. Typically, the upload process should send this
                 only when it is done with the upload request, and should not
                 continue with any uploads after this
          \param error Error with which upload failed
          \return QByteArray corresponding to the failed request
         */
        static QByteArray uploadFailed (const WebUpload::Error & error);

        #ifdef WARNINGS_ENABLED
        /*!
          \brief Function called by the upload process when there is a warning
                in the upload.
          \param warning Warning message
        */
        static QByteArray uploadWarning(const WebUpload::Error& warning);
        #endif

        /*!
          \brief Function called by the update process when update/add request
                 has failed
          \param errorId Enumeration of the error - normally one of
                    CODE_CONNECT_FAILURE, CODE_AUTH_FAILED, CODE_INV_DATE_TIME.
                    Other error codes do not make sense here
          \param failedIds List of options for which update failed. This string
                    list is empty in case the update was called for a single
                    option or if it was an add request that failed
          \return QByteArray corresponding to the failed request
         */
        static QByteArray updateFailed (const WebUpload::Error::Code & errorId,
            const QStringList & failedIds);

        /*!
          \brief Function called by the upload process when there is some 
                 option whose value needs to be modified. 
                 Typical use case is where the user opts to upload to an album
                 that has been deleted. Error might be shown to the user, and
                 option given to upload to some default album. Since plugin
                 process is not allowed to change the xml file, this signal can
                 be emitted so that the upload engine can make the appropriate
                 change to the xml file.
                 NOTE: The plugin process should handle maintaining the changed
                 value in the current instance - this signal will have no
                 effect on the option values in the current plugin instance.
          \param optionName Name of the option to be modified
          \param optionValue QVariant containing value that needs to be changed
          \param mediaIndex -1 if the option to be changed is an option in
                    entry, otherwise index of the media for which the option is
                    to be changed.
         */
        static QByteArray optionValueChanged (const QString & optionName,
            const QVariant & optionValue, int mediaIndex);

        //------- FUNCTION CALLED BY BOTH -----------------------------------
        /*!
          \brief Function provided for future use. If any additional
                 communication is required between the two processes (besides
                 the one mentioned here), then this function is called. It is
                 upto the calling process to have already converted the
                 exchange information into a byte array. This function will
                 only encapsulate that byte array in a form that can be handled
                 by the other end too, without messing any of the other
                 communications.
          \param inputStream Already converted byte array with the information
                    to be encoded
          \return QByteArray corresponding to the custom request
         */
        static QByteArray customRequest (const QByteArray & inputStream);

        /*!
          \brief Function called by the both the webupload-engine and the
                 upload process to translate a recieved byte array into the
                 appropriate request. This function will read the given byte
                 array and emit the appropriate signals corresponding to the
                 read information.
                 Processed values are removed from the byte array. If the byte
                 array does is incomplete (i.e it cannot be translated to a
                 request), then the contents are copied to a local buffer. The
                 byte array recieved in the next call to this function are
                 appended to the local buffer and then parsed
          \param recvdInfo The byte array that needs to be converted to the
                    requests
         */
        void processByteArray (const QByteArray & recvdInfo);

    Q_SIGNALS:

        /*!
          \brief Signal emitted when the byte array recieved corresponds to the
                 startUpload request
          \param entryXmlPath Path of the entry xml file corresponding to the
                    upload request
          \param error Existing error in the upload
         */
        void startUploadSignal (QString entryXmlPath, WebUpload::Error error);

        /*!
          \brief Signal emitted when the byte array recieved corresponds to the
                 updateAll request
          \param accountStringId String representation of the account
         */
        void updateAllSignal (QString accountStringId);

        /*!
          \brief Signal emitted when the byte array recieved corresponds to the
                 update request
          \param accountStringId String representation of the account
          \param optionId Id of the option to be updated
         */
        void updateSignal (QString accountStringId, QString optionId);

        /*!
          \brief Signal emitted when the byte array recieved corresponds to the
                 addValue request
          \param accountStringId String representation of the account
          \param optionId Id of the option to be updated
          \param valueName New value to be added
         */
        void addValueSignal (QString accountStringId, QString optionId, 
            QString valueName);

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 stop request
         */
        void stopSignal ();

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 sendMedia request
          \param index Index of the media file being sent currently
         */
        void sendingMediaSignal (quint32 index);

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 progress request
          \param pAmt Progress of the upload
         */
        void progressSignal (float pAmt);

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 done request
         */
        void doneSignal ();

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 stopped request
         */
        void stoppedSignal ();

        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 uploadFailed request
          \param error Error with which upload failed
         */
        void uploadFailedSignal (WebUpload::Error error);

        #ifdef WARNINGS_ENABLED
        /*! 
          \brief Signal emitted when the byte array recieved corresponds to the
                 uploadWarning request
          \param warning Warning
         */
        void uploadWarningSignal (WebUpload::Error warning);
        #endif

        /*!
          \brief Signal emitted when the byte array recieved corresponds to the
                 updateFailed request
          \param errorId Enumeration of the error - normally one of
                    CODE_CONNECT_FAILURE, CODE_AUTH_FAILED, CODE_INV_DATE_TIME.
                    Other error codes do not make sense here
          \param failedIds List of options for which update failed. This string
                    list is empty in case the update was called for a single
                    option or if it was an add request that failed
         */
        void updateFailedSignal (WebUpload::Error::Code errorId,
            QStringList failedIds);

        /*!
          \brief Signal emitted when the byte stream recieved corresponds to
                 the optionValueChanged request.
          \param optionName Name of the option to be modified
          \param optionValue QVariant containing value that needs to be changed
          \param mediaIndex -1 if the option to be changed is an option in
                    entry, otherwise index of the media for which the option is
                    to be changed.
         */
        void optionValueChangedSignal (QString optionName, QVariant
            optionValue, int mediaIndex);

        /*!
          \brief Signal emitted when the byte steam recieved is not one of the
                 known requests. (Supported for any possible future
                 enhancements)
          \param inputStream Byte array with the information 
         */
        void customRequestSignal (const QByteArray inputStream);

    private:

        ProcessExchangeDataPrivate *d_ptr;
    
        Q_DISABLE_COPY(ProcessExchangeData)

        friend class ProcessExchangeDataPrivate;

    };
}

#endif
