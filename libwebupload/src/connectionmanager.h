
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

#ifndef _WEBUPLOAD_CONNECTION_MANAGER_H_
#define _WEBUPLOAD_CONNECTION_MANAGER_H_

#include <WebUpload/export.h>
#include <QObject>

namespace WebUpload {

    class ConnectionManagerPrivate;

    /*!
     * \class ConnectionManager
     * \brief This class provides the status of the connectivity
     * \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */

    class WEBUPLOAD_EXPORT ConnectionManager : public QObject
    {
        Q_OBJECT
    public:
        /*!
           \brief constructor
           \param parent Pointer to parent QObject
           \param askConnection <code>true</code> if connection dialog should
                        be popped up when there is no connection.
        */
        ConnectionManager(QObject *parent = 0, bool askConnection = true);
        
        virtual ~ConnectionManager();

        /*!
            \brief Checks if device is connected. Connected and disconnected
                   signals are only emitted after this is called. It will try
                   to open connection only once.
            \return <code>true</code> if connected else <code>false</code>.
        */
        bool isConnected();

    Q_SIGNALS:

        /*!
            \brief session created successfully
        */
        void connected();
        
        /*!
            \brief session disconnected
        */
        void disconnected();

    private:
        Q_DISABLE_COPY(ConnectionManager)    
        ConnectionManagerPrivate * const d_ptr;
    };
}
#endif
