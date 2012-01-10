 
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

#ifndef _WEBUPLOAD_UPDATE_BASE_H_
#define _WEBUPLOAD_UPDATE_BASE_H_

#include <WebUpload/export.h>
#include <WebUpload/UpdateInterface>
#include <QObject>
#include <WebUpload/Account>
#include <WebUpload/ServiceOption>
#include <WebUpload/Error>

namespace WebUpload {

    class UpdateBasePrivate;
    class AuthBase;

    /*!
       \class  UpdateBase
       \brief  This class provides the base class to be used by the Upload
               plugins to update their post options.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT UpdateBase : public UpdateInterface {
        
        Q_OBJECT

    public:
        /*!
          \brief Constructor
          \param parent QObject parent
         */
        UpdateBase (QObject *parent = 0);
        
        /*!
          \brief Destructor
         */
        virtual ~UpdateBase();
        
        /*!
          \brief Function to start update of all updatable options. Calls
                 update function for all updatable options found. Default
                 functionality will call update function with each updatable
                 option found. Override default functionality if need to group
                 option updates or for some other advanced features. If one of
                 the updates fails then update is stopped and given error
                 emitted.
          \param account Account which updatable option values are updated
         */
        virtual void updateAll (WebUpload::Account * account);
        
        /*!
          \brief Function to start update for single option. 
          \param account Account which option is updated
          \param option Option that need updating
         */
        virtual void update (WebUpload::Account * account,
            WebUpload::ServiceOption * option);   
                
        /*!
          \brief Function to add a new value to a given option
          \param account Account which option is changed
          \param option Option that needs new value
          \param valueName New value name to be added to option
         */
        virtual void addValue (WebUpload::Account * account,
            WebUpload::ServiceOption * option, const QString &valueName);

        /*!
          \brief Get the pointer to the class that will handle authentication.
                 The inheriting class owns this pointer, so it should ensure
                 deletion of this class to avoid leaks.
                 Default implementation will return pointer to AuthBase class.
          \return Pointer to AuthBase class, to do the authentication
         */
        virtual AuthBase * getAuthPtr () = 0;

        /*!
          \brief Function to start update of all updatable options with forced
                 reauthorization. Calls update function for all updatable options
                 found. Default functionality will call update function with each
                 updatable option found. If one of the updates fails then update
                 is stopped and given error emitted.
          \param account Account which updatable option values are updated
         */
        void updateAllForceReAuth (WebUpload::Account * account);

        /*!
          \brief Function to start update for single option with forced reauthorization.
          \param account Account which option is updated
          \param option Option that need updating
         */
        void updateForceReAuth (WebUpload::Account * account,
            WebUpload::ServiceOption * option);

        /*!
          \brief Function to add a new value to a given option with forced
                 reauthorization
          \param account Account which option is changed
          \param option Option that needs new value
          \param valueName New value name to be added to option
         */
        void addValueForceReAuth (WebUpload::Account * account,
            WebUpload::ServiceOption * option, const QString & valueName);

    public Q_SLOTS:

        /*!
           \brief Slot for cancelling the current update process. Default
                  implementation will call cancel to auth or update part based
                  on current state.
         */
        virtual void cancel();
        
    Q_SIGNALS:

        /*!
          \brief Signal that has be emitted by inheriting class when update is
                 done successfully.
         */    
        void optionDone ();
        
        /*!
          \brief Signal that has to be emitted by inheriting class after
                 addOptionValue is done successfully.
          \param updateNeeded If system should update option after this call
         */
        void optionAdded (bool updateNeeded);
        
        /*!
          \brief Signal that has to be emitted by inheriting class when update
                 or addOptionValue ends with an error.
          \param updateError Error
         */        
        void optionFailed (WebUpload::Error::Code updateError);

        /*!
           \brief Alternative option fail signal to be able to specify custom
                  error strings.
           \param error Error
         */
        void optionFailed (WebUpload::Error error);

        /*!
          \brief Notification that some problem with the authentication caused
                 the update to fail. The plugin emits this signal when it is
                 possible to continue with the update with a fresh auth-update
                 cycle. The plugin should store any information it requires to
                 know that the next auth request is actually a retry. This
                 signal is to enable communication between the inheriting class
                 and this class, and should not be used outside this class
         */
        void reAuth ();

        /*!
          \brief Notification that reauthorization should be forced for next
                 operation regardless of earlier authorization outcome.
         */
        void forceReAuthorization ();
        
    protected Q_SLOTS:

        /*!
          \brief Needed update steps has to be done in implementation of this
                 function. Emit optionDone or optionFailed signal when update
                 is done. This function works on the account and option which
                 can be got with the functions account() and currentOption()
                 respectively.
         */        
        virtual void updateOption () = 0;

        /*!
          \brief Needed steps to add new value to the option has to be done in
                 the implementation of this function. Emit optionDone or
                 optionFailed signal when new value addition is complete. This
                 function works on the account and option which can be got with
                 the functions account() and currentOption() respectively.
          \param valueName Name of value to be added
         */
        virtual void addOptionValue (const QString &valueName) = 0;

        /*!
          \brief Function to force reauthorization. The inheriting class should
                 implement this if the auth plugin has some special way of forcing
                 reauthorization, and reconnect the forceReAuthorization signal.
         */
        void forceReAuth ();

    protected:

        /*!
          \brief Will return pointer to current option updated
         */
        virtual WebUpload::Account * account ();

        /*!
          \brief Will return pointer to current option updated
         */
        virtual WebUpload::ServiceOption * currentOption ();

        /*!
          \brief Check if the current request being processed is a update or
                 add option
          \return <code>true</code> if update option is being processed and
                  <code>false</code> if add option is being processed
         */
        bool isUpdate () const;

        /*!
          \brief Cancel function for update parts needs to be implemented by
                 inheriting classes
         */
        virtual void optionCancel() = 0;                  

    private:
    
        Q_DISABLE_COPY(UpdateBase)
        UpdateBasePrivate * const d_ptr; //!< Private data
    };
}

#endif
