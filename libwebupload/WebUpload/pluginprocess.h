
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
 
#ifndef _WEBUPLOAD_PLUGIN_PROCESS_H_
#define _WEBUPLOAD_PLUGIN_PROCESS_H_

#include <QProcess>
#include <QList>
#include <WebUpload/processexchangedata.h>
#include <WebUpload/Account>

namespace WebUpload {
 
    /*!
       \class PluginProcess
       \brief Base class for starting plugins and communicating with those
       \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class PluginProcess : public QObject { 
    Q_OBJECT
    
    public:
        PluginProcess (QObject *parent = 0);
        virtual ~PluginProcess ();
        
        /*!
          \brief Way to check if there is active process. If it has new process
                 must not be started until there is no more active process.
          \return true if there is active process
         */
        bool isActive() const;

        /*!
          \brief This function is called to kill the currently running process. 
         */
        void stopRunningProcess ();
        
        /*!
          \brief Way to kill all followed processes 
         */
        virtual void killAll();
        
        /*!
          \brief Number of followed processes
          \return Number of followed processes
         */
        int processCount() const;

    protected:
            
        /*!
          \brief Start new process for account.
          \param account Account for which we need process
          \return <code>true</code> if the process could be started, else
                  <code>false</code>
         */
        virtual bool startProcess (const WebUpload::Account * account);
                
        /*!
          \brief Send to current process
          \param data Data send
         */
        virtual void send (const QByteArray & data);
    
        QProcess * m_currentProcess; //!< Current active process
    
        //! List of still running processes
        QList <QProcess*> m_runningProcesses;
        
        ProcessExchangeData m_pdata; //!< Procotol coder
        
    protected Q_SLOTS:
    
        /*!
          \brief Slot for QProcess::started. This *HAS* to be implemented by
                 the child classes
         */
        virtual void processStarted () = 0;

        /*!
          \brief Slot for QProcess::error.
         */
        virtual void processError (QProcess::ProcessError error);
        
        /*!
          \brief Signal for QProcess::readyReadStandardOutput. Will give input
                 data to m_pdata;
         */
        virtual void inputReadyRead ();
        
        /*!
          \brief Slot for QProcess::readyReadStandardError. Will print input
                 with qDebug.
         */
        virtual void errorReadyRead ();
        
        /*!
          \brief Slot for QProcess::finished
         */        
        virtual void processFinished (int exitCode,
            QProcess::ExitStatus exitStatus);
        
    Q_SIGNALS:
    
        /*!
          \brief Signal emitted when all processes have shutdown
         */
        void noProcesses();
        
        /*!
          \brief Signal emitted if current process stopped. This happens if
                 implementing class has not marked m_currentProcess to null and
                 it is shutdown. Usually it means a crash.
         */
        void currentProcessStopped();

    };
}

#endif
