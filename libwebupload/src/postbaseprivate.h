
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

#ifndef _WEBUPLOAD_POST_BASE_PRIVATE_H_
#define _WEBUPLOAD_POST_BASE_PRIVATE_H_

#include <QObject>
#include <WebUpload/Entry>
#include <WebUpload/Media>
#include <WebUpload/AuthBase>
#include <WebUpload/Error>

namespace WebUpload {

    class PostBase;

    /*!
      \class PostBasePrivate
      \brief Private class for WebUpload::PostBase
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class PostBasePrivate : public QObject {

        Q_OBJECT
    
    public:

        //! constructor
        PostBasePrivate(PostBase * parent);

        //! Destructor
        ~PostBasePrivate ();

        //! \brief  Reset the class back to idle state
        void reset();

        /*!
          \brief  Set the entry in this class
          \param  entry_t : The entry pointer to be saved in this class
          \return True if the entry pointer could be saved, else false
         */
        bool setEntry (WebUpload::Entry *entry_t);

        /*!
          \brief Start the authentication process
                 Emits either error, stopped or mediaAuthDone signals
         */
        void startAuthentication (AuthBase *authP = 0);

        //! \brief Stop current upload.
        void stop();

        /*!
          \brief Error has been fixed. Start media upload now.
         */
        void errorFixed (AuthBase *authP);

    Q_SIGNALS:
        
        /*!
          \brief Upload progress of entry. This is connected to the
                 corresponding signal in the parent PostBase class
          \param uploaded Float value between 0.0 (not started) and 1.0
                 (done)
         */
        void progress (float uploaded);

        
        /*!
           \brief Signal emitted when whole entry upload failed and upload
                  stopped. This is connected to the corresponding signal in the
                  parent PostBase class
           \param error Error report
         */
        void error (WebUpload::Error error);


#ifdef WARNINGS_ENABLED
        /*!
          \brief Signal emitted when there was a non-fatal problem. This is 
                 connected to the corresponding signal in the parent class.
        \param error Warning message
        */
        void warning(WebUpload::Error warning);
#endif

        /*!
          \brief Signal emitted when the upload is stopped. This is connected
                 to the corresponding signal in the parent PostBase class
         */
        void stopped ();
        
        //! \brief Upload of entry was done successfully
        void done ();

        /*!
           \brief Notification that media upload was stated. This connects to
                  the corresponding signal in the parent PostBase class
           \param media Media for which upload stated
        */
        void mediaStarted (WebUpload::Media *media);

        /*!
          \brief Emit this signal after authentication started by authMedia is
                 finished successfully. This signal is connected to the
                 uploadMedia of the PostBase class
           \param media Media for which upload needs to done
         */
        void nowUploadMedia (WebUpload::Media *media);
        
    public Q_SLOTS:
     
        /*!
          \brief Slot to receive mediaDone signal from inheriting class.
          \param destUrl: The url of where the uploaded file can be found
         */
        void mediaDoneSlot (QString destUrl);
        
        //! \brief Slot to receive mediaError signal from inheriting class.
        void mediaErrorSlot (WebUpload::Error error);

#ifdef LBWEBUPLOAD_EXPERIENTAL
#ifdef WARNINGS_ENABLED
        //! \brief Slot to receive mediaWarning signal from inheriting class.
        void mediaWarningSlot (WebUpload::Error warning);
#endif
#endif

        //! \brief Slot to recieve the reAuth signal from inheriting class.
        void reAuthSlot ();
        
        //! \brief Slot to receive mediaStopped signal from inheriting class.
        void mediaStoppedSlot ();

    public:

        WebUpload::Entry *entry; //!< Pointer to entry under process
        WebUpload::Media *media; //!< Pointer to media being handled

        enum PostBaseState {
            STATE_IDLE, //!< Not active
            STATE_AUTH_PENDING, //!< Middle of authorization step
            STATE_UPLOAD_PENDING, //!< Middle of upload step
            STATE_CANCEL_PENDING, //!< Waiting for canceled reply
            STATE_FIX_ERROR_PENDING, //!< Waiting for error fix to be done

            STATE_INVALID
        };


        PostBaseState state;
        quint64 totalSize;
        quint64 sentSize;
        float ofItemDone;

    private Q_SLOTS:

        //!< Slot for AuthBase::authResult signal
        void authResultSlot (int result);
    
        //!< Slot for AuthBase::authUnknownError signal
        void authUnknownErrorSlot (const QString &errMsg);

    private:
        PostBase * const publicObject;
        AuthBase *authPtr;
        Error transferError;
    };
}

#endif
