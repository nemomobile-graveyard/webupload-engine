
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

#ifndef _WEBUPLOAD_POST_INTERFACE_H_
#define _WEBUPLOAD_POST_INTERFACE_H_

#include <WebUpload/export.h>
#include <WebUpload/Error>
#include <WebUpload/Entry>
#include <QObject>

namespace WebUpload {

    /*!
       \class PostInterface
       \brief Parent class for all post classes used by WebUploadEngine. Defines
              interfaces used to communicate with engine. For implementing
              WebUploadPostBase is better starting point.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT PostInterface : public QObject {
    
        Q_OBJECT

    public:

        /*!
          \brief Constructor
          \param parent QObject parent
         */    
        PostInterface (QObject *parent = 0);
        
        virtual ~PostInterface();
        
        /*!
          \brief Pending reason to be used in signal
         */
        enum PendingReason {
            PENDING_CUSTOM, //!< Use message to present reason
            PENDING_AUTHENTICATION //!< Authentication is pending
        };

        /*!
          \brief Upload whole entry to web service.
          \param entry Entry to be uploaded
          \param error Any error that might have happened in any previous
                 attempt to upload this entry
         */
        virtual void upload (WebUpload::Entry *entry,
            WebUpload::Error error) = 0;


    public Q_SLOTS:

        /*!
           \brief Stop current upload.
         */
        virtual void stop() = 0;

    Q_SIGNALS:
            
        /*!
          \brief Upload progress of entry
          \param uploaded Float value between 0.0 (not started) and 1.0
                 (done)
         */
        void progress (float uploaded);
        
        /*!
          \brief If there is no progress for longer period then this signal
                 should be emitted and descripe why transfer is blocked.
          \param reason Why transfer is pending
          \param message Pending message
         */
        void pending (WebUpload::PostInterface::PendingReason reason,
            QString message);
            
        /*!
           \brief Notification that media upload was stated. Signal emitted by
                  the class when a new media uploading started. WUE connects to
                  this signal to send media properties (Media Title, Media
                  Thumbnail) to TUI.
           \param media Media for which upload stated
        */
        void mediaStarted (WebUpload::Media *media);

        /*!
           \brief Signal emitted when whole entry upload failed and upload
                  stopped.
           \param error Error report
         */
        void error (WebUpload::Error error);

        #ifdef LIBWEBUPLOAD_EXPERIENTAL
        #ifdef WARNINGS_ENABLED
        /*!
          \brief Signal emitted when there was a non-fatal problem with the 
                 whole entry. THIS IS NOT YET OFFICIALLY SUPPORTED!
          \param warning Warning report
        */
        void warning (WebUpload::Error warning);
        #endif
        #endif

        /*!
          \brief Signal emitted when some option value has been changed
                 and the change has to be known even in the upload engine
                 Typical use case is where the user opts to upload to an album
                 that has been deleted. Error might be shown to the user, and
                 option given to upload to some default album. Since plugin
                 process is not allowed to change the xml file, this signal can
                 be emitted so that the upload engine can make the appropriate
                 change to the xml file.
                 NOTE: The plugin process should handle maintaining the changed
                 value in the current instance - this signal will have no
                 effect on the option values in the current plugin instance.
          \param optionId  Id of the changed option
          \param optionValue Changed value
          \param mediaIndex  If this is -1, then it means the option is 
                    applicable for all media (i.e. it is an entry-level
                    option), otherwise it contains the index of the media for
                    which this change is applicable.
         */
        void optionValueChanged (const QString & optionId,
            const QVariant & optionValue, int mediaIndex);

        /*!
          \brief Signal emitted when the upload is stopped.
         */
        void stopped ();
        
        /*!
          \brief Upload of entry was done successfully
         */
        void done ();

    };
}

Q_DECLARE_METATYPE(WebUpload::PostInterface::PendingReason)

#endif // #ifndef _WEB_UPLOAD_POST_INTERFACE_H_
