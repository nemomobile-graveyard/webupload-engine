
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
 
#ifndef _WEBUPLOAD_UPDATE_PROCESS_H_
#define _WEBUPLOAD_UPDATE_PROCESS_H_

#include <WebUpload/export.h>
#include <WebUpload/pluginprocess.h>
#include <WebUpload/Account>
#include <WebUpload/ServiceOption>

namespace WebUpload {

class UpdateProcessPrivate;

/*!
   \class  UpdateProcess
   \brief  This class manages the process in which options are updated and
           added.
           
           Please note, there is a primary assumption that only one update can
           be active at a time. This class definition currently does not cater
           to multiple updates happening at the same time.

   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class WEBUPLOAD_EXPORT UpdateProcess : public WebUpload::PluginProcess {
    Q_OBJECT

public:
    UpdateProcess (QObject *parent = 0);
    virtual ~UpdateProcess();  

    /*!
      \brief Checks whether the current process has been sent the cancel
             request already or not.
      \return <bool>true</code> if the current process has been sent the cancel
              request, else <code>false</code>
     */
    bool cancelCalled () const;

public Q_SLOTS:

    /*!
      \brief Starts update for specified option
      \param account Account used
      \param option Option updated
     */
    void startUpdate (WebUpload::Account * account,
        WebUpload::ServiceOption * option);

    /*!
      \brief Starts update for all options
      \param account Account used
     */
    void startUpdateAll (WebUpload::Account * account);

    /*!
      \brief Starts update for all options with forced reauthorization.
      \param account Account used
     */
    void startUpdateAllForceReAuth (WebUpload::Account * account);
    /*!
      \brief Start add value to option
      \param account Account used
      \param option Option updated
      \param value Value added
     */
    void startAddValue (WebUpload::Account * account,
        WebUpload::ServiceOption * option, const QString & value);  

    /*!
      \brief Start add value to option with forced reauthorization
      \param account Account used
      \param option Option updated
      \param value Value added
     */
    void startAddValueForceReAuth (WebUpload::Account * account,
        WebUpload::ServiceOption * option, const QString & value);

    /*!
      \brief Cancel current update/add process
     */
    void cancel ();

Q_SIGNALS:

    /*!
      \brief Signal emitted when action is done successfully
     */
    void finished ();

    /*!
      \brief Signal emitted when action has failed
     */
    void failed (WebUpload::Error::Code error, QStringList failedIds);

    /*!
      \brief Alternative signal emitted when action has failed with custom
             error string
     */
    void failed (WebUpload::Error error, QStringList failedIds);

#ifdef WARNINGS_ENABLED
    /*!
      \brief Signal emitted when action raised a warning
     */
    void warning (WebUpload::Error::Code warning, QStringList failedIds);
#endif

    /*!
      \brief Signal emitted when action is canceled
     */
    void canceled ();
        
protected Q_SLOTS:

    /*! \reimp */
    virtual void processStarted ();
    /*! \reimp_end */
        
private:

    UpdateProcessPrivate *d_ptr;


    friend class UpdateProcessPrivate;

};

}

#endif
