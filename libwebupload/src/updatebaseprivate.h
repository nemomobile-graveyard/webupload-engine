
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
 
#ifndef _WEBUPLOAD_POST_BASE_PRIVATE_H_
#define _WEBUPLOAD_POST_BASE_PRIVATE_H_ 

#include <WebUpload/Account>
#include <WebUpload/Error>
#include <QVariant>
#include <QString>
#include <QObject>

namespace WebUpload {
 
    // Forward declarations
    class UpdateBase;
    class AuthBase;
    class PostOption;

    /*!
       \class  UpdateBasePrivate
       \brief  This class provides protected functionality of the base class to
               be used by the Upload plugins to update their post options.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */ 
    class UpdateBasePrivate : public QObject {

        Q_OBJECT

    public:

        UpdateBasePrivate(UpdateBase * parent);
        
        virtual ~UpdateBasePrivate();        
        
        /*!
          \brief Set the account on which update/add is to be done
          \param account Account
         */
        void updateAll (WebUpload::Account * account);

        /*!
          \brief Function to start update for single option. 
          \param account Account where option is updated
          \param option Option to be updated
          \param singleTask <code>true</code> If this is a single update to be
                    done, <code>false</code> if this is a single update in an
                    update all. Default value is <code>true</code>
         */
        void update (WebUpload::Account * account,
            WebUpload::ServiceOption * option, bool singleTask = true); 

        /*!
          \brief Add new value for the option
          \param account Account where option is updated
          \param option Option to be updated
          \param valueName Name of value to be added
         */
        void addValue (WebUpload::Account * account,
            WebUpload::ServiceOption * option, const QString &valueName);

        //! \brief Cancel current authentication process
        void cancelAuth ();

        enum State {
            STATE_IDLE, //!< Nothing to do
            STATE_AUTH, //!< Waiting for auth response
            STATE_UPDATE, //!< Waiting for update response
            STATE_CANCEL, //!< To be cancelled when response received

            STATE_INVALID
        };

        
        //! Service parameter given when called        
        WebUpload::Account * accountWas;
       
        //! Current item in auth, update or add value 
        WebUpload::ServiceOption * currentOption;        
        
        //! If true, then current operation is update, else current
        //  operation is add new value
        bool isUpdate;
        
        State state; //!< Current state of base class

    Q_SIGNALS:
        
        /*! 
          \brief Update/Add value finished. Connects to the done of the
                 public UpdateBase class
         */
        void done ();

        /*!
          \brief Signal emitted when whole entry upload failed and upload
                 stopped. This is connected to the corresponding signal in the
                 parent PostBase class
          \param error Error code
          \param failedIds Options not updated
         */
        void error (WebUpload::Error::Code error, QStringList failedIds);

        /*!
          \brief Signal emitted when the upload is stopped. This is connected
                 to the corresponding signal in the parent PostBase class
         */
        void canceled ();
        
        /*!
          \brief Signal emitted when authentication for an update has been
                 successfully completed. This is connected to the slot
                 updateOption in the UpdateBase class
         */
        void updateAuthDone ();

        /*!
          \brief Signal emitted when authentication for an add has been
                 successfully completed. This is connected to the slot
                 addOptionValue in the UpdateBase class
          \param valueName Name of the value that has to be added
         */
        void addAuthDone (QString valueName);
        
    public Q_SLOTS:
            
        /*!
          \brief Slot for optionDone. Will continue with next option or
                 emit done signal. Or error if there was errors before.
         */
        virtual void optionDoneSlot ();
        
        /*!
          \brief Slot for optionFailed. Will check error type and either
                 emit error signal or continue with next option
         */
        virtual void optionFailedSlot (WebUpload::Error::Code errCode);
        
        /*!
          \brief Slot for optionAdded signal
         */
        virtual void optionAddedSlot (bool needsUpdate);

        //! \brief Slot to recieve the reAuth signal from inheriting class.
        void reAuthSlot ();

    private Q_SLOTS:

        //!< Slot for AuthBase::authResult signal
        void authResultSlot (int result);
    
        //!< Slot for AuthBase::authUnknownError signal
        void authUnknownErrorSlot (const QString &errMsg);

    private:

        /*!
          \brief Start the authentication process
                 Emits either error, stopped or authDone signals
         */
        void startAuthentication (AuthBase *authP);

        //! \brief Reset back to idle state
        void reset ();
        
        //! If true will not check if there is more options to update
        bool singleTask;

         //! For indexing the options
        QListIterator<WebUpload::PostOption *> optionIter;

        // The following variables are used only when adding a new value
        //! Value to be added to given option
        QString valueName;

        UpdateBase * publicObject; //!< Public parent object

        AuthBase *authPtr; //!< Authentication class pointer
    };
}

#endif //#ifndef _WEB_UPLOAD_POST_BASE_PRIVATE_H_
