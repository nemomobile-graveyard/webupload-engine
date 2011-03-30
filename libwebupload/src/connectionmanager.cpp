 
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

// -- private class ------------------------------------------------------------

ConnectionManagerPrivate::ConnectionManagerPrivate(QObject *parent, 
    bool askConnection) : QObject(parent), m_askConnection(askConnection) {

    connect(&m_manager, SIGNAL(onlineStateChanged(bool)), this ,
        SLOT(onlineStateChanged(bool)));

    connect(&m_manager, SIGNAL(configurationChanged(const QNetworkConfiguration&)),this ,
        SLOT(configurationChanged(const QNetworkConfiguration&)));

    m_isConnectionReady = m_manager.isOnline ();
    connect (this, SIGNAL (askNetworkSession()), this, 
        SLOT (createNetworkSession()), Qt::QueuedConnection);
}

ConnectionManagerPrivate::~ConnectionManagerPrivate() {
    m_manager.disconnect (this);
    this->disconnect (this);
}

bool ConnectionManagerPrivate::isConnected() {
    if (m_askConnection && (m_isConnectionReady == false)) {
        Q_EMIT (askNetworkSession());
    } 

    // m_askConnection = false;
    return m_isConnectionReady;
}

void ConnectionManagerPrivate::createNetworkSession() {

    qDebug() << "ConnectionManagerPrivate::createNetworkSession start";
    const bool canStartIAP = (m_manager.capabilities() &
        QNetworkConfigurationManager::CanStartAndStopInterfaces);
    qDebug() << "\tcanStartIAP" << canStartIAP;

    QNetworkConfiguration cfg = m_manager.defaultConfiguration();
    qDebug() << "\tcfg.isValid" << cfg.isValid ();
    qDebug() << "\tcfg.state" << cfg.state ();
    if((cfg.isValid() == true) && (canStartIAP || cfg.state() ==
        QNetworkConfiguration::Active)) {

        qDebug() << "\t\tCalling QNetworkSession::open";
        QNetworkSession  *session = new QNetworkSession(cfg, this);
        connect (session, SIGNAL (opened()), session, SLOT (deleteLater()));
        connect (session, SIGNAL (error(QNetworkSession::SessionError)),
            this, SLOT (error(QNetworkSession::SessionError)),
            Qt::QueuedConnection);
        session->setSessionProperty ("ConnectInBackGround", QVariant (false));
        session->open();
    }
    qDebug() << "ConnectionManagerPrivate::createNetworkSession end";
}

void ConnectionManagerPrivate::error (QNetworkSession::SessionError error) {
    Q_UNUSED (error)
    QNetworkSession *sentBy = qobject_cast<QNetworkSession *>(sender ());
    if (sentBy == 0) {
        qCritical () << "ConnectionManagerPrivate::error -> invalid usage";
        return;
    }

    qDebug() << "QNetworkSession error -> " << sentBy->errorString ();
    sentBy->deleteLater ();
}

void ConnectionManagerPrivate::onlineStateChanged(bool online) {
    qDebug() << __FUNCTION__ << online << m_isConnectionReady;
    if (m_isConnectionReady == online) 
        return;

    m_isConnectionReady = online;
    if(online == true) {    
        Q_EMIT(connected());
    } else {
        Q_EMIT(disconnected());
    }
}

void ConnectionManagerPrivate::configurationChanged ( const QNetworkConfiguration & config ) {
    qDebug() << __FUNCTION__ << "Some config changed .... " << 
        "check the default config status";
    QNetworkConfiguration cfg = m_manager.defaultConfiguration();

    QNetworkConfiguration::StateFlags defaultConfigStateFlags = cfg.state();
    QNetworkConfiguration::StateFlags newConfigStateFlags = config.state();

    //default configuration active state
    bool defaultConfigActive  = 
        defaultConfigStateFlags.testFlag(QNetworkConfiguration::Active);

    //new configuration active state
    bool newConfigActive = 
        newConfigStateFlags.testFlag(QNetworkConfiguration::Active);

    //default configuration discovered state. If state set 
    bool defaultConfigDiscovered = 
        defaultConfigStateFlags.testFlag(QNetworkConfiguration::Discovered);

    //Default configuration has changed its state to inactive, in this case,
    //there will be no network available, so disconnect.
    if((newConfigActive == true) && (defaultConfigActive == false)) {
        onlineStateChanged(false);
    } else {
        //If default config is discovered, try to connect.
        if (defaultConfigDiscovered == true) {
            onlineStateChanged(true);
        }
    }
}

