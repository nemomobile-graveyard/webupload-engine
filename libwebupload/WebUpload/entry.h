
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

#ifndef _WEBUPLOAD_ENTRY_H_
#define _WEBUPLOAD_ENTRY_H_

#include <WebUpload/export.h>
#include <WebUpload/Account>
#include <QObject>
#include <QString>
#include <QVectorIterator>
#include <QList>
#include <QUrl>
#include <WebUpload/enums.h>
#include <QDateTime>
#include <WebUpload/GeotagInfo>

namespace WebUpload {

    class EntryPrivate;
    class Media;

    /*!
        \class Entry
        \brief Entry class containing data transfered to services
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT Entry : public QObject {
        Q_OBJECT

    public:

        /*!
          \brief Create new empty entry
          \param parent QObject parent
         */
        Entry (QObject *parent = 0);
        virtual ~Entry();

        /*!
          \brief Load entry from file
          \param path Local path to file where entry is read
          \param allowSerialization If entry is initialized with this flag off
                                    then you can not serialize it. This is
                                    safety feature used when entry is given to
                                    upload plugins.
          \return true if init success is success
         */
        bool init (const QString &path, bool allowSerialization = true);

        /*!
          \brief Get localpath where entry is serialized
          \return Localpath or empty string if not serialized
         */
        QString serializedTo() const;

        /*!
          \brief  Get the tracker URI represented by this transfer. If the
                  information in this WebUpload::Entry class has never been
                  serialized, this would be an empty string.
          \return Constant referent to the string containing the tracker
                  URI
         */
        QString trackerIRI() const; 
         
        /*!
          \brief Get name of the account to which the transfer is to happen.
                 Use account instead if possible. This function might be
                 removed.
          \return Constant reference to the string containing the account
                  name
         */
        QString accountId() const;
        
        /*!
          \brief Get account marked as destination of this entry
          \return Shared account pointer
         */
        SharedAccount account () const;

        /*!
          \brief Get option value from entry
          \param id Id of option searched
          \return String containing the value, or empty if value is not
                  found
         */
        QString option(const QString & id) const;

        /*!
          \brief Can this entry instance make file changes?
          \return <code>true</code> if this entry instance can make file
                  changes. This includes changes like deleting the entry xml
                  and the copy files
                  <code>false></code> otherwise
         */
        bool canReserialize () const;

        /*!
          \brief Is media still in pending state
          \return <code>true</code> if media is still in pending state, but 
                  <code>false</code> if the media is in pending state due to
                  error.
         */
        bool isPending() const;

        /*!
          \brief Is media being sent currently
          \return <code>true</code> if media is in active state
         */
        bool isActive() const;
        
        /*!
          \brief Is media already sent
          \return <code>true</code> if media is already sent
         */            
        bool isSent() const;

        /*!
          \brief Is media canceled
          \return <code>true</code> if media is canceled
         */            
        bool isCanceled() const;

        /*!
          \brief Does the media have error 
          \return <code>true</code> if the last attempt to send this media in
                  this session resulted in error
         */
        bool hasError() const;

        /*!
          \brief Is media still in paused state
          \return <code>true</code> if media is still in paused state 
         */
        bool isPaused() const;

        /*!
          \brief Append media to entry. Media is own by Entry after this.
          \param newMedia New media element added to entry
         */
        void appendMedia (Media *newMedia);

        /*!
          \brief Return the index of the media in the list(vector) of medias
          \param media Media whose index is required
          \return index as of media
         */
        int indexOf (Media * media) const;

        /*!
          \brief Get media iterator
          \return Iterator to browser media
         */
        QVectorIterator<Media *> media() const;

        /*!
          \brief Get media with index number
          \param index Index of media (starting from 0)
          \return Pointer to media or null if no media found with given
                   index
         */
        Media * mediaAt (int index) const;

        /*!
          \brief  Get the pointer to the next media which has not been sent
          \param includeError parameter has value <code>true</code> if even
                 media which are unsent due to error are supposed to be
                 included. Default value for this parameter is
                 <code>false</code>
          \return Pointer to the next media which has not been sent or null
                  if all media has been sent
         */
        Media * nextUnsentMedia (bool includeError = false) const;

        /*!
          \brief How many media elements entry has
          \return Number of media elements in entry
         */
        unsigned int mediaCount() const;

        /*!
          \brief How many media elements were sent 
          \return Number of media elements sent in the entry
        */
        unsigned int mediaSentCount() const;

        /*!
          \brief Get total size
          \return Total size of entry
         */
        qint64 totalSize() const;

        /*!
          \brief Get size of unsent parts
          \return Size of unsent parts
         */
        qint64 unsentSize() const;

        /*!
          \brief Read the tags in the media elements and either add them to the
                 common tags list or partially selected tags list.
          \param commonTags Reference to list of urls which will be filled in
                    this function with the list of tags applicable to ALL the
                    media
          \param partialTags Reference to map of urls versus count of number of
                    media for which the tag is applicable. This map will be
                    filled in this function. 
         */
        void getAllTags (QList<QUrl> & commonTags, 
            QMap<QUrl, int> & partialTags) const;

        /*!
          \brief Read the tags in the media elements and either add them to the
                 common tags list or partially selected tags list.
          \param commonTags Reference to list of urls which will be filled in
                    this function with the list of tags applicable to ALL the
                    media
          \param partialTags Reference to map of urls versus count of number of
                    media for which the tag is applicable. This map will be
                    filled in this function. 
          \param commonGeotag If all the media have the exact same
                    country,city,district triple, then this parameter will
                    contain that triple, else it will be empty.
          \param partialTags If all the media do not have the exact same 
                    country,city,district triple, then this parameter will
                    contain a map of each distinct instance of geotag versus a
                    count of the number of media for which that geotag is
                    applicable. 
         */
        void getAllTags (QList<QUrl> & commonTags, 
            QMap<QUrl, int> & partialTags, GeotagInfo & commonGeotag,
            QMap<GeotagInfo, int> & partialGeotags) const;

        /*!
          \brief Function called to set the common and partial tags in the
                 media. If the media has a tag that does not appear in either
                 the common or the partial tag list, it will be removed.
          \param commonTags List of uris of tags that should be present in all
                    the media
          \param partialTags List of uris of tags that are present in only some
                    of the media. This list is used to figure our whether a
                    non-common tag should still apply to the media or not
         */
        void setTags (QList<QUrl> commonTags, QList<QUrl> partialTags);

        /*!
          \brief Function called to set the geotag which would be common for
                 all the media. Since each media can have only one set of
                 geotag triple, at least one of the parameters would be empty
          \param commonGeotag Geotag information that is applicable for all the
                    media. 
          \param partialGeotags List of geotag triples that are present in only
                    some of the media. This list is used to figure our whether
                    a non-common geotag should still apply to the media or not
         */
        void setGeotag (GeotagInfo commonGeotag, 
            QList <GeotagInfo> partialGeotags);


        /*!
          \brief Get tag that are found in all media elements
          \return List of tags found in all media elements
         */
        QList<QUrl> commonTags () const;

        /*!
          \brief Get unified list of tracker types for all the media. (This
                 is required for the tagging widget)
          \return List of unique tracker types applicable to one or more of
                  the media
         */
        QList<QUrl> allTrackerTypes () const;

        /*!
          \brief  Check if the given metadata type is filtered
          \param  metad Metadata whose sharing filtering is being checked
          \return <code>true</code> if it can not be shared (i.e it is filtered
                  out) and <code>false</code> otherwise
         */
        bool checkShareFilter (MetadataFilter metad) const;

        /*!
          \brief  Get the resize option for images
          \return Enumeration giving information on what kind of resizing
                  needs to be performed, if at all
         */
        ImageResizeOption imageResizeOption() const;

        /*!
          \brief  Get the resize option for videos
          \return Enumeration giving information on what kind of video resizing
                  needs to be performed, if at all
         */
        VideoResizeOption videoResizeOption() const;

        /*!
          \brief Get the metadata filter
          \return Integer value which has the bits corresponding to the
          metadata filter in the enumeration MetadataFilter
        */
        int metadataFilterOption () const;

        /*!
          \brief Mark this entry as canceled
         */
        void cancel ();

        /*!
          \brief Mark this entry as failed
         */
        void setFailed ();

        /*!
          \brief Remove error associated with this entry
         */
        void clearFailed ();
        
        /*!
          \brief Forced entry cleaning. Will remove all resources related
                 to entry. This should be used only to recover system
                 failures.
          \param path Path to entry
          \return <code>true</code> if entry was cleaned
         */
        static bool cleanUp (const QString & path);
        
        /*!
          \brief Get all mime types of all medias in this entry
          \return Mime types found. Each mime type is only once in the list.
         */
        QStringList mimeTypes () const;
        
        /*!
          \brief Get creatation time of entry
          \return When entry was created
         */
        QDateTime created () const;
        
        /*!
          \brief Soft deprecation, USE account()!
         */
        Account * loadAccount (QObject * parent = 0) const;
       
        
    public Q_SLOTS:

        /*!
          \brief Serialize entry to defined path
          \param path Local path where entry is written, including filename
          \return <code>true</code> if success
         */
        bool serialize (const QString &path);

        /*!
          \brief Reserialize entry to old path
          \return <code>true</code> if success
         */
        bool reSerialize();
        
        /*!
          \brief Set id of the account to which this transfer is
                 happening. Use setAccount if possible this function might
                 be removed.
          \param accountId Id of account
         */
        void setAccountId(const QString &accountId);

        /*!
          \brief Pair account to entry
          \param account Pointer to account to be paired with entry
         */
        void setAccount (const WebUpload::Account * account);

        /*!
          \brief Set option value to entry
          \param id ID of option
          \param value Id of option
         */
        void setOption(const QString &id, const QString &value);

        /*!
          \brief Append tag to all media elements in entry
          \param tag Tracker url of the tag to be added to media elements
         */
        void appendTagToAllMedia (const QUrl & tag);

        /*!
          \brief Append tag to all media elements in entry
          \param tag String value of tag to be added to media elements
         */
        void appendTagToAllMedia (const QString & tag);

        /*!
          \brief Remove tag from all media elements in entry
          \param tag Tracker url of the tag to be removed from media elements
         */
        void removeTagFromAllMedia (const QUrl & tag);

        /*!
          \brief Remove tag from all media elements in entry
          \param tag String value of tag to be removed from media elements
         */
        void removeTagFromAllMedia (const QString & tag);

        /*!
          \brief Set the resize option for images
          \param resize_option Integer variable which should take be one
                 of the WebUpload::ImageResizeOption enumerations
         */
        void setImageResizeOption (WebUpload::ImageResizeOption resizeOption);

        /*!
          \brief Set the resize option for video
          \param resizeOption Integer variable which should take be one
                 of the WebUpload::VideoResizeOption enumerations
         */
        void setVideoResizeOption (WebUpload::VideoResizeOption resizeOption);

        /*!
          \brief Set metadata filter information which should be shared.
          \param metadataFilter : Integer variable whose bits are set
          according to WebUpload::MetadataFilter enum to indicate metadata
          filter.
        */
        void setMetadataFilter (int metadataFilter);
        
        /*!
          \brief Set shared account to be used with this entry
         */
        void setAccount (SharedAccount account);

    Q_SIGNALS:
        /*!
          \brief New media element was added to entry
          \param media Media element added
         */
        void mediaAdded (const WebUpload::Media * media);

        /*!
          \brief Emitted when the state of the transfer changes
          \param entry Entry which state has changed
         */
        void stateChanged (const WebUpload::Entry * entry);

        /*!
          \brief Emitted when the share options have changed
          \param metadataOptions The changed options
         */
        void shareOptionsChanged (int metadataOptions);

    private:

        class EntryPrivate * const d_ptr; //!< Private data
    };
}

#endif
