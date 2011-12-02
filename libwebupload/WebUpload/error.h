 
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

#ifndef _WEBUPLOAD_ERROR_H_ 
#define _WEBUPLOAD_ERROR_H_

#include <WebUpload/export.h>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QByteArray>

namespace WebUpload {

    class ErrorPrivate;

    /*!
       \class Error
       \brief Class containing error information of webupload processes
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT Error {
    
    Q_ENUMS(Error)
    
    public:

        /*!
          \brief Use static functions to make objects
                 This constructor is public because that is required to use
                 this type as a Qt MetaType. Ideally, this constructor should
                 never be called.
         */
        Error ();

        virtual ~Error();

        /*!
          \brief Constructor which reads the information in the byte stream
                 and creates the error structure. If the byte stream is an
                 invalid error stream, then the error code will be CODE_CUSTOM
                 with the byte stream assigned to the description
          \param byteStream Byte stream containing error information
         */
        Error (QByteArray & byteStream);

        /*!
          \brief Copy constructor
         */
        Error (const Error & src);

        /*!
          \brief Assignment operator
         */
        Error &operator= (const Error &other);

        /*!
          \brief Merge the new error into the existing error.
                 When merging, the following things happen:\n
                 (a) The failed count becomes sum of the failed count of the
                     two errors\n
                 (b) If the upload process cannot continue with the new error,
                     then that state is also updated\n
                 (c) If the new error is repairable, but the existing one was
                     not, then the existing error details (code, title,
                     description) are over-written with the details from the
                     new error
          \param other Error being merged into existing error
         */
        void merge (const Error & other);

        /*!
          \brief Clear all error information, and reset code back to
                 CODE_NO_ERROR
         */
        void clearError ();

        /*!
          \brief Set count of files impacted by the error. This function is
                 used only if the error effects more than one file
          \param count: Count of files not uploaded
         */
        void setFailedCount (unsigned int count);
        
        /*!
          \brief Set the total number of files in the transfer. This is
                 required to decide the strings to be present in the error
                 description. Some errors require to show count of failed files
                 when it is a multi-file transfer, and not show that otherwise.
                 THIS SHOULD BE SET ONLY ONCE FOR AN ERROR INSTANCE, and it
                 should have been set before checking the error title or error
                 description.
          \param count: Count of total number of files in transfer
         */
        void setTransferFileCount (unsigned int count);

        // --- static functions ------------------------------------------------

        /*!
          \brief Error when there is no network connection
          \return Error generated
         */
        static Error noConnection();

        /*!
          \brief Error for connection failure
          \return Error generated          
         */
        static Error connectFailure();

        /*!
          \brief Error shown when service connection times out
          \return Error generated
         */
        static Error serviceTimeOut();
        
        /*!
          \brief Error for authorization failure
          \return Error generated
         */
        static Error authorizationFailed();
        
        /*!
          \brief Error when service upload limit is exceeeded. The function
                 setFailedCount 
          \return Error generated
         */
        static Error uploadLimitExceeded();
        
        /*!
          \brief Error when file is of type that service does not accept
          \return Error generated
         */
        static Error invalidFileType();
        
        /*!
          \brief Error when the size of the file is more than what the service
                 accepts
          \return Error generated
         */
        static Error fileTooBig();
        
        /*!
          \brief Service has a problem with the file - either it is of a type
                 that is not accepted by the service, or it is of a size
                 greater than that accepted by the service. This error is used
                 only when the service does not give an error that can
                 differentiate between these two cases.
          \return Error generated
         */
        static Error fileError();

        /*!
          \brief The date/time of the device is not set correctly. As a result,
                 connection requests are failing
          \return Error generated
         */
        static Error dateTimeError();
        
        /*!
          \brief Service returned an error which is not handled by any of the
                 specific error codes
          \param errString Error string as provided by the service
          \return Error generated
         */
        static Error serviceError(const QString & errString);

        /*!
          \brief Error for missing target (album). Normally, the setFailedCount
                 function should be called after this, to set the count of the
                 unsent files
          \return Error generated                       
         */
        static Error targetDoesNotExist();
        
        /*!
          \brief Error when the user removes the account being used for sharing
                 from the device
          \return Error generated
         */
        static Error accountRemoved();
        
        /*!
          \brief Error when the user disables the account being used for
                 sharing from the device
          \return Error generated
         */
        static Error accountDisabled();
        
        /*!
          \brief Transfer failed due to reasons not explicitly handled
          \return Error generated
         */
        static Error transferFailed();

        /*!
          \brief Transfer failed since some of the files to be shared have been
                 deleted
          \return Error generated
         */
        static Error missingFiles();

        /*!
          \brief Transfer failed since the device does not have sufficient 
                 memory to make copy 
         \return Error generated
        */
        static Error outOfMemory ();
        
        /*!
          \brief Error for call of unimplemented functionality
          \return Error generated          
         */
        static Error unimplemented();
        
        /*!
          \brief Secure connection handshake failed.
          \return Error generated
         */
        static Error secureConnection();

        /*!
          \brief Make custom error
          \param message Sort message of error
          \param description Longer description of error
          \param retryMsg Message for retry - if this is empty, then it means
                    that retry is not possible. Default value is empty string
          \param canContinue Error is only related to current media/file
                               uploaded. And other files can be still uploaded
                               successfully.
          \return Error generated                               
         */        
        static Error custom (const QString & message,
            const QString & description, const QString & retryMsg = "",
            bool canContinue = false);

        // --- static functions end --------------------------------------------

        /*! 
          \brief This method can be used by plugins to save some
                 plugin-specific information in the error. This information
                 then can be read when attempting a retry of the upload.
          \param info The plugin-specific information to be saved
         */
        void setData (const QVariant &info);

        /*! 
          \brief Read the plugin-specific information stored in the error
          \return The information
         */
        const QVariant &data () const;

        /*!
          \brief Error codes
         */
        enum Code {
            CODE_NO_ERROR = 0, //!< Error type unknown
            CODE_NO_CONNECTION, //!< Error when there is no network connection
            //! Authorization failure (ask user to fix account)
            CODE_AUTH_FAILED, 
            CODE_CONNECT_FAILURE, //!< Couldn't connect to service
            //! Upload limit of service was exceeded
            CODE_UPLOAD_LIMIT_EXCEEDED, 
            CODE_INV_FILE_TYPE, //!< Service does not accept given file type
            CODE_FILE_SIZE_ERROR, //!< File is too big for the service
            //! Service error that does not differentiate between file type and
            //  file size errors
            CODE_FILE_SIZE_TYPE_ERROR, 
            //! Device has incorrect date/time set, causing errors in secure
            //  connection
            CODE_INV_DATE_TIME, 
            //! Transfer failed for some non-service error, not explicitly
            //  mentioned here
            CODE_TRANSFER_FAILED, 
            CODE_SERVICE_ERROR, //!< Unhandled service error
            //! Album/Set to which upload should be done no longer exists with
            //  the service
            CODE_TARGET_DOES_NOT_EXIST, 
            //! User removed the account to which the upload is happening from
            //  the device before upload could happen
            CODE_ACCOUNT_REMOVED, 
            //! User disabled the account to which the upload is happening from
            //  the device before upload could happen
            CODE_ACCOUNT_DISABLED, 
            CODE_MISSING_FILES, //!< Files to be shared no longer exist
            CODE_UNIMPLEMENTED, //!< Called functionality not implemented,
            CODE_SERVICE_TIME_OUT, //!< Service connection timed out
            CODE_OUT_OF_MEMORY, //!< Device does not have sufficient memory
            CODE_SECURE_CONNECTION_ERROR, //!< SSL connection error
            
            CODE_CUSTOM = -1 //!< Custom service specific error
        };
        
        /*!
          \brief Get code of error
          \return Error code
         */
        Code code() const;
        
        /*!
          \brief Get title of error
          \return Error title
         */
        QString title () const;
        
        /*!
          \brief Get description of error
          \return Error description
         */
        QString description () const;

        /*!
          \brief Get the service specific error in the case where the error is
                 service error
          \return Error string got from the service
         */
        QString serviceErrorString () const;
        /*!
          \brief Get the message to be shown for recovery.
          \return Recovery message
         */
        QString recoverMsg () const;
        
        /*!
          \brief Tells if error is something that can be fixed
          \return <code>true</code> if error is repairable
         */
        bool repairable() const;
        
        /*!
          \brief Tells if we can continue uploading remaining files
          \returns <code>true</code> if error is related only to media/file
                   that was tried to upload
         */
        bool canContinue() const;

        /*!
          \brief Returns count of number of files impacted by this error
          \return unsigned int which is count of number of files whose transfer
                  failed
         */
        unsigned int failedCount () const;
                
        /*!
          \brief Get the total number of files in the transfer. 
          \return Count of total number of files in transfer
         */
        unsigned int transferFileCount () const;
        
        /*!
          \brief Serializes the current contents of the class into a byte
                 stream. This byte stream can be used to construct another
                 WebUpload::Error instance using the Error (const QByteArray &
                 byteStream) constructor
          \return QByteArray containing the error information serialized
         */
        QByteArray serialize () const;

        /*!
          \brief Over-ride existing title for the given error
          \param title String to be used as title
         */
        void setTitle (const QString & title);
        
        /*!
          \brief Over-ride existing description for the given error
          \param description String to be used as description
         */
        void setDescription (const QString & description);

        /*!
          \brief Get the name of the account as stored in the error
          \return Name of the account for which this error was raised, if it
                  was set, else empty string
         */
        QString accountName () const;

        /*!
          \brief Set the name of the account on which the error is raised
          \param accountName Name of the account
         */
        void setAccountName (const QString & accountName);
        
        /*!
          \brief Returns custom error title if it has been set using setTitle
          \return String which is custom error title, if setTitle was called,
                  else empty string
                  This function is used internally when making copy of the
                  error class. Normally this function would not need to be used
                  from outside.
         */
        QString customTitle () const;

        /*!
          \brief Returns custom error description if it has been set using
                 setDescription
          \return String which is custom error description, if setDescription
                  was called, else empty string
                  This function is used internally when making copy of the
                  error class. Normally this function would not need to be used
                  from outside.
         */
        QString customDescription () const;

    private:
        
        /*!
          \brief Use static functions to make objects
         */        
        Error (Code code, const QString & message, const QString & description,
            const QString & recoverMsg = "");

        /*!
          \brief Private constructor for errors related to standard error codes
                 By default, count will be set as 1
          \param code Error code
          \param canContinue <code>true</code> if upload can continue with
                 other files, despite this error
          \param repairable <code>true</code> if this error can be fixed
         */
        Error (Code code, bool canContinue, bool repairable);
            
        ErrorPrivate * const d_ptr; //!< Private data
            
    };
}

Q_DECLARE_METATYPE(WebUpload::Error);
Q_ENUMS(WebUpload::Error::Code)

#endif
