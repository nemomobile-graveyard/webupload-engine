
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
 
#include "WebUpload/pluginprocess.h"
#include "WebUpload/system.h"
#include <QDebug>
#include <QFile>

#define STREAM_PREFIX "PluginProcess:"
#define DBG_STREAM qDebug() << STREAM_PREFIX
#define WARN_STREAM qWarning() << STREAM_PREFIX
#define CRIT_STREAM qWarning() << STREAM_PREFIX

using namespace WebUpload;

PluginProcess::PluginProcess (QObject *parent) : QObject (parent),
    m_currentProcess (0), m_pdata (this) {

}

PluginProcess::~PluginProcess () {
    killAll ();
    m_currentProcess = 0;
}
        
bool PluginProcess::isActive() const {
    qDebug() <<  "PluginProcess::isActive()";
    
    bool isRunning = false;
    
    if (m_currentProcess != 0) {
        if (m_currentProcess->state () == QProcess::Running) {
            isRunning = true;
        }
    }
    
    qDebug() << "Is process running? " << isRunning;
    
    return isRunning;
}

void PluginProcess::stopRunningProcess () {
    if (m_currentProcess != 0) {
        m_currentProcess->kill ();
        m_currentProcess = 0;
    }
}
        
void PluginProcess::killAll() {
    for (int i = 0; i < m_runningProcesses.size(); ++i) {

        WARN_STREAM << "killing" <<  m_runningProcesses[i]->pid();

        m_runningProcesses[i]->disconnect (this);
        m_runningProcesses[i]->kill ();
    }
}
        
int PluginProcess::processCount() const {
    return m_runningProcesses.size ();
}


bool PluginProcess::startProcess (const WebUpload::Account * account) {

    WebUpload::System system;
    QString processName = system.pluginProcessPathForAccount (account);
    DBG_STREAM << "launching" << processName;

    if (processName.isEmpty () || !(QFile::exists (processName))) {
        WARN_STREAM << "invalid plugin:" << processName;
        return false;
    }

    m_currentProcess = new QProcess (this);
    connect (m_currentProcess, SIGNAL (error(QProcess::ProcessError)), this, 
        SLOT (processError(QProcess::ProcessError)));
    connect (m_currentProcess, SIGNAL (started()), this, 
        SLOT (processStarted()));
    connect (m_currentProcess, SIGNAL (readyReadStandardError()), this, 
        SLOT (errorReadyRead()));
    connect (m_currentProcess, SIGNAL (readyReadStandardOutput()), this, 
        SLOT (inputReadyRead()));
    connect (m_currentProcess, SIGNAL (finished(int,QProcess::ExitStatus)),
        this, SLOT (processFinished(int,QProcess::ExitStatus)));

    m_runningProcesses.append (m_currentProcess);
    
    DBG_STREAM << "calling start...";
    m_currentProcess->start (processName);
    DBG_STREAM << "...start called.";
    return true;
}
                
void PluginProcess::send (const QByteArray & data) {
    if (isActive()) {
        QDataStream writeStream (m_currentProcess);
        writeStream << data;
    }
}
    
void PluginProcess::processError (QProcess::ProcessError error) {
    QProcess * senderProcess = qobject_cast<QProcess *>(sender());
    if (senderProcess == 0) {
        CRIT_STREAM << __FUNCTION__ << "invalid usage" << sender();
        return;
    }

    DBG_STREAM << "Received error" << error << "(" << senderProcess->pid()
        << "). Exiting the process...";

    if (!m_runningProcesses.removeOne (senderProcess)) {
        WARN_STREAM << "Received finished from untracked process";
    }
    senderProcess->disconnect (this);
    senderProcess->deleteLater ();

    if (senderProcess == m_currentProcess) {
        m_currentProcess = 0;
        Q_EMIT (currentProcessStopped());
    } 
    
    if (m_runningProcesses.isEmpty ()) {
        Q_EMIT (noProcesses());
    }
}

void PluginProcess::inputReadyRead () {
    QProcess * senderProcess = qobject_cast<QProcess *>(sender());
    if (senderProcess == 0) {
        CRIT_STREAM << __FUNCTION__ << "invalid usage" << sender();
        return;
    }

    if (senderProcess == m_currentProcess) {
        // Process only byte stream recieved from currently active process.
        // Ignore all the rest
        QByteArray byteArray = senderProcess->readAllStandardOutput ();
        m_pdata.processByteArray (byteArray);
    }

    return;
}
        
    
void PluginProcess::errorReadyRead () {
    QProcess * senderProcess = qobject_cast<QProcess *>(sender());
    if (senderProcess == 0) {
        CRIT_STREAM << __FUNCTION__ << "invalid usage" << sender();    
        return;
    }

    QByteArray byteArray = senderProcess->readAllStandardError ();
    DBG_STREAM << "[" << senderProcess->pid() << "]" << byteArray;
}
        
        
void PluginProcess::processFinished (int exitCode, 
    QProcess::ExitStatus exitStatus) {

    Q_UNUSED (exitCode);
    Q_UNUSED (exitStatus);

    qDebug() << "Process" << sender() << "exited" << exitCode << exitStatus;
    QProcess * exitingProcess = qobject_cast<QProcess *>(sender());
    if (exitingProcess == 0) {
        CRIT_STREAM << __FUNCTION__ << "invalid usage" << sender();    
        return;
    }

    if (exitingProcess == m_currentProcess) {
        m_currentProcess = 0;
        Q_EMIT (currentProcessStopped());
    } 
    
    if (!m_runningProcesses.removeOne (exitingProcess)) {
        WARN_STREAM << "Received finished from untracked process";
    }

    exitingProcess->disconnect (this);
    exitingProcess->deleteLater ();

    if (m_runningProcesses.isEmpty ()) {
        Q_EMIT (noProcesses());
    }
}

