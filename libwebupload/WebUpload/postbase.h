 
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

#ifndef _WEBUPLOAD_POST_BASE_H_
#define _WEBUPLOAD_POST_BASE_H_

#include <WebUpload/export.h>
#include <WebUpload/PostInterface>
#include <WebUpload/Entry>
#include <WebUpload/Media>

namespace WebUpload {

    class PostBasePrivate;
    class AuthBase;

    /*!
       \class  PostBase
       \brief  This class provides the base class to be used by the Upload
               plugins to post data to various services.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT PostBase : public PostInterface {
        Q_OBJECT

    public:

        /*!
          \brief Constructor
          \param parent QObject parent
         */    
        PostBase(QObject *parent = 0);
        
        virtual ~PostBase();
                    
        /*!
          \brief See WebUploadPostInterface::upload. This implementation will
                 call authMedia and uploadMedia functions and listen internal
                 signals how to continue.
          \param entry Entry to be uploaded
          \param error Error that might have happened with previous upload
                 attempt for this entry. This pointer can be 0 if there was no
                 known previous error, or if upload is being attempted for the
                 first time.
         */
        virtual void upload (WebUpload::Entry * entry, WebUpload::Error error);

        /*!
          \brief Get the count of files/media not sent yet.
          \return Count of files/media not sent yet
         */
        unsigned int unsentCount () const;
                
    public Q_SLOTS:

        /*! \reimp */
        virtual void stop ();
        /*! \reimp_end */

    Q_SIGNALS:
        
        /**** 
         * SIGNALS EMITTED FROM THE INHERITING CLASS (DEFINED IN PLUGIN) AND
         * PROCESSED HERE. THESE SIGNALS SHOULD NOT BE CONNECTED TO FROM
         * OUTSIDE THIS CLASS 
         ****/
         
        /*!
          \brief Emit this signal if the error was successfully fixed and the
                 upload can start
         */
        void errorFixed ();

        /*!
          \brief Emit this signal if the error could not be fixed. Send the new
                 error 
          \param error New error that happened while trying to fix the previous
                       error
         */
        void errorFixFailed (WebUpload::Error error);         
         
        /*!
          \brief Emit progress of media uploaded currently. Will be converted
                 to progress of total entry. Send by inheriting class. This
                 signal is to enable communication between the inheriting class
                 and this class, and should not be used outside this class
          \param uploaded Float value between 0.0 (not started) and 1.0
                 (done)
         */        
        void mediaProgress (float uploaded);        
        
        /*!
          \brief Notification that media uploaded is done successfully. Send by
                 inheriting class. This signal is to enable communication
                 between the inheriting class and this class, and should not be
                 used outside this class
          \param destUrl: The url of where the uploaded file can be found
         */
        void mediaDone (QString destUrl);
        
        /*!
           \brief Notification that media upload failed. Send by inheriting
                  class.This signal is to enable communication between the
                  inheriting class and this class, and should not be used
                  outside this class
           \param error Error report
         */        
        void mediaError (WebUpload::Error error);

        #ifdef LIBWEBUPLOAD_EXPERIENTAL
        #ifdef WARNINGS_ENABLED
        /*!
          \brief Notification that there was a non-fatal problem when trying to
                 upload media. THIS IS FUNCTION IS NOT YET OFFICIALLY SUPPORTED!
          \param error Warning message
        */
        void mediaWarning (WebUpload::Error error);
        #endif
        #endif
        
        /*!
          \brief Notification that media upload was stopped. Send by inheriting
                 class.This signal is to enable communication between the
                 inheriting class and this class, and should not be used
                 outside this class
         */
        void mediaStopped ();      

        /*!
          \brief Notification that some problem with the authentication caused
                 the upload to fail. The plugin emits this signal when it is
                 possible to continue with the upload with a fresh auth-upload
                 cycle. The plugin should store any information it requires to
                 know that the next auth request is actually a retry. This
                 signal is to enable communication between the inheriting class
                 and this class, and should not be used outside this class
         */
        void reAuth ();
        
    protected:

        /*!
          \brief Get the pointer to the class that will handle authentication.
                 The inheriting class owns this pointer, so it should ensure
                 deletion of this class to avoid leaks
          \return Pointer to AuthBase class, to do the authentication
         */
        virtual AuthBase * getAuthPtr () = 0;

        /*!
          \brief Function called to fix any errors that might have happened
                 previously with the upload. This should be implemented by the
                 plugin
          \param entry Entry whose previous upload had the error
          \param error Error that might have happened with previous upload
                 attempt for this entry. This pointer can be 0 if there was no
                 known previous error, or if upload is being attempted for the
                 first time.
         */
        virtual void fixError (WebUpload::Entry * entry,
            WebUpload::Error error); 

        /*!
          \brief Checks the error to see if only retry needs to be called.
          \param error Information about the error
          \return <code>true</code> if the only error fixing required is to
                  call retry and <code>false</code> if the error fixing should
                  be done by the plugin
         */
        virtual bool canRetry (WebUpload::Error error);

        /*!
          \brief Will try to stop upload by calling abort to reply received.
         */
        virtual void stopMediaUpload() = 0;

     protected Q_SLOTS:

        /*!
          \brief Slot to receive mediaProgress signal from inheriting class.
         */
        virtual void mediaProgressSlot (float uploaded);

        /*!
          \brief Process upload of media. Response with mediaDone, mediaError
                 or mediaStopped signal. Also remember to emit mediaProgress
                 signals.
          \param media Media to be uploaded
         */
        virtual void uploadMedia (WebUpload::Media * media) = 0;
        
    private Q_SLOTS:
        /*!
          \brief Error has been fixed. Start media upload now.
         */
        void errorFixedSlot ();

    private:

        Q_DISABLE_COPY(PostBase)
        PostBasePrivate * const d_ptr; //!< Private data

    };
}

#endif // _WEB_UPLOAD_POST_BASE_H_