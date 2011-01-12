 
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

#ifndef _WEBUPLOAD_UPDATE_INTERFACE_H_
#define _WEBUPLOAD_UPDATE_INTERFACE_H_

#include <WebUpload/export.h>
#include <WebUpload/Error>
#include <WebUpload/Account>
#include <WebUpload/ServiceOption>
#include <QObject>
#include <QStringList>
#include <QString>

namespace WebUpload {

    /*!
       \class UpdateInterface
       \brief Parent class for all update classes used by WebUploadEngine. Defines
              interfaces used to communicate with engine. For implementing
              WebUploadUpdateBase is better starting point.
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
      */
    class WEBUPLOAD_EXPORT UpdateInterface : public QObject {
        Q_OBJECT

    public:

        /*!
          \brief Constructor
          \param parent QObject parent
         */    
        UpdateInterface (QObject *parent = 0);
        
        virtual ~UpdateInterface();
        
        /*!
          \brief Update all options related to account
          \param account Account where options are updated
         */
        virtual void updateAll (WebUpload::Account * account) = 0;
        
        /*!
          \brief Update single option
          \param account Account where option is updated
          \param option Option to be updated
         */
        virtual void update (WebUpload::Account * account,
            WebUpload::ServiceOption * option) = 0;        

        /*!
          \brief Add new value for the option
          \param account Account where option is updated
          \param option Option to be updated
          \param valueName Name of value to be added
         */
        virtual void addValue (WebUpload::Account * account,
            WebUpload::ServiceOption * option, const QString &valueName) = 0;

    public Q_SLOTS:

        /*!
           \brief Cancel current update(s).
         */
        virtual void cancel() = 0;

    Q_SIGNALS:
                    
        /*!
          \brief Signal emitted when whole update/addValue failed and so
                 update/addValue is stopped. Usually authorization failure.
          \param error Error code (if multiple different errors happen, then
                     this is "one of those"). Support only for
                     CODE_CONNECT_FAILURE, CODE_AUTH_FAILED, CODE_INV_DATE_TIME
                     - other error codes do not make sense for update
          \param failedIds Options not updated
         */
        void error (WebUpload::Error::Code error, QStringList failedIds);

        /*!
          \brief Signal emitted when the update is canceled
         */
        void canceled ();
        
        /*!
          \brief Update/Add value finished
         */
        void done ();

    };
}

#endif