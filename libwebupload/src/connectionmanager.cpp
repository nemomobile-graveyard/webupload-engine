 
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

#include "connectionmanager.h"
#include "connectionmanagerprivate.h"
#include <qnetworkconfiguration.h>
#include <qnetworksession.h>

using namespace WebUpload;

ConnectionManager::ConnectionManager(QObject *parent, bool askConnection) : 
    QObject(parent), d_ptr(new ConnectionManagerPrivate(this, askConnection)) {
   
   connect(d_ptr, SIGNAL(connected()), this, SIGNAL(connected()));
   connect(d_ptr, SIGNAL(disconnected()), this, SIGNAL(disconnected()));   

}

ConnectionManager::~ConnectionManager() {
    delete d_ptr;
}

bool ConnectionManager::isConnected() {
#ifdef __i386__
    return true;
#else
    return d_ptr->isConnected();
#endif
}

void ConnectionManager::releaseConnection() {
    d_ptr->releaseConnection();
}

// -- private class ------------------------------------------------------------

ConnectionManagerPrivate::ConnectionManagerPrivate(QObject *parent, 
    bool askConnection) : QObject(parent), m_askConnection(askConnection),
    m_connectionRequired(false), m_sessionOpening(false), m_session(0)
{

    connect(&m_manager, SIGNAL(onlineStateChanged(bool)), this ,
        SLOT(onlineStateChanged(bool)));

    connect(&m_manager, SIGNAL(configurationChanged(const QNetworkConfiguration&)),this ,
        SLOT(configurationChanged(const QNetworkConfiguration&)));

    m_isConnectionReady = m_manager.isOnline ();

    qDebug() << "Connection manager created, default configuration:";
    m_defaultConfig = m_manager.defaultConfiguration();
    printNetworkConfigStatus(m_defaultConfig);

    createSession();
}



ConnectionManagerPrivate::~ConnectionManagerPrivate() {
    m_manager.disconnect (this);
    this->disconnect (this);
}

void ConnectionManagerPrivate::createSession() {

    if (m_session != 0) {
        qDebug() << "Deleting old network session";
        delete m_session;
        m_session = 0;
    }

    qDebug() << "Creating network session";
    m_session = new QNetworkSession(m_defaultConfig, this);
    m_session->setSessionProperty ("ConnectInBackGround", QVariant (false));
    connect(m_session, SIGNAL(stateChanged(QNetworkSession::State)),
        this, SLOT(sessionStateChanged(QNetworkSession::State)));
    connect(m_session, SIGNAL(opened()), this, SLOT(sessionOpened()));
    connect(m_session, SIGNAL(closed()), this, SLOT(sessionClosed()));
    connect(m_session, SIGNAL (error(QNetworkSession::SessionError)),
        this, SLOT (error(QNetworkSession::SessionError)));
}

bool ConnectionManagerPrivate::isConnected() {

    qDebug() << __FUNCTION__;

    if (!m_askConnection) {
        return m_isConnectionReady;
    }

    m_connectionRequired = true;

    if (m_session->configuration() != m_defaultConfig) {
        qDebug() << "Default config has changed";
        createSession();
    }

    if (!m_session->isOpen()) {
        qDebug() << "Opening network session";
        m_sessionOpening = true;
        m_session->open();
    }

    bool sessionOpen = m_session->isOpen();
    qDebug() << "Network session now open:" << sessionOpen;

    return sessionOpen;
}

void ConnectionManagerPrivate::releaseConnection() {

    qDebug() << __FUNCTION__;

    m_connectionRequired = false;

    if (m_session != 0) {
        if (m_session->isOpen()) {
            qDebug() << "Closing network session";
            m_session->close();
        }
        else {
            qDebug() << "Network session already closed";
        }
    }
}

void ConnectionManagerPrivate::sessionStateChanged(QNetworkSession::State state) {
    qDebug() << "Network session state changed:" << state;

    if (state == QNetworkSession::Connected && m_connectionRequired &&
        m_session != 0 && !m_session->isOpen() && !m_sessionOpening) {

        m_sessionOpening = true;
        m_session->open();
    }
}

void ConnectionManagerPrivate::sessionOpened() {
    qDebug() << "Network session opened";
    m_sessionOpening = false;
    Q_EMIT(connected());
}

void ConnectionManagerPrivate::sessionClosed() {
    qDebug() << "Network session closed";
    m_sessionOpening = false;
    Q_EMIT(disconnected());
}

void ConnectionManagerPrivate::error (QNetworkSession::SessionError error) {
    Q_UNUSED (error)
    qDebug() << "Network session error:" << m_session->errorString();
    m_sessionOpening = false;
}

void ConnectionManagerPrivate::onlineStateChanged(bool online) {
    qDebug() << "Online state changed from" << m_isConnectionReady << "to" << online;

    if (m_isConnectionReady == online) 
        return;

    m_isConnectionReady = online;
}

void ConnectionManagerPrivate::configurationChanged ( const QNetworkConfiguration & config ) {

    qDebug() << "Configuration changed, status:";
    printNetworkConfigStatus(config);

    QNetworkConfiguration defaultConfig = m_manager.defaultConfiguration();
    if (defaultConfig != m_defaultConfig) {
        qDebug() << "Default configuration changed, new default:";
        m_defaultConfig = defaultConfig;
        printNetworkConfigStatus(m_defaultConfig);

        if (m_session != 0 && !m_session->isOpen()) {
            createSession();
        }
    }

}

void ConnectionManagerPrivate::printNetworkConfigStatus(const QNetworkConfiguration &config) {

    qDebug() << "Bearer:" << config.bearerTypeName();
    qDebug() << "Roaming available:" << config.isRoamingAvailable();
    qDebug() << "Valid:" << config.isValid();
    qDebug() << "Name:" << config.name();
    qDebug() << "Purpose:" << config.purpose();
    qDebug() << "State:" << config.state();
    qDebug() << "Type:" << config.type();
}

