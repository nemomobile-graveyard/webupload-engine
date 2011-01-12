 
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

#ifndef _WEBUPLOAD_MEDIA_H_
#define _WEBUPLOAD_MEDIA_H_

#include <WebUpload/export.h>
#include <WebUpload/enums.h>
#include <QObject>
#include <QString>
#include <QVectorIterator>
#include <QDomElement>
#include <QList>
#include <QUrl>
#include <QStringList>
#include <QMetaType>

class MDataUri;

namespace WebUpload {

    class MediaPrivate;
    class Entry;

    /*!
        \class Media
        \brief Media element class containing file and metadata related to that
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT Media : public QObject {
    
        Q_OBJECT

    public:
        
        /*!
          \brief  Constructor
          \param parent Entry
         */
        Media (QObject *parent = 0);
        
        /*!
          \brief  Destructor
         */
        virtual ~Media();

        /*!
          \brief Initialize media from XML data
          \param mediaElem Media XML element
          \return true/false depending on whether or not the structure
                  could be filled.
         */
        bool init (QDomElement &mediaElem);

        /*!
          \brief Initialize media from file tracker IRI
          \param tIri Tracker IRI to file information in Tracker
          \return <code>true</code> if init was success
         */            
        bool initFromTrackerIri (const QString &tIri);
        
        /*!
          \brief Initialize media from data uri
          \param dUri Data URI
          \return <code>true</code>/<code>false</code> depending on whether or
                  not the structure could be filled
         */
        bool initFromDataUri (const MDataUri & dUri);
                
        /*!
          \brief Initial media with given values. Given values has to be valid.
                 This function skips many tracker queries and so is fasted than
                 normal init with Tracker URI.
          \param tIri IRI to file information in tracker
          \param fileUri Files URI
          \param mimeType Mime type of the file
          \param size Size of the file in bytes
          \param fileTitle Title of the file, as in tracker
          \param fileDesc Description of the file, as in tracker
          \return <code>true</code> or <code>false</code> depending on whether
                  or not the structure could be filled
         */          
        bool fastInitFromTrackerIri (const QString &tIri,
            const QString & fileUri, const QString &mimeType, qint64 size,
            const QString & fileTitle, const QString & fileDesc);
                    
        /*! 
          \brief  Get the pointer to the Entry class to which this media
                  instance belongs
          \return Pointer to the Entry class if this media has been added
                  to a WebUpload::Entry class. Otherwise null.
         */            
        const Entry * entry() const;
        
        /*!
          \brief Get tracker IRI of transfer element
          \return tracker Iri of transfer element
         */                        
        QString trackerIri() const;
        
        /*!
          \brief Get URI to original file
          \return URI to file or empty string if not defined
         */
        QUrl origURI() const;
        
        /*!
          \brief Get path to copy file made. This is the file which should be
                 uploaded to services. All modifications before sending are done
                 to this file. If media isn't file based there never will be
                 copy file.
          \return Path to file if there is copy file made. Empty string if not.
         */
        QString copyFilePath() const;
        
        /*!
          \brief Get copied data content. This is non file data given as upload
                 content. This is alternative for copyFilePath() function.
          \return Data content as string if available. Empty string if no text
                  data was copied for this media.
         */
        QString copiedTextData() const;

        //! \brief Read state of the media from tracker and set it here
        void refreshStateFromTracker ();
        
        //! Media types
        enum Type {
            TYPE_UNDEFINED, //!< Media is uninitialized

            //! Media is file based (it will have copyFilePath)
            TYPE_FILE,
            
            //! Media is text data based (data will have copiedTextData)
            TYPE_TEXT_DATA
        };
                
        /*!
          \brief Media type
          \return Type of media
         */
        Type type() const;
                    
        /*!
          \brief Get MIME type of media
          \return MIME type of media
         */
        QString mimeType() const;
                      
        /*!
          \brief Get file size of media
          \return Size in bytes or -1 if undefined
         */
        qint64 fileSize() const;
                    
        /*!
          \brief Get file name of media
          \return File name
         */
        QString fileName() const;
                    
        /*!
          \brief Get title
          \return Title of media
         */
        QString title() const;
                    
        /*!
          \brief Get description.
          \return Description of media
         */
        QString description() const;
                       
        /*!
          \brief Get tags
          \return Tags of media in a stringlist
         */
        QStringList tags () const;

        /*!
          \brief Get urls of tags applicable to this media. 
                 This function will return correct value only if the class was
                 initialized from tracker, and not from xml file.
          \return Url list of tags of media
         */
        QList<QUrl> tagUrls() const;
        
        /*!
          \brief Get Tracker types
          \return String list of tracker typed for the media
         */
        const QList<QUrl> trackerTypes() const;          
        
        //! Copy results
        enum CopyResult {
            COPY_RESULT_SUCCESS, //!< Copy made successfully            
            COPY_RESULT_UNDEFINED_FAILURE, //!< Undefined failure
            COPY_RESULT_ALREADY_COPIED, //!< Old copy exists
            COPY_RESULT_FORBIDDEN, //!< Media state forbids copying
            COPY_RESULT_METADATA_FAILURE, //!< Failed to write metadata to copy
            COPY_RESULT_NO_SPACE, //!< If there isn't enough space for copy
            COPY_RESULT_NOTHING_TO_COPY, //!< There is nothing to copy (success)
            
            //! This error should only be used internally while processing
            COPY_RESULT_FILETYPE_NOT_ACCEPTED
        };
        
        /*!
          \brief Make copy from original file. If this media already has
                 own copy this function will return false and doesn't do
                 anything.
          \param path Where copy should be done. If empty then default path
                      is used.
          \return <code>CopyResult</code> result of copy
         */            
        CopyResult makeCopy(const QString &path = "");
        
        /*!
          \brief Is media still in pending state
          \return true if media is still in pending state (returns false if
                  the media is in pending state due to error)
         */
        bool isPending() const;

        /*!
          \brief Is media being sent currently
          \return true if media is in active state
         */
        bool isActive() const;
        
        /*!
          \brief Is media already sent
          \return true if media is already sent
         */            
        bool isSent() const;

        /*!
          \brief Is media canceled
          \return true if media is canceled
         */            
        bool isCanceled() const;

        /*!
          \brief Does the media have error 
          \return true if the last attempt to send this media in this
                  session resulted in error
         */
        bool hasError() const;

        /*!
          \brief Is media still in paused state
          \return true if media is still in paused state 
         */
        bool isPaused() const;
        
        /*!
           \brief  Change the state of the media to active
           \return true/false depending on whether the media could be
                   marked as completed.
         */
         bool setActive();

        /*!
           \brief  Change the state of the media to completed and set the
                   destination URL
           \param  destUrl : URL to where uploaded file can be found
           \return true/false depending on whether the media could be
                   marked as completed.
         */
         bool setCompleted(const QString &destUrl);

        /*!
           \brief  Change the state of the media to canceled
           \return true/false depending on whether the media could be
                   marked as canceled.
         */
         bool setCanceled();

        /*!
           \brief  Change the state of the media to failed
           \return true/false depending on whether the media could be
                   marked as failed.
         */
         bool setFailed();

        /*!
           \brief  Clear error associated with the media
           \return true/false depending on whether the error could be
                   cleared
         */
         bool errorFixed();

        /*!
           \brief  Change the state of the media to paused
           \return true/false depending on whether the media could be
                   marked as paused.
         */
         bool setPaused();
        
        /*!
          \brief Adds a new instance of mto::TransferElement into the
                 tracker. All initializations of this instance are done as
                 RDF statements and added into the list recieved as
                 argument. All instances in this argument are then applied
                 at one time. This reduces the amount of DBus
                 communications with the tracker, and should be faster.
          \return The URI of the transferElement created.
         */
        QUrl addToTracker();
        
        /*!
          \brief Serialize media to XML format
          \param doc Document used to make XML data
          \return DOM element containing data
         */            
        QDomElement serializeToXML(QDomDocument &doc) const;

        /*!
          \brief Set option value to media
          \param id ID of option
          \param value Id of option
         */
        void setOption (const QString &id, const QString &value);
        
        /*!
          \brief Get option value from media. If media does not have value
                 defined then parent entry will be asked.
          \param id Id of option searched
          \return String containing the value, or empty if value is not
                  found
         */
        QString option(const QString & id) const;
        
        /*!
          \brief Deprecated, usee initFromTrackerIri
         */
        Q_DECL_DEPRECATED bool init (const QString & tUri);        
        
        /*!
          \brief Deprecated use fastInitFromTrackerIri
         */
        Q_DECL_DEPRECATED bool fastInit (const QString &tUri,
            const QString & fileUri, const QString &mimeType, qint64 size);
            
        /*!
          \brief Deprecated use trackerIri()
         */
        Q_DECL_DEPRECATED QString trackerURI() const;
        
        
    public Q_SLOTS:

        /*!
          \brief Set title
          \param newTitle New title for media
         */
        void setTitle (const QString &newTitle);
        
        /*!
          \brief Set description
          \param newDescription New description for media
        */            
        void setDescription (const QString &newDescription);    
               
        /*!
          \brief Append new tag to media.
          \param tagUrl Tracker url of tag appended to media
         */            
        void appendTag (const QUrl &tagUrl);
        
        /*!
          \brief Remove tag from media if found
          \param tagUrl Tracker url of tag removed
         */            
        void removeTag (const QUrl &tagUrl);
        
        /*!
          \brief Append new tag to media.
          \param tagUrl String value of tag appended to media
         */            
        void appendTag (const QString &tagUrl);
        
        /*!
          \brief Remove tag from media if found
          \param tagUrl String value of tag removed
         */            
        void removeTag (const QString &tagUrl);
        
        /*!
          \brief Clear all tags
         */            
        void clearTags();
        
        /*!
          \brief Remove copy file if it exists
          \return true if there is no copy file after this function
         */            
        bool removeCopyFile();

        /*!
          \brief Slot mainly for Entry::shareOptionChanged notification of
                 option changes. Will emit media's changes as signals.
         */
        void shareOptionsChange (int metadataOptions);
        
        /*!
          \brief Add file that should be removed from the filesystem with media
          \param filePath File path
         */
        void addToCleanUpList (const QString & filePath);
        
    Q_SIGNALS:
        /*!
          \brief Signal when state of media changes
          \param media Which media has state changed
         */
        void stateChanged (const WebUpload::Media * media);
                    
        /*!
          \brief Signal emitted when title of media has changed. For now
                 only when value is set with setTitle function. Not emitted
                 when media is initialized.
          \param title New title of media
         */
        void titleChanged (QString title);
        
        /*!
          \brief Signal emitted when description of media has changed. For
                 now only when value is set with setTitle function. Not
                 emitted when media is initialized.
          \param description New description of media
         */
        void descriptionChanged (QString description); 
        
        /*!
          \brief Signal emitted when tags related to media has been changed.
                 For now signal is only emitted when change is done with
                 via appendTag() removeTag() clearTags() functions.
          \param tags Updated list of tags
         */
        void tagsChanged (QList<QUrl> tags);                   

    private:
                
        MediaPrivate * const d_ptr; //!< Private data of class
    };
}

Q_DECLARE_METATYPE(WebUpload::Media::CopyResult)

#endif