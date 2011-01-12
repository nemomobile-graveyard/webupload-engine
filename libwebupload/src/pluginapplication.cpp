 
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

#include "WebUpload/System"
#include "WebUpload/PluginApplication"
#include "WebUpload/PostInterface"
#include "WebUpload/UpdateInterface"
#include "WebUpload/Entry"
#include "WebUpload/Error"
#include "pluginapplicationprivate.h"
#include <fcntl.h>
#include <QDebug>

using namespace WebUpload;

PluginApplication::PluginApplication (PluginInterface * interface, int & argc,
    char ** argv) : QCoreApplication (argc, argv),
    d_ptr (new PluginApplicationPrivate (interface, this)) {
            
    connect (d_ptr, SIGNAL (readyToShutdown()), this, SLOT (quit()),
        Qt::QueuedConnection);
}

PluginApplication::~PluginApplication () {
    d_ptr->disconnect (this);
    delete d_ptr;
}

// - private -------------------------------------------------------------------

PluginApplicationPrivate::PluginApplicationPrivate (PluginInterface * interface,
    PluginApplication * parent) : QObject (parent), m_interface (interface),
    m_post (0), m_update (0), m_account(0), m_option(0), m_inputNotifier (0),
    m_initFailed (false) {
    
    // This will register needed meta types (have to be done in private)
    WebUpload::System::registerMetaTypes ();

    WebUpload::System::loadLocales ();
        
    // STDIN hacks
    int stdinStatusFlags = fcntl(fileno(stdin), F_GETFL, 0);
    fcntl(fileno(stdin), F_SETFL, stdinStatusFlags | O_NONBLOCK);    
    
    // Open IO files
    m_fileStdIn.open (stdin, QIODevice::ReadOnly);
    m_fileStdOut.open (stdout, QIODevice::WriteOnly);
    m_fileStdErr.open (stderr, QIODevice::WriteOnly);        
    
    // Initialize plugin
    if (m_interface == 0) {
        m_initFailed = true;
    } else {
        m_initFailed = (m_interface->init() == false);
    }
       
    // Incoming messages   
    connect (&m_coder, SIGNAL (stopSignal()), this, SLOT (stop()),
        Qt::QueuedConnection);
    connect (&m_coder, SIGNAL (startUploadSignal(QString,WebUpload::Error)),
        this, SLOT (postStart(QString,WebUpload::Error)), Qt::QueuedConnection);
    connect (&m_coder, SIGNAL (updateAllSignal(QString)), this,
        SLOT (updateAll(QString)), Qt::QueuedConnection);
    connect (&m_coder, SIGNAL (updateSignal(QString,QString)), this,
        SLOT (updateValue(QString,QString)), Qt::QueuedConnection);
    connect (&m_coder, SIGNAL (addValueSignal(QString,QString,QString)), this,
        SLOT (updateAddValue(QString,QString,QString)), Qt::QueuedConnection);
    
    // Setup notifier
    m_inputNotifier = new QSocketNotifier (STDIN_FILENO, QSocketNotifier::Read);
    connect (m_inputNotifier, SIGNAL (activated(int)), this,
        SLOT (stdinReadyRead()));
    m_inputNotifier->setEnabled (true);
}

PluginApplicationPrivate::~PluginApplicationPrivate () {
    delete m_inputNotifier;
    m_inputNotifier = 0;

    m_fileStdIn.close ();
    m_fileStdOut.close ();
    m_fileStdErr.close ();

    if (m_account != 0) {
        delete m_account;
    }
}

void PluginApplicationPrivate::stdinReadyRead () {
    m_inputNotifier->setEnabled (false);
    
    QDataStream readStream (&m_fileStdIn);
    QByteArray inputData;
    readStream >> inputData;
    
    m_coder.processByteArray (inputData);
    
    m_inputNotifier->setEnabled (true);
}

void PluginApplicationPrivate::send (const QByteArray & array) {
    m_fileStdOut.write (array);
    m_fileStdOut.flush ();
}

void PluginApplicationPrivate::shutdown () {

    m_inputNotifier->disconnect (this);
    qDebug() << "Shutdown started";    

    if (m_post != 0) {
        delete m_post;
        m_post = 0;
    } else if (m_update != 0) {
        delete m_update;
        m_update = 0;
    }
    
    if (m_interface != 0) {
        delete m_interface;
        m_interface = 0;
    }
    
    Q_EMIT (readyToShutdown());
}

void PluginApplicationPrivate::postStart (const QString & pathToEntry,
    WebUpload::Error error) {
    
    if (m_initFailed == true) {
        WebUpload::Error myError = WebUpload::Error::custom ("Plugin Error",
            "Init failed when plugin loaded");
        send (m_coder.uploadFailed (myError));            
        shutdown();
    }
    
    m_entry = new Entry();
    if (m_entry->init (pathToEntry, false) != true) {
        QString message("Invalid entry path given or invalid entry:");
        message.append (pathToEntry);
        WebUpload::Error myError = WebUpload::Error::custom ("Plugin Error",
            message);
        send (m_coder.uploadFailed (myError));
        shutdown ();
        return;
    }

    m_post = m_interface->getPost();
    if (m_post == 0) {
        WebUpload::Error myError = WebUpload::Error::custom ("Plugin Error",
            "Failed to load PostInterface");
        send (m_coder.uploadFailed (myError));
        shutdown ();
        return;
    }
    
    // These are queued connections, because as soon as the bytestream
    // corresponding to these are sent to the webupload engine, we shall
    // shutdown this process
    connect (m_post, SIGNAL (error(WebUpload::Error)), this,
        SLOT (postError(WebUpload::Error)), Qt::QueuedConnection);

    #ifdef WARNINGS_ENABLED                
    connect (m_post, SIGNAL (warning(WebUpload::Error)), this,
        SLOT (postWarning(WebUpload::Error)), Qt::QueuedConnection);
    #endif
   
    connect (m_post, SIGNAL (done()), this, SLOT (commonDone()),
        Qt::QueuedConnection);
    connect (m_post, SIGNAL (stopped()), this, SLOT (commonStopped()),
        Qt::QueuedConnection);

    connect (m_post, SIGNAL (progress(float)), this, 
        SLOT (postProgress(float)));
    connect (m_post, SIGNAL (mediaStarted(WebUpload::Media*)), this,
        SLOT (postMediaStarted(WebUpload::Media*)));
    connect (m_post, 
        SIGNAL (pending(WebUpload::PostInterface::PendingReason,QString)), this,
        SLOT (postPending(WebUpload::PostInterface::PendingReason,QString)));
    
    m_post->upload (m_entry, error);    
}

void PluginApplicationPrivate::stop () {
    if (m_post != 0) {
        m_post->stop();
    } else if (m_update != 0) {
        m_update->cancel();
    } else {
        shutdown ();
    }
}

void PluginApplicationPrivate::postError (WebUpload::Error error) {
    qDebug() << __FUNCTION__ << error.code();
    m_post->disconnect (this);
    send (m_coder.uploadFailed (error));
    shutdown ();
}

#ifdef WARNINGS_ENABLED            
void PluginApplicationPrivate::postWarning (WebUpload::Error warning) {
    qDebug() << __FUNCTION__ << warning.code();
    send (m_coder.uploadWarning (warning));
}
#endif

void PluginApplicationPrivate::commonStopped () {
    if(m_post != 0) {
        m_post->disconnect (this);
    } else if (m_update != 0) {
        m_update->disconnect (this);
    }
    
    send (m_coder.stopped ());
    shutdown ();
}

void PluginApplicationPrivate::commonDone () {
    if(m_post != 0) {
        m_post->disconnect (this);
    } else if (m_update != 0) {
        if (m_account != 0) {
            m_account->syncValues();
        }
        m_option = 0;
        m_update->disconnect (this);
    }
    
    send (m_coder.done ());
    shutdown ();
}

void PluginApplicationPrivate::postProgress (float uploaded) {
    send (m_coder.progress (uploaded));
}

void PluginApplicationPrivate::postPending (
    WebUpload::PostInterface::PendingReason reason, QString message) {
    
    qWarning() << "postPending not implemented" << reason << ":" << message;

    Q_UNUSED (reason);
    Q_UNUSED (message);     
}

void PluginApplicationPrivate::postMediaStarted (WebUpload::Media* media) {
    int index = -1;
    
    if (m_entry != 0) {
        index = m_entry->indexOf (media);
    }

    send (m_coder.sendingMedia (index));
}

bool PluginApplicationPrivate::initUpdate (const QString & accountStringId,
    const QStringList & optionIds) {
    
    if (m_initFailed == true) {
        send (m_coder.updateFailed (WebUpload::Error::CODE_CUSTOM, optionIds));
        return false;
    }
    
    m_account = new WebUpload::Account (this);
    if (m_account->initFromStringId (accountStringId) == false) {
        send (m_coder.updateFailed (WebUpload::Error::CODE_ACCOUNT_REMOVED,
            optionIds));
        m_account->deleteLater();
        m_account = 0;
        return false;
    }
    
    m_update = m_interface->getUpdate();
    if (m_update == 0) {
        send (m_coder.updateFailed (WebUpload::Error::CODE_CUSTOM, optionIds));
        m_account->deleteLater();
        m_account = 0;        
        return false;
    }
    
    // These are queued connections because process will shutdown and delete
    // post class in these slots. Without queued connection this will crash and
    // burn.
    connect (m_update, SIGNAL (done()), this, SLOT (commonDone()),
        Qt::QueuedConnection);    
    connect (m_update, SIGNAL (canceled()), this, SLOT (commonStopped()),
        Qt::QueuedConnection);
    connect (m_update, SIGNAL (error(WebUpload::Error::Code,QStringList)), this,
        SLOT (updateError(WebUpload::Error::Code,QStringList)),
        Qt::QueuedConnection);
    
    return true;
}

void PluginApplicationPrivate::updateAddValue (QString accountStringId, 
    QString optionId, QString valueName) {
    
    QStringList optionIds;
    optionIds << optionId;    
    if (initUpdate (accountStringId, optionIds) == false) {
        shutdown();
        return;
    }
    
    m_option = m_account->service()->serviceOption (optionId);
    if (m_option == 0) {
        send (m_coder.updateFailed (WebUpload::Error::CODE_CUSTOM,
           QStringList()));
        shutdown();
        return;
    }
    
    m_update->addValue (m_account, m_option, valueName);    
}

void PluginApplicationPrivate::updateValue (QString accountStringId, 
    QString optionId) {

    QStringList optionIds;
    optionIds << optionId;
    if (initUpdate (accountStringId, optionIds) == false) {
        shutdown();
        return;
    }    
    
    m_option = m_account->service()->serviceOption (optionId);
    if (m_option == 0) {
        send (m_coder.updateFailed (WebUpload::Error::CODE_CUSTOM,
           QStringList()));
        shutdown();
        return;
    }    
        
    m_update->update (m_account, m_option);
}

void PluginApplicationPrivate::updateAll (QString accountStringId) {

    if (initUpdate (accountStringId, QStringList()) == false) {
        shutdown();
        return;
    }
        
    m_update->updateAll (m_account);
}

void PluginApplicationPrivate::updateError(WebUpload::Error::Code error,
    QStringList failedIds) {
    
    m_update->disconnect (this);
    
    send (m_coder.updateFailed (error, failedIds));
    shutdown ();
}