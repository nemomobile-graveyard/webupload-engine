 
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

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QMutex>
#include <QElapsedTimer>
#include <QList>
#include <QDebug>

#define DBGSTREAM qDebug()
#define WARNSTREAM qWarning()
#define CRITSTREAM qCritical()

/**
 * @brief Simple Qt logging redirector class.
 *
 * This class is used to redirect normal Qt debug log messages from stdout into a given file.
 * The messages are appended to the file and prefixed with a date and log level. The purpose of this
 * class is to make logging available for applications where stdout isn't normally visible.
 **/
class Logger {

public:

    /*!
      \brief Create new logger instance
      \param name Name of log. By default timestamp.
     */
    Logger (const QString & name = QString());

    /*!
      \brief Destructor, closes the log file.
     */
    virtual ~Logger ();

    /*!
      \brief Enables filtering of so called "crap" that should not be logged.
             This can be enabled to filter out stuff that makes debugging hard
             because of flooding. All strings in @p patterns are checked
             against each logged message and a case sensitive match means that
             the message is not logged. By default no patterns or filtering are
             used.
             Set to an empty list to disable.
      \param patterns the patterns to check against.
     */
    void setFilter (const QStringList & patterns);
    
    /*!
      \brief Act as new function but check if logging should be enabled first
      \return Pointer to logger or null if logging is not enabled
     */
    static Logger * newIfEnabled();

private:

    /*!
      \brief The actual message handler method. This method will get called for
             each logged @p msg. The log @p type is mapped to a string and
             prefixed, along with the current time. The @p msg is simply
             appended to a log file.
      \param type the log level type.
      \param msg the message to be logged.
     */
    static void messageHandler (QtMsgType type, const char *msg);

    //! an instance pointer to the class
    static Logger * m_instance;

    //! the stream we're writing to
    QTextStream m_stream;

    //! an open file where the above stream writes
    QFile m_file;

    //! a list of log level strings
    QStringList m_log_levels;

    //! the old msg handler
    QtMsgHandler m_old_msg_handler;

    //! a list of patterns to filter out
    QStringList m_patterns;
    
    //! mutex to make code thread safe
    QMutex m_mutex;
    
    //! list keeping track of all threads
    QList<Qt::HANDLE> m_threads;

    //! timer used to get timestamps
    QElapsedTimer m_time;
};

#endif // #ifndef _LOGGER_H_
