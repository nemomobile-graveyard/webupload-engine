 
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

#ifndef _WEB_UPLOAD_CONNECTION_MANAGER_PRIVATE_H_
#define _WEB_UPLOAD_CONNECTION_MANAGER_PRIVATE_H_

#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

namespace WebUpload {

    class ConnectionManagerPrivate : public QObject
    {
        Q_OBJECT
    public:
        ConnectionManagerPrivate(QObject *parent = 0, 
            bool askConnection = true);
        
        virtual ~ConnectionManagerPrivate();

        /*!
            \brief is device connected
            \return bool true if connected else false
        */
        bool isConnected();

    Q_SIGNALS:
        /*!
            session created
        */
        void connected();
        
        /*!
            session disconnected
        */
        void disconnected();

        /*!
          Ask for network session
         */
        void askNetworkSession ();

    private Q_SLOTS:
        /*!
            \brief slot to recieve state changed signal
            \online state of the connectivity
        */
        void onlineStateChanged(bool online);

        /*!
          \brief Slot to connect to the askNetworkSession signal
         */
        void createNetworkSession ();

    private:
        //! is connection ready to start upload
        bool m_isConnectionReady;
        
        //! Ask connection if it is not there. This is used only the first time
        // the isConnected function is called.
        bool m_askConnection;
        
        //! Network configuration manager
        QNetworkConfigurationManager m_manager;
    };
}
#endif 