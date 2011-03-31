 
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

#ifndef _WEBUPLOAD_MEDIA_PRIVATE_H_
#define _WEBUPLOAD_MEDIA_PRIVATE_H_

#include <QObject>
#include "WebUpload/Media"
#include "WebUpload/enums.h"
#include <QString>
#include <QVector>
#include <QStringList>
#include <QDomElement>
#include "internalenums.h"
#include <QList>
#include <QUrl>
#include <QDateTime>
#include <QMap>
#include "WebUpload/geotaginfo.h"

// If using qtsparql
#include <QtSparql>

namespace WebUpload {
    /*
     * Currently not storing the pointers to the applications that started the
     * transfer and process the transfer
     */
    class MediaPrivate : public QObject {
    
        Q_OBJECT
    
    public:
    
        /*!
          \brief Constructor
          \param parent Parent QObject
        */
        MediaPrivate (Media * parent = 0);
        
        virtual ~MediaPrivate();
        
        Media * m_media; //!< Public class using this private class    

        QString m_trackerURI;

        // Information coming from tracker         
        TransferState m_state; //!< State of media
        QString m_mimeType; //!< Mime type of the media
        qint64 m_size; //!< Size of the media in bytes

        QString m_title; //!< Title of media
        QString m_description; //!< Description of media
        QList<QUrl> m_tagUrls; //!< Url of all the tags
        QStringList m_tags; //!< Tags (keywords) of media

        GeotagInfo m_geotag; //!< Geotag info associated with this media

        //! Tracker types, or empty if not yet queried
        QList<QUrl> m_trackerTypes;

        QDateTime m_startTime; //!< When media was started
        QDateTime m_completedTime; //!< When media was completed
        QString m_destUrl; //!< Where media was uploaded
        
        QUrl m_origFileTrackerUri; //!< Tracker IRI of the original file
        QUrl m_origFileUri; //!< URI of original file
        QUrl m_copyFileUri; //<! URI of copied file            
    
        QString m_fileName; //!< Original file name

        //! Set to true if the previous upload attempt in this session had
        //  resulted in an error
        bool m_hadError;
               
        QString m_copiedTextData; //!< Copied text data if any
        QStringList m_cleanUpFiles; //!< Files to be removed with media copy
        
        QMap<QString, QString> m_options; //!< Options stored to media

        QSparqlConnection * m_sparqlConnection; // Connection used with qsparql
        
        /*!
          \brief Create media from XML data
          \param mediaElem Media XML element
          \return true/false depending on whether or not the structure
                  could be filled
        */
        bool init (QDomElement &mediaElem);

        /*!
          \brief Create media from tracker URI
          \param tUri Tracker IRI
          \return true/false depending on whether or not the structure
                  could be filled
        */
        bool initFromTrackerIri (const QString &tUri);

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
          \brief Get tags for media with given tracker uri
                 m_origFileTrackerUri should have been been set before this is
                 called
         */
        bool getTagsFromTracker ();

        bool initFromDataUri (const MDataUri & dUri);
        
        /*!
          \brief Media::fastInitFromTrackerIri
         */
        bool fastInitFromTrackerIri (const QString & tIri,
            const QString & fileUri, const QString &mimeType, qint64 size,
            const QString &fileTitle, const QString &fileDesc);

        /*!
          \brief Media::fastInitFromTrackerIriNoTags
         */
        bool fastInitFromTrackerIriNoTags (const QString & tIri,
            const QString & fileUri, const QString &mimeType, qint64 size,
            const QString &fileTitle, const QString &fileDesc);
        
        /*!
          \brief Query tracker types if not already queried
         */
        void queryTrackerTypes();

        // options here is the integer value whose bits indicate which
        // metadata information is to be shared and which is not
        QDomElement serializeToXML(QDomDocument &doc, int options);

        /*!
          \brief Get the path of the source file
          \return Path of the source file
         */
        QString srcFilePath () const;
        
        bool readTrackerInfo();
        bool updateTracker(bool updateState = false);
        QUrl addToTracker();
                
        /*!
          \brief Make copy for media file
          \param path To which path the copy is stored
          \param imageResizeOption Resize option for image file
          \param videoResizeOption Resize option for video file
          \param spaceNeeded Will check that there is enough space for copy if
                             provided (in bytes)
          \return result
         */
        Media::CopyResult makeCopyOfFile (const QString & path = "", 
            ImageResizeOption imageResizeOption = IMAGE_RESIZE_NONE,
            VideoResizeOption videoResizeOption = VIDEO_RESIZE_NONE);
        
        /*!
          \brief scale the image and copy to the temp file
          \param path to which the copy is stored                
          \param resizeOption image resize option
          \return result
        */
        Media::CopyResult scaleAndSaveImage (const QString & origPath, 
            QString & copyPath, ImageResizeOption resizeOption);
        
        /*!
          \brief Remove copy of file
          \return true if there isn't copy after this function
         */
        bool removeFileCopy ();
        
        /*!
          \brief Convert to url from tracker to valid format
          \param url Url to be converted
          \return Valid url
         */
        QUrl convertTrackerUrl (QUrl url);
        
        Media::Type type() const;
        
        //! To make copy system must have original file size plus this as empty
        //  space for the copy (in bytes).
        static const qint64 m_spaceCheckMargin = 1048576;

        /*!
          \brief Construct a unique (full) file path for the upload item
          \param suggestedTargetDir Suggested target directory for the file
          \param uniqueFilePath Constructed unique file path (out)
          \return Operation result
         */
        Media::CopyResult constructTargetFilePath(
            const QString& suggestedTargetDir, QString& uniqueFilePath);

        /*!
          \brief Check if there is enough space for a file copy to be made
          \param targetDirectory Directory to be checked for disc space
         */
        bool checkDiscSpace(const QString& targetDirectory);

        /*!
          \brief Process source image before uploading
          If processing fails, this functions tries at least to make a direct
          copy of the source file.
          \param originalFilePath Full path to the original file
          \param targetPath Full path to the target file
          \param imageResizeOption Image resize option
          \return Operation result
         */
        Media::CopyResult processImage(const QString& originalFilePath,
           const QString& targetPath, ImageResizeOption imageResizeOption);

        /*!
          \brief Process source video before uploading
          If processing fails, this functions tries at least to make a direct
          copy of the source file.
          \param originalFilePath Full path to the original file
          \param targetPath Full path to the target file
          \param videoResizeOption Video resize option
          \return Operation result
         */
        Media::CopyResult processVideo(const QString& originalFilePath,
            const QString& targetPath, VideoResizeOption videoResizeOption);

        /*!
          \brief Process source file before uploading
          If processing fails, this functions tries at least to make a direct
          copy of the source file.
          \param originalFilePath Full path to the original file
          \param targetPath Full path to the target file
          \return Operation result
         */
        Media::CopyResult processGenericFile(const QString& originalFilePath,
            const QString& targetPath);

        /*!
          \brief Try to make a direct copy of a file
          \param originalFilePath Full path to the original file
          \param targetPath Full path to the target file
          \return Operation result
         */
        Media::CopyResult copyFile(const QString& originalFilePath,
            const QString& targetPath);


        /*!
          \brief Filter and sync metadata to target file
          \param originalFilePath Source file path
          \param targetPath Destination file path
          \return Result of this step
         */
        Media::CopyResult filterAndSyncVideoMetadata(
            const QString& originalFilePath, const QString& targetPath,
            MetadataFilters filters);
        
        /*!
          \brief Filter and sync metadata to target file
          \param originalFilePath Source file path
          \param targetPath Destination file path
          \return Result of this step
         */
        Media::CopyResult filterAndSyncImageMetadata(
            const QString& originalFilePath, const QString& targetPath,
            MetadataFilters filters);

    };
}

#endif
