 
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

#ifndef _WEBUPLOAD_ENTRY_PRIVATE_H_
#define _WEBUPLOAD_ENTRY_PRIVATE_H_

#include <QObject>
#include <QString>
#include <QMap>
#include <QUrl>
#include <QVector>
#include <QDateTime>
#include "WebUpload/enums.h"
#include "WebUpload/Account"
#include "internalenums.h"

#include <QtSparql>

namespace WebUpload {

    class Media;
    class Entry;

    /*
     * Currently not storing the pointers to the applications that started the
     * transfer and process the transfer
     */
    class EntryPrivate : public QObject {
        Q_OBJECT
    
    
    public:
    
        /*!
          \brief Create new private object for Entry
          \param parent If not valid pointer to public object all functionality
                        will not work.
         */
        EntryPrivate (Entry * parent = 0);
        
        virtual ~EntryPrivate ();
        
        Entry * publicObject; //!< Public object and parent

        QDateTime m_created; //!< When created
        
        //! Set to true if the upload of this entry failed in this session
        // and it has not been retried
        bool failed;
        
        WebUpload::TransferState state; //!< Entry state
        
        //!< Resize options for any images selected
        ImageResizeOption image_resize_option; 

        //!< Resize options for any videos selected
        VideoResizeOption video_resize_option; 
        
        MetadataFilters metadataFilter; //!< metadata filter option            

        QString trackerId;
        QString accountId; //!< Id of account related to entry
        SharedAccount m_account; //!< Shared account used by entry
        
        QMap<QString, QString> options; //!< Options stored to entry
        QVector<Media *> media; //!< Medias in entry
        QString serialized_to; //!< If not empty, where serialized
        bool m_allowSerialize; //!< Safety flag

        QList<QUrl> m_allTrackerTypes; //!< Types for all medias 

        QSparqlConnection * m_sparqlConnection; // Connection used with qsparql

        /*!
           \brief Used when the data structure has to be filled from an
                  XML file
           \param path Path of the XML file
           \param entry Pointer to the WebUpload::Entry instance that owns this
                        instance.
           \param demandProper <code>true</code> if content is strict
                               validated
           \return true/false, depending on whether the file could be loaded
                   or not.
         */
        bool init (const QString &path, Entry *entry,
            bool demandProper = true, bool allowSerialization = true);
        
        /*!
          \brief Serialize entry to defined path
          \param path Local path where entry is written, including filename
          \return true if success
         */
        bool serialize(const QString &path);

        /*!
          \brief Function to make the actual sparql query and return the
                 response. 
          \param query Sparql query to be made
          \param singleResponse A boolean parameter that is set to
                    <code>true</code> if only a single row of response is
                    expected. This parameter is used to do some additional
                    result checking
          \return The function returns the result of the sparql query or a null
                  string if the query had errors
         */
        QSparqlResult * blockingSparqlQuery (const QSparqlQuery & query, 
            bool singleResponse=false);

        /*!
          \brief Size of the transfer or size of files already
                 transferred, depending on the parameter passed.
          \param calc_sent A boolean variable which is true when only size
                           of completed transfer is required and false when
                           complete transfer size is required. 
          \return  Size requested for
         */
        qint64 size(bool calc_sent) const;
        
        /*!
           \brief Add the transfer information contained here to the
                  tracker.
           \return Boolean value representing success/failure.
         */
        bool addToTracker(void);
        
        /*!
          \brief Remove serialization of entry and medias related to it
         */
        bool removeSerialized ();
        
        /*!
          \brief Get current option values from account (now service) and
                 store those to entry
         */
        void syncOptionsFromAccount ();

        /*!
          \brief Get current post option values.
         */
        void syncPostOptions ();
        
        /*!
          \brief Implementation of public class' function with same name
         */
        WebUpload::Account * loadAccount (QObject * parent = 0) const;
        
        /*!
          \brief Implementation of public class' function with same name
         */
        void setOption (const QString & id, const QString & value);

        /*!
          \brief Implementation of public class' function with same name
         */
        void setAccountId(const QString &accountId);
    
    public Q_SLOTS:
    
        /*!
          \brief Handle state change in a media element
                 Sets the state of the transfer based on the state of the
                 media element. Remember, at one time, there can only be a
                 single media element being uploaded. The next media
                 element upload starts only after the current media
                 element's upload is completed.
          \param changedMedia  Constant pointer to media element whose
                    state has changed. By default, a null pointer
         */
        void mediaStateChanged (const WebUpload::Media *changedMedia = 0);        
        
    Q_SIGNALS:
    
         /*!
           \brief Signal connected to Entry::stateChanged
         */
        void stateChanged (const WebUpload::Entry * entry);
    };
}

#endif 