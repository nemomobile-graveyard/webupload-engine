
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
 
#ifndef _WEBUPLOAD_PLUGIN_APPLICATION_PRIVATE_H_ 
#define _WEBUPLOAD_PLUGIN_APPLICATION_PRIVATE_H_ 
 
#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include <WebUpload/PostInterface>
#include <WebUpload/UpdateInterface>
#include <WebUpload/processexchangedata.h>

namespace WebUpload {

    class PluginInterface;
    class PluginApplication;
    class UpdateInterface;
    class Entry;
    class Media;
    class ServiceOption;

    /*!
       \class PluginApplicationPrivate
       \brief Private class for PluginApplication
       \author  Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class PluginApplicationPrivate : public QObject {
    Q_OBJECT
    
    public:
    
        PluginApplicationPrivate (PluginInterface * interface,
            PluginApplication * parent = 0);
        virtual ~PluginApplicationPrivate();
               
        /*!
          \brief Will sent bytearray to engine
          \param array Data to be sent
         */
        void send (const QByteArray & array);
        
        /*!
          \brief Shutdown plugin application
         */
        void shutdown ();        
        
        ProcessExchangeData m_coder; //!< Coder for protocol
        PluginInterface * m_interface; //!< Plugin interface class
        PostInterface * m_post; //!< Post object or null
        Entry * m_entry; //!< Entry or null
        
        UpdateInterface * m_update; //!< Update object or null
        Account * m_account; //!< Account updated or null
        ServiceOption * m_option; //!< Option updated or null
        
        QFile m_fileStdIn; //!< STDIN for reading
        QFile m_fileStdOut; //!< STDOUT for writing
        QFile m_fileStdErr; //!< STDOUT for writing
        QSocketNotifier * m_inputNotifier; //!< Notifier for readyRead signals
        
        bool m_initFailed; //!< Flag for init failure
        
    public Q_SLOTS:
                
        /*!
          \brief Stop current action
         */
        void stop ();
        
    private Q_SLOTS:
    
        /*!
          \brief Read to read from STDIN
         */
        void stdinReadyRead ();

        /*!
          \brief Start upload 
          \param pathToEntry Path to entry
          \param error Error data
         */
        void postStart (const QString & pathToEntry, WebUpload::Error error);
        
        /*!
          \brief Slot for PostInterface::error
         */
        void postError (WebUpload::Error error);

        #ifdef WARNINGS_ENABLED            
        /*!
          \brief Slot for PostInterface::warning
         */
        void postWarning (WebUpload::Error warning);
        #endif
        
        /*!
          \brief Slot for PostInterface::done and UpdateInterface::done
         */
        void commonDone ();

        /*!
          \brief Slot for PostInterface::stopped and UpdateInterface::canceled
         */
        void commonStopped ();
        
        /*!
          \brief Slot for PostInterface::progress
         */
        void postProgress (float uploaded);
        
        /*!
          \brief Slot for PostInterace::mediaStarted
         */
        void postMediaStarted (WebUpload::Media* media);
        
        /*!
          \brief Slot for PostInterface::pending
         */
        void postPending (WebUpload::PostInterface::PendingReason reason,
            QString message);

        /*!
          \brief Slot for PostInterface::optionValueChanged
         */
        void postOptionValueChanged (const QString & optionId,
            const QVariant & optionValue, int mediaIndex);
                   
        /*!
          \brief Start update all
          \param accountStringId See Account::init
         */
        void updateAll (QString accountStringId);
        
        /*!
          \brief Start update
          \param accountStringId See Account::init
         */
        void updateValue (QString accountStringId, QString optionId);
        
        /*!
          \brief Start add
          \param accountStringId See Account::init
         */
        void updateAddValue (QString accountStringId, QString optionId,
            QString valueName);

        /*!
          \brief Slot for UpdateInterface::updateError
         */
        void updateError(WebUpload::Error::Code error, QStringList failedIds);
        
    Q_SIGNALS:
        
        /*!
          \brief Signal to shutdown application
         */
        void readyToShutdown();
        
    private:
    
        /*!
          \brief Initialize m_account and m_update
          \param accountStringId See Account::init    
          \param optionIds List of option ids used in error signals      
          \return false if initialize failed and process should not continue
         */
        bool initUpdate (const QString & accountStringId,
            const QStringList & optionIds);
    
        Q_DISABLE_COPY(PluginApplicationPrivate)

    };
}

#endif
