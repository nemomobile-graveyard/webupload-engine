
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

#include "WebUpload/Media"
#include "mediaprivate.h"
#include "WebUpload/enums.h"
#include "WebUpload/Entry"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTemporaryFile>
#include <QSize>
#include <MDataUri>
#include <QProcess>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include <QSystemStorageInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QtConcurrentRun>
#include <quillmetadata/QuillMetadata>

// If using qtsparql
#include <QtSparql>
#include <QUuid>

using namespace WebUpload;

const QString Media::PresentationOptionId = QLatin1String ("_PRESENTATION");

Media::Media(QObject *parent) : QObject(parent),
    d_ptr (new MediaPrivate(this)) {
    
}

bool Media::init(QDomElement &mediaElem) {
    // Check that element has correct name
    if(mediaElem.tagName() != "item") {
        qWarning() << "Invalid XML data";
        return false;
    }

    return d_ptr->init(mediaElem);
}

const Entry* Media::entry() const {
    QObject *parentPtr = parent();

    // Returns 0 if the cast could not be made or if the parameter is 0
    return qobject_cast<WebUpload::Entry *>(parentPtr);
}

bool Media::initFromTrackerIri (const QString &tIri) {
    if(tIri.isEmpty()) {
        return false;
    }
    
    return d_ptr->initFromTrackerIri (tIri);
}

bool Media::initFromDataUri (const MDataUri & dUri) {
    if (dUri.isValid() == false) {
        return false;
    }
    
    return d_ptr->initFromDataUri (dUri);
}

bool Media::fastInitFromTrackerIri (const QString &tIri, 
    const QString & fileUri, const QString &mimeType, qint64 size,
    const QString & fileTitle, const QString & fileDesc) {
        
    return d_ptr->fastInitFromTrackerIri (tIri, fileUri, mimeType, size, 
        fileTitle, fileDesc);
}

bool Media::fastInitFromTrackerIriNoTags (const QString &tIri,
    const QString & fileUri, const QString &mimeType, qint64 size,
    const QString & fileTitle, const QString & fileDesc) {

    return d_ptr->fastInitFromTrackerIriNoTags (tIri, fileUri, mimeType, size,
        fileTitle, fileDesc);
}

Media::~Media() {
    // Destroy the private pointer
    delete d_ptr;
}

void Media::setTitle(const QString &newTitle) {
    d_ptr->m_changedTitle = newTitle;
    Q_EMIT (titleChanged (title()));
}

QString Media::title () const {
    return title (true);
}

QString Media::title(bool getMetadataValue) const {
    if ((entry() != 0) &&
        (entry()->checkShareFilter(METADATA_FILTER_TITLE))) {
        return "";
    } else {
        if ((d_ptr->m_changedTitle.isEmpty ()) && (getMetadataValue == true)) {
            return d_ptr->m_metadataTitle;
        } else {
            return d_ptr->m_changedTitle;
        }
    }
}

QString Media::fileName() const {
    return d_ptr->m_fileName;
}

void Media::setDescription(const QString &newDescription) {
    d_ptr->m_changedDescription = newDescription;
    Q_EMIT (descriptionChanged (description()));
}

QString Media::description () const {
    return description (true);
}

QString Media::description(bool getMetadataValue) const {
    if ((entry() != 0) &&
        (entry()->checkShareFilter(METADATA_FILTER_DESCRIPTION))) {
        return "";
    } else {
        if ((d_ptr->m_changedDescription.isEmpty()) && 
            (getMetadataValue == true)) {

            return d_ptr->m_metadataDescription;
        } else {
            return d_ptr->m_changedDescription;
        }
    }
}

void Media::appendTag(const QUrl &tag) {
    if (d_ptr->m_tags.size () != d_ptr->m_tagUrls.size ()) {
        qWarning() << "Tag and tag url lists are not in sync."
            "Ignoring appendTag request";
        return;
    }

    if (!tag.isEmpty()) {
        if (!d_ptr->m_tagUrls.contains (tag)) {
            // Add a tag only if it does not already exist in the list
            QString queryString = "SELECT ?prefLabel WHERE { "
                "?:tagUrl a nao:Tag; nao:prefLabel ?prefLabel . }";
            QSparqlQuery query (queryString);
            query.bindValue ("tagUrl", tag);

            QSparqlResult * result = d_ptr->blockingSparqlQuery (query, true);
            if (result == 0) {
                return;
            } else {
                result->first ();
                d_ptr->m_tags << result->binding(0).value().toString();
            }
            d_ptr->m_tagUrls << tag;
            Q_EMIT (tagsChanged (tagUrls()));
        }
    } else {
        qWarning() << "Can't add empty tag to media";
    }
}

void Media::appendTag(const QString &tag) {
    if (d_ptr->m_tags.size () != d_ptr->m_tagUrls.size ()) {
        qWarning() << "Tag and tag url lists are not in sync."
            "Ignoring appendTag request";
        return;
    }

    if (!tag.isEmpty()) {
        if (!d_ptr->m_tags.contains (tag)) {
            QSparqlQuery query ("SELECT ?tagUrl WHERE { "
                "?tagUrl a nao:Tag; nao:prefLabel ?:tagValue . }");
            query.bindValue ("tagValue", QUrl(tag));

            QSparqlResult * result = d_ptr->blockingSparqlQuery (query);
            if (result == 0) {
                return;
            } else if (result->size () == 0) {
                qDebug() << query.preparedQueryText() << "had no reponse";
                delete result;

                QString uuidToUse =
                    QUuid::createUuid().toString().remove('{').remove('}');
                QString tagUrl = QString("urn:uuid").append (uuidToUse);
                QSparqlQuery insertQuery("INSERT { ?:tagUrl a nao:Tag; "
                    "nao:prefLabel ?:tagValue . }",
                    QSparqlQuery::InsertStatement);
                insertQuery.bindValue ("tagUrl", QUrl(tagUrl));
                insertQuery.bindValue ("tagValue", QUrl(tag));

                result = d_ptr->blockingSparqlQuery (insertQuery);
                if (result == 0) {
                    qDebug() << "Insertion also did not work";
                    qDebug() << "Could not add tag" << tag;
                    return;
                }
                d_ptr->m_tagUrls << tagUrl;
            } else {
                result->first ();
                // Not bothering about the number of results. Just taking the
                // first one.
                d_ptr->m_tagUrls << result->binding(0).value().toString();
            }
            d_ptr->m_tags << tag;

            Q_EMIT (tagsChanged (tagUrls()));
        }
    } else {
        qWarning() << "Can't add empty tag to media";
    }
}

void Media::appendTag (const QUrl &tagUrl, const QString &tag) {

    if (d_ptr->m_tags.size () != d_ptr->m_tagUrls.size ()) {
        qWarning() << "Tag and tag url lists are not in sync."
            "Ignoring appendTag request";
        return;
    }

    if (!tagUrl.isEmpty() && !tag.isEmpty()) {
        if (!d_ptr->m_tagUrls.contains (tagUrl) &&
            !d_ptr->m_tags.contains(tag)) {

            d_ptr->m_tagUrls << tagUrl;
            d_ptr->m_tags << tag;
            Q_EMIT (tagsChanged (tagUrls()));
        }
    }
    else {
        qWarning() << "Can't add empty tag to media";
    }
}

QString Media::mimeType() const {
    return d_ptr->m_mimeType;
}

qint64 Media::fileSize() const {
    return d_ptr->m_size;
}

QUrl Media::origURI() const {
    return d_ptr->m_origFileUri;
}

QUrl Media::origFileTrackerURI() const {
    return d_ptr->m_origFileTrackerUri;
}

QString Media::trackerIri() const {
    return d_ptr->m_trackerURI;
}

QString Media::copyFilePath() const {
    return d_ptr->m_copyFileUri.toLocalFile();
}

const QList<QUrl> Media::trackerTypes() const {
    d_ptr->queryTrackerTypes ();
    return d_ptr->m_trackerTypes;
}

QList<QUrl> Media::tagUrls() const {
    if ((entry() != 0) &&
        (entry()->checkShareFilter(METADATA_FILTER_TAGS))) {
        return QList<QUrl>();
    } else {
        return d_ptr->m_tagUrls;
    }
}

QStringList Media::tags() const {
    if ((entry() != 0) &&
        (entry()->checkShareFilter(METADATA_FILTER_TAGS))) {
        return QStringList();
    } else {
        return d_ptr->m_tags;
    }
}

QStringList Media::allTags() const {
    QStringList alltags;
    if ((entry() != 0) && (!entry()->checkShareFilter(METADATA_FILTER_TAGS))) {
        alltags = d_ptr->m_tags;
        if (!d_ptr->m_geotag.isEmpty ()) {
            alltags << d_ptr->m_geotag.country();
            alltags << d_ptr->m_geotag.city();
            alltags << d_ptr->m_geotag.district();
        }
    }

    return alltags;
}

void Media::clearTags() {
    if (d_ptr->m_tags.isEmpty() == false) {
        d_ptr->m_tags.clear();
        d_ptr->m_tagUrls.clear();
        Q_EMIT (tagsChanged (tagUrls()));
    }
}

bool Media::setActive() {
    bool retVal = false;

    if((d_ptr->m_state == TRANSFER_STATE_PENDING) ||
       (d_ptr->m_state == TRANSFER_STATE_PAUSED)) {

        qDebug() << "MEDIA STATE CHANGE: active";
        // Clear error. Should have happened anyways
        d_ptr->m_hadError = false;
        if (d_ptr->m_state == TRANSFER_STATE_PENDING) {
            d_ptr->m_startTime = QDateTime::currentDateTime();
        }

        d_ptr->m_state = TRANSFER_STATE_ACTIVE;

        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    } else if (d_ptr->m_state == TRANSFER_STATE_ACTIVE) {
        retVal = true;
    } else {
        qWarning() << "Invalid media state change from" << d_ptr->m_state
            << "to active";
    }

    return retVal;
}

bool Media::setCompleted (const QString &destUrl) {
    bool retVal = false;

    if (d_ptr->m_state == TRANSFER_STATE_ACTIVE) {
        qDebug() << "MEDIA STATE CHANGE: done";
        d_ptr->m_destUrl = destUrl;
        d_ptr->m_state = TRANSFER_STATE_DONE;
        d_ptr->m_completedTime = QDateTime::currentDateTime();

        removeCopyFile();
        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    } else if (d_ptr->m_state == TRANSFER_STATE_DONE) {
        retVal = true;
    } else {
        qWarning() << "Invalid media state change from" << d_ptr->m_state
            << "to completed";
    }

    return retVal;
}

bool Media::setCanceled() {
    bool retVal = false;

    if (d_ptr->m_state == TRANSFER_STATE_CANCELLED) {
        retVal = true;
    } else if (d_ptr->m_state == TRANSFER_STATE_DONE) {
        qDebug() << "Media state is already done";
        retVal = true;
    } else {
        qDebug() << "MEDIA STATE CHANGE: cancelled";
        d_ptr->m_state = TRANSFER_STATE_CANCELLED;

        removeCopyFile();
        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    } 

    return retVal;
}

bool Media::setFailed() {
    bool retVal = false;

    if (d_ptr->m_state == TRANSFER_STATE_ACTIVE) {
        
        qDebug() << "MEDIA STATE CHANGE: pending (failed)";
        d_ptr->m_state = TRANSFER_STATE_PENDING;
        d_ptr->m_hadError = true;

        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    } else if (d_ptr->m_state == TRANSFER_STATE_PENDING) {
        // Can go to failed directly from pending state only while doing 
        // processing. Right now, the only scenario where this happens is that
        // of when the source file has already been deleted. No need to update
        // tracker. Also no need to let entry know about this right now - there
        // is no need to mark entire entry as failed. We can let entry
        // uploading continue and show this error in the end.
        d_ptr->m_hadError = true;
        retVal = true;

    } else if (d_ptr->m_hadError == true) {
        retVal = true;
    } else {
        qWarning() << "Invalid media state change from" << d_ptr->m_state
            << "to failed";
    }    

    return retVal;
}

bool Media::errorFixed() {
    bool retVal = false;

    if (d_ptr->m_state == TRANSFER_STATE_PENDING) {
        qDebug() << "MEDIA STATE CHANGE: pending (error fixed)";
        d_ptr->m_hadError = false;

        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    }

    return retVal;
}

bool Media::setPaused() {
    bool retVal = false;

    if (d_ptr->m_state == TRANSFER_STATE_ACTIVE) {
        
        qDebug () << "MEDIA STATE CHANGE: paused";
        d_ptr->m_state = TRANSFER_STATE_PAUSED;

        d_ptr->updateTracker(true);
        retVal = true;

        Q_EMIT (stateChanged(this));
    } else if (d_ptr->m_state == TRANSFER_STATE_PAUSED) {
        retVal = true;
    }

    return retVal;
}

bool Media::isPending() const {
    return ((d_ptr->m_state == TRANSFER_STATE_PENDING) && 
        (d_ptr->m_hadError == false));
}

bool Media::isActive() const {
    return (d_ptr->m_state == TRANSFER_STATE_ACTIVE);
}

bool Media::isSent() const {
    return (d_ptr->m_state == TRANSFER_STATE_DONE);
}

bool Media::isCanceled() const {
    return (d_ptr->m_state == TRANSFER_STATE_CANCELLED);
}

bool Media::hasError() const {
    return d_ptr->m_hadError;
}

bool Media::isPaused() const {
    return (d_ptr->m_state == TRANSFER_STATE_PAUSED);
}

Media::CopyResult Media::makeCopy (const QString & path) {

    if (type() != TYPE_FILE) {
        qWarning() << "No copy to be made media is not a file";
        return COPY_RESULT_NOTHING_TO_COPY;
    }

    QObject *parentPtr = parent();
    // Returns 0 if the cast could not be made or if the parameter is 0
    Entry * myEntry = qobject_cast<WebUpload::Entry *>(parentPtr);

    if (myEntry) {
        Media::CopyResult retVal = d_ptr->makeCopyOfFile (path,
            myEntry->imageResizeOption(), myEntry->videoResizeOption());
        myEntry->reSerialize ();
        return retVal;
    } else {
        qCritical() << "Parent of media does not seem to be WebUpload::Entry";
        // Cannot resize without knowing the resize option
        return Media::COPY_RESULT_UNDEFINED_FAILURE;
    }
}

void Media::removeTag (const QUrl &tag) {
    if (d_ptr->m_tags.size () != d_ptr->m_tagUrls.size ()) {
        qWarning() << "Tag and tag url lists are not in sync."
            "Ignoring appendTag request";
        return;
    }

    int index = d_ptr->m_tagUrls.indexOf (tag);
    if (index >= 0) {
        d_ptr->m_tagUrls.removeAt (index);
        d_ptr->m_tags.removeAt (index);
        Q_EMIT (tagsChanged (tagUrls()));
    }
}

void Media::removeTag (const QString &tag) {
    if (d_ptr->m_tags.size () != d_ptr->m_tagUrls.size ()) {
        qWarning() << "Tag and tag url lists are not in sync."
            "Ignoring appendTag request";
        return;
    }

    int index = d_ptr->m_tags.indexOf (tag);
    if (index >= 0) {
        d_ptr->m_tagUrls.removeAt (index);
        d_ptr->m_tags.removeAt (index);
        Q_EMIT (tagsChanged (tagUrls()));
    }
}

bool Media::removeCopyFile() {
    QObject *parentPtr = parent();
    // Returns 0 if the cast could not be made or if the parameter is 0
    Entry * myEntry = qobject_cast<WebUpload::Entry *>(parentPtr);

    bool retVal = d_ptr->removeFileCopy();
    if (myEntry) {
        myEntry->reSerialize ();
    } else {
        qCritical() << "Parent of media does not seem to be WebUpload::Entry"
            << "Deleted file, but did not reSerialize";
    }

    return retVal;
}

QUrl Media::addToTracker() {
    return d_ptr->addToTracker();
}

QDomElement Media::serializeToXML(QDomDocument & doc) const {
    const Entry * myEntry = entry ();

    if (myEntry) {
        return d_ptr->serializeToXML(doc, myEntry->metadataFilterOption());
    } else {
        qDebug() << "Parent of media does not seem to be WebUpload::Entry";
        // Return empty QDomElement
        QDomElement retVal;
        return retVal;
    }
}

void Media::shareOptionsChange (int metadataOptions) {
    Q_UNUSED (metadataOptions);
    Q_EMIT (titleChanged (title()));
    Q_EMIT (descriptionChanged (description()));
    Q_EMIT (tagsChanged (tagUrls()));
}

Media::Type Media::type() const {
    return d_ptr->type();
}

QString Media::copiedTextData() const {
    return d_ptr->m_copiedTextData;
}

void Media::addToCleanUpList (const QString & filePath) {
    d_ptr->m_cleanUpFiles << filePath;
}

void Media::refreshStateFromTracker () {

    QString queryString = "SELECT ?state WHERE { "
        "?:teUri a mto:TransferElement; mto:state ?state . } ";
    QSparqlQuery query (queryString);
    query.bindValue ("teUri", QUrl(d_ptr->m_trackerURI));

    QSparqlResult * result = d_ptr->blockingSparqlQuery (query, true);
    if (result == 0) {
        // Tracker db has got incorrect state value, perhaps because plugin
        // crashed just after deleting current state, but before updating new
        // state value, so setting state to pending.
        // This might result in same media being uploaded multiple times, but
        // that is better than giving some wierd error to the user
        qDebug() << "Tracker does not have state value set";
        d_ptr->m_state = TRANSFER_STATE_PENDING;
    } else {
        result->first ();
        d_ptr->m_state = 
            transferStateEnum (result->binding(0).value().toString());
        delete result;
    }

    if ((d_ptr->m_state == TRANSFER_STATE_DONE) || 
        (d_ptr->m_state == TRANSFER_STATE_CANCELLED)) {

        removeCopyFile ();
    }

    Q_EMIT (stateChanged (this));

    return;
}

void Media::setOption (const QString &id, const QString &value) {
    d_ptr->m_options.insert (id, value);
}

QString Media::option (const QString & id) const {
    QString ret;
    
    // Try to find from own options
    QMap<QString, QString>::const_iterator i = d_ptr->m_options.find (id);
    if (i != d_ptr->m_options.end()) {
        ret = i.value();
    }

    // Try entry as backup
    if (ret.isEmpty() == true && entry() != 0) {
        ret = entry()->option (id);
    }
    
    return ret;
}

QString Media::srcFilePath () const {
    return d_ptr->srcFilePath ();
}

const GeotagInfo &Media::geotag () const {
    return d_ptr->m_geotag;
}

void Media::setGeotag (const GeotagInfo & geotag) {
    d_ptr->m_geotag = geotag;
}

void Media::clearGeotag () {
    d_ptr->m_geotag.clear ();
}

/*******************************************************************************
 * Definition of functions for MediaPrivate
 ******************************************************************************/
MediaPrivate::MediaPrivate (Media * parent) : QObject (parent),
    m_media (parent), m_state(TRANSFER_STATE_UNINITIALIZED), m_size (-1),
    m_hadError (false), m_sparqlConnection (0) {
}

MediaPrivate::~MediaPrivate() {
    if (m_sparqlConnection != 0) {
        delete m_sparqlConnection;
        m_sparqlConnection = 0;
    }
}

bool MediaPrivate::init(QDomElement & mediaElem) {

    m_copiedTextData = mediaElem.attribute("textData", "");    
    
    // Always read size
    m_size = 0;
    QString temp = mediaElem.attribute("size", "");
    if(temp.isEmpty() == false) {
        bool ok;
        m_size = temp.toLongLong (&ok, 10);
        
        if (ok == false) {
            m_size = 0;
        }
    }
    
    // Check other attributes
    if ((m_state == TRANSFER_STATE_CANCELLED) ||
        (m_state == TRANSFER_STATE_DONE)) {

        if (m_size == 0) {
            qCritical() << "Media, invalid size attribute" << temp;
            return false;
        }
    } else {
    
        QString mimeType = mediaElem.attribute("mime", "");
        if (mimeType.isEmpty() == false) {
            m_mimeType = mimeType;
        }

        QString copyString = mediaElem.attribute("copy", "");
        
        if (!copyString.isEmpty()) {
            QFileInfo fileInfo(copyString);
            m_size = fileInfo.size();
            m_copyFileUri = QUrl::fromLocalFile (copyString);

        } else {
            qDebug() << "Copy file not created yet";
        }
    }

    QString tmp;
    // Read metadata
    QDomNode n = mediaElem.firstChild();

    while(!n.isNull()) {
        QDomElement e = n.toElement();
        n = n.nextSibling(); // This is next after this!

        if(e.isNull()) {
            continue;
        }
        
        if(e.tagName() == "title") {
            m_metadataTitle = e.text();
        } else if(e.tagName() == "description") {
            m_metadataDescription = e.text();
        } else if(e.tagName() == "tags") {
            QDomNode n1 = e.firstChild();
            while(!n1.isNull()) {
                QDomElement e1 = n1.toElement();
                QString tagUrl = e1.attribute ("tracker_url");
                if(e1.tagName() == "tag") {
                    m_tags << e1.text();
                    m_tagUrls << QUrl (tagUrl);
                } else {
                    qDebug() << "Invalid tagName " << e1.tagName() <<
                        ".  Expected \"tag\"";
                }
                n1 = n1.nextSibling();
            }
        } else if (e.tagName () == "geotag") {
            if (m_geotag.isEmpty ()) {
                QDomNode n1 = e.firstChild ();
                while (!n1.isNull ()) {
                    QDomElement e1 = n1.toElement ();
                    if (e1.tagName () == "country") {
                        m_geotag.setCountry (e1.text ());
                    } else if (e1.tagName () == "city") {
                        m_geotag.setCity (e1.text ());
                    } else if (e1.tagName () == "district") {
                        m_geotag.setDistrict (e1.text ());
                    } else {
                        qDebug() << "Invalid geotag" << e1.tagName ()
                            << "with value" << e1.text ();
                    }
                    n1 = n1.nextSibling ();
                }
            } else {
                // There should be only one geotag, so not looping to
                // see if there are more
                qDebug() << "Geotag already defined once. Ignoring "
                    "others";
            }
        } else if (e.tagName() == "cleanUpFile") {
            m_cleanUpFiles << e.text();
            
        } else if(e.tagName() == "options") {
            QDomNode n1 = e.firstChild();
            while(n1.isNull() == false) {
                QDomElement e1 = n1.toElement();
                QString optionId = e1.attribute("id");

                if (optionId.isEmpty() == false) {
                    QString optionValue = e1.text();
                    m_options.insert (optionId, optionValue);
                }

                n1 = n1.nextSibling();
            }
            
        } else {
            qWarning() << "Unknown tag under media:" << e.tagName();
        }
    }

    QString origString = mediaElem.attribute("original", "");
    if (!origString.isEmpty()) {
        m_origFileUri = QUrl::fromLocalFile (origString);
    } else {
        qDebug() << "entry xml created with older version";
    }

    // Read tracker information after reading the rest of the information from
    // the xml file
    m_trackerURI = mediaElem.attribute("tracker", "");
    if (m_trackerURI.isEmpty() == false && readTrackerInfo() == false) {
        m_copiedTextData.clear();
        m_trackerURI.clear();
        return false;
    }
    
    return true;
}

Media::CopyResult MediaPrivate::makeCopyOfFile (
    const QString & suggestedTargetDir, ImageResizeOption imageResizeOption,
    VideoResizeOption videoResizeOption) {

    if ((m_state == TRANSFER_STATE_CANCELLED) || 
        (m_state == TRANSFER_STATE_DONE)) {

        qWarning() << "Invalid transfer state";
        return Media::COPY_RESULT_FORBIDDEN;
    }

    if (!m_copyFileUri.isEmpty()) {
        qWarning() << "Processed file already exists";
        return Media::COPY_RESULT_ALREADY_COPIED;
    }

    QString targetPath;
    Media::CopyResult result = 
        constructTargetFilePath(suggestedTargetDir, targetPath);
    qDebug() << "target path: " << targetPath;

    QString targetDir = QFileInfo(targetPath).absoluteDir().canonicalPath();
    bool enoughDiscSpace = checkDiscSpace(targetDir);

    if (!enoughDiscSpace) {
        qDebug() << "Not enough disc space to make file copy";
        return Media::COPY_RESULT_NO_SPACE;
    }

    QString originalFilePath = srcFilePath ();

    if (m_mimeType.startsWith ("image/")) {
        result = processImage(originalFilePath, targetPath, imageResizeOption);
    } else if (m_mimeType.startsWith ("video/")) {
        result = processVideo(originalFilePath, targetPath, videoResizeOption);
        qDebug() << "video metadata processing result: " << result;
    } else {
        result = processGenericFile(originalFilePath, targetPath);
    }

    qDebug() << "makeCopyOfFile result: " << result;
    qDebug() << "copy made to: " << m_copyFileUri;

    return result;
}


Media::CopyResult MediaPrivate::scaleAndSaveImage (const QString & origPath,
    QString& copyPath, ImageResizeOption imageResizeOption) {

    // Get the original size
    QImageReader originalImage (origPath);
    QSize originalsize = originalImage.size ();
    QSize newSize = originalsize; //new size

    int reSizeScale = 0;
    switch(imageResizeOption) {
        case IMAGE_RESIZE_LARGE:
            reSizeScale = 1920;
            break;
        case IMAGE_RESIZE_MEDIUM:
            reSizeScale = 1280;
            break;
        case IMAGE_RESIZE_SMALL:
            reSizeScale = 640;
            break;
        default: // None
            // Should never happen since we are checking before entering here
            qWarning () << "Invalid resize option " << imageResizeOption;
            return Media::COPY_RESULT_FILETYPE_NOT_ACCEPTED;
    }

    int imageHeight = originalsize.height();
    int imageWidth  = originalsize.width();
    bool htLonger = true;
    int longSide, shortSide, newShort;
    double aspectRatio = 0;

    qDebug() << "Orig size h=" << imageHeight << " w=" <<
        imageWidth;
    if (imageHeight < imageWidth) {
        htLonger = false;
        longSide = imageWidth;
        shortSide = imageHeight;
    } else {
        htLonger = true;
        longSide = imageHeight;
        shortSide = imageWidth;
    }

    aspectRatio =  (1.0 * shortSide) / (1.0 * longSide);
    if(longSide >= reSizeScale) {
        newShort = reSizeScale * aspectRatio;
        // After rescaling the short edge has become less than 1 pixel.
        // Reset to 1 pixel at least
        if (newShort < 1) {
            newShort = 1;
        }
    } else {
        // No need to do any scaling. Just simple copy is required.
        // This error causes normal copying to happen in calling function
        return Media::COPY_RESULT_FILETYPE_NOT_ACCEPTED;
    }

    if (htLonger) {
        newSize.setWidth (newShort);
        newSize.setHeight (reSizeScale);
    } else {
        newSize.setHeight (newShort);
        newSize.setWidth (reSizeScale);
    }

    qDebug() << "New size h=" << newSize.height() << " w=" <<
        newSize.width();

    originalImage.setScaledSize (newSize);
    QImage resizedImage = originalImage.read ();
    if (resizedImage.isNull ()) {
        qCritical () << "Qt didn't load given image, can't scale" 
            << originalImage.error ();
        return Media::COPY_RESULT_UNDEFINED_FAILURE;
    }

    QImageWriter savedImage (copyPath);
    if (!savedImage.canWrite ()) {
        qDebug() << "Can't save image. Setting format as PNG";
        savedImage.setFormat ("png");
        copyPath.append (".png");
        savedImage.setFileName (copyPath);
        m_mimeType = "image/png";
    }

    qDebug() << copyPath;
    if (savedImage.write (resizedImage)) {
        return Media::COPY_RESULT_SUCCESS;
    } else {
        qCritical () << 
            "QImage writer could not write the resized image to " 
            << copyPath << savedImage.error();
        return Media::COPY_RESULT_UNDEFINED_FAILURE;
    }
}
    
bool MediaPrivate::fastInitFromTrackerIri (const QString & tIri,
    const QString & fileUri, const QString &mimeType, qint64 size,
    const QString & fileTitle, const QString & fileDesc) {
    
    bool success = fastInitFromTrackerIriNoTags(tIri, fileUri, mimeType, size,
        fileTitle, fileDesc);

    if (success) {
        success = getTagsFromTracker ();
    }
    
    return success;
}

bool MediaPrivate::fastInitFromTrackerIriNoTags (const QString & tIri,
    const QString & fileUri, const QString &mimeType, qint64 size,
    const QString & fileTitle, const QString & fileDesc) {

    qDebug() << "Fast init of Media" << tIri;

    m_origFileTrackerUri = tIri;
    m_origFileUri = fileUri;
    m_mimeType = mimeType;
    m_size = size;
    m_metadataTitle = fileTitle;
    m_metadataDescription = fileDesc;

    QString filePath = srcFilePath ();
    m_fileName = QFileInfo(filePath).fileName();
    m_copyFileUri.clear();
    m_state = TRANSFER_STATE_PENDING;

    return true;
}

bool MediaPrivate::getTagsFromTracker () {
    qDebug() << "PERF: Getting tags for " << m_origFileUri << ": START";

    QString queryString = "SELECT ?tagUrl ?tag WHERE { "
        "?tagUrl a nao:Tag; nao:prefLabel ?tag . "
        "?:ieElem a nie:InformationElement; nao:hasTag ?tagUrl . } ";
    QSparqlQuery query (queryString);
    query.bindValue ("ieElem", m_origFileTrackerUri);

    QSparqlResult * result = blockingSparqlQuery (query);
    if (result == 0) {
        return false;
    } else {
        // Query can have 0 rows as well - when there are no tags
        while (result->next ()) {
            m_tagUrls << result->binding(0).value().toString();
            m_tags << result->binding(1).value().toString();
        } 

        delete result;
    }

    qDebug() << "PERF: Getting tags for " << m_origFileUri << ": END";

    qDebug() << "PERF: Getting geotag for " << m_origFileUri << ": START";
    QString geotagQueryString = "SELECT ?country ?city ?district WHERE { "
        "?:ieElem a nie:InformationElement . "
        "    OPTIONAL { ?:ieElem slo:location ?loc . "
        "        OPTIONAL { ?loc slo:postalAddress ?pAdd . "
        "            OPTIONAL { ?pAdd nco:country ?country . } "
        "            OPTIONAL { ?pAdd nco:locality ?city . } "
        "            OPTIONAL { ?pAdd nco:region ?district . } "
        "        } "
        "    } "
        "} ";

    QSparqlQuery geotagQuery (geotagQueryString);
    geotagQuery.bindValue ("ieElem", m_origFileTrackerUri);

    result = blockingSparqlQuery (geotagQuery);
    if (result == 0) {
        return false;
    } else {
        // Query can have 0 rows as well - when there are no tags
        while (result->next ()) {
            if (!result->binding(0).value().isNull()) {
                m_geotag.setCountry (result->binding(0).value().toString());
            }
            if (!result->binding(1).value().isNull()) {
                m_geotag.setCity (result->binding(1).value().toString());
            }
            if (!result->binding(2).value().isNull()) {
                m_geotag.setDistrict (result->binding(2).value().toString());
            }
        } 

        delete result;
    }

    qDebug() << "PERF: Getting geotag for " << m_origFileUri << ": END";
    return true;
}

QUrl MediaPrivate::convertTrackerUrl (QUrl url) {
    // QUrl url = dao->getUrl();
    QByteArray array = url.toString().toAscii();
    QString string = QUrl::fromPercentEncoding (array);
    url = QUrl (string);
    return url;    
}

bool MediaPrivate::initFromTrackerIri (const QString &tUri) {
    qDebug() << "Uri  of selected file in tracker is " << tUri;
    qDebug() << "PERF: MediaInit START";
    qDebug() << "Slow init used";

    QString queryString = "SELECT ?fUri ?size ?mime WHERE {"
        "?:tUri a nie:DataObject; nie:url ?fUri; nie:byteSize ?size . "
        "?:tUri a nie:InformationElement; nie:mimeType ?mime . }";
    QSparqlQuery query (queryString);

    // First make query assuming that tUri is tracker uri for transfer
    query.bindValue ("tUri", QUrl (tUri));

    QSparqlResult *result = blockingSparqlQuery (query, true);
    if (result == 0) {
        QUrl fUrl(tUri);
        QString fPath = fUrl.toLocalFile();
        if (fPath.isEmpty()) {
            qCritical() << tUri <<
                " is neither a tracker uri not a file path uri.\n"
                " This cannot be handled";
            return false;
        }

        queryString = QString ("SELECT ?tUri ?size ?mime WHERE {"
        "?tUri a nie:DataObject; nie:url ?:fUri; nie:byteSize ?size . "
        "?tUri a nie:InformationElement; nie:mimeType ?mime . }");
        QSparqlQuery newQuery (queryString);
        newQuery.bindValue ("fUri", QUrl::fromLocalFile(fPath));

        result = blockingSparqlQuery (newQuery, true);
        if (result == 0)  {
            qCritical() << fPath <<
                " does not seem to have been indexed by tracker";
            return false;
        }

        result->first ();
        m_origFileTrackerUri = result->binding(0).value().toString();
        m_origFileUri = QUrl::fromLocalFile (fPath);
    } else {
        m_origFileTrackerUri = QUrl(tUri);
        result->first ();
        m_origFileUri = QUrl(result->binding(0).value().toString());
    }

    m_size = result->binding(1).value().toInt();

    if (m_mimeType.isEmpty()) {
        m_mimeType = result->binding(2).value().toString();
        qDebug() << "Media mime-type is:" << m_mimeType;
        if (m_mimeType.isEmpty()) {
            qWarning() << "Media, Failed resolve mime type";
            return false;
        }
    }

    delete result;
    result = 0;

    QString filePath = srcFilePath ();
    m_fileName = QFileInfo(filePath).fileName();
    m_copyFileUri.clear();
    m_state = TRANSFER_STATE_PENDING;

    getTagsFromTracker ();

    qDebug() << "PERF: MediaInit END";    
    return true;
}
    
QSparqlResult * MediaPrivate::blockingSparqlQuery (const QSparqlQuery &query,
    bool singleResponse) {

    if (m_sparqlConnection == 0) {
        m_sparqlConnection = new QSparqlConnection ("QTRACKER");
        if (!m_sparqlConnection->isValid ()) {
            qDebug() << "Could not create valid QSparqlConnection";
            delete m_sparqlConnection;
            m_sparqlConnection = 0;
            return 0;
        }
    }

    QSparqlResult * result = m_sparqlConnection->exec (query);
    result->waitForFinished ();

    if (result->hasError ()) {
        qDebug() << "Error with query" << query.preparedQueryText() << ":" << 
            result->lastError().message ();
        delete result;
        return 0;
    } else if ((singleResponse == true) && (result->size () != 1)) {
        // There should be exactly 1 row in the response if it was a valid
        // tracker iri
        qDebug() << query.preparedQueryText() << "returned" << 
            result->size() << "rows";
        delete result;
        return 0;
    }

    return result;
}

bool MediaPrivate::initFromDataUri (const MDataUri & dUri) {
    
    MDataUri myUri (dUri);
    
    m_metadataTitle = myUri.attribute("title");
    m_metadataDescription = myUri.attribute("description");
    m_mimeType = myUri.mimeType();
    m_size = myUri.data().size();
    
    // TODO: for now this only cleans base64 icon, doesn't make real copy for it
    QString iconAttr = myUri.attribute ("icon");
    if (myUri.attribute ("icon").startsWith ("base64,") == true) {
        myUri.setAttribute ("icon", QString());
    }
    
    m_copiedTextData = myUri.toString();
    m_copyFileUri.clear();
    m_state = TRANSFER_STATE_PENDING;
        
    return true;
}



/*!
    \brief Serialize media to XML format
    \param doc Document used to make XML data
    \return DOM element containing data
 */
QDomElement MediaPrivate::serializeToXML(QDomDocument & doc, int options) {
    QDomElement mediaTag = doc.createElement("item");

    // Store tracker iri of media transfer
    mediaTag.setAttribute ("tracker", m_trackerURI);

    mediaTag.setAttribute ("original", m_origFileUri.toLocalFile ());

    // Store file path of copied file or textData
    if (m_copyFileUri.isEmpty() == false) {
        mediaTag.setAttribute ("copy", m_copyFileUri.toLocalFile());
    } else  if (m_copiedTextData.isEmpty() == false) {
        mediaTag.setAttribute ("textData", m_copiedTextData);    
    }
    
    // Store mime type of media
    mediaTag.setAttribute ("mime", m_mimeType);

    // Always write size if known (needed in all cases, is original size if
    // copy attribute isn't defined)
    if (m_size > 0) {
        // Don't use setAttribute(qlonglong) as it uses locales
        mediaTag.setAttribute ("size", QString::number(m_size, 10));
    }

    // Title
    QDomElement dataTag = doc.createElement("title");
    dataTag.appendChild(doc.createTextNode(m_media->title (true)));
    mediaTag.appendChild(dataTag);

    // Description
    dataTag = doc.createElement("description");
    dataTag.appendChild(doc.createTextNode(m_media->description (true)));
    mediaTag.appendChild(dataTag);

    // Tags
    if (options != METADATA_FILTER_TAGS) {
        if (m_tags.isEmpty() == false) {
            dataTag = doc.createElement("tags");
            for (int i = 0; i < m_tags.size(); ++i) {
                QDomElement tagTag = doc.createElement("tag");
                tagTag.setAttribute ("tracker_url", 
                    m_tagUrls.at (i).toString ());
                tagTag.appendChild (doc.createTextNode (m_tags.at(i)));
                dataTag.appendChild (tagTag);
            }
            mediaTag.appendChild (dataTag);
        }

        if (!m_geotag.isEmpty ()) {
            dataTag = doc.createElement ("geotag");

            QDomElement tagTag = doc.createElement ("country");
            tagTag.appendChild (doc.createTextNode (m_geotag.country ()));
            dataTag.appendChild (tagTag);

            tagTag = doc.createElement ("city");
            tagTag.appendChild (doc.createTextNode (m_geotag.city ()));
            dataTag.appendChild (tagTag);

            tagTag = doc.createElement ("district");
            tagTag.appendChild (doc.createTextNode (m_geotag.district ()));
            dataTag.appendChild (tagTag);

            mediaTag.appendChild (dataTag);
        }
    }
    
    // Remember files to be cleaned
    for (int i = 0; i < m_cleanUpFiles.size(); ++i) {
        dataTag = doc.createElement("cleanUpFile"); 
        dataTag.appendChild (doc.createTextNode (m_cleanUpFiles.at(i)));
        mediaTag.appendChild (dataTag);
    }
    
    // Write option values
    if(m_options.isEmpty() == false) {
        QDomElement optionsTag = doc.createElement("options");

        QMap<QString, QString>::iterator i = m_options.begin();
        while (i != m_options.end()) {
             QDomElement optionTag = doc.createElement("option");
             optionTag.setAttribute("id", i.key());
             optionTag.appendChild(doc.createTextNode(i.value()));
             optionsTag.appendChild(optionTag);
             ++i;
        }

        mediaTag.appendChild(optionsTag);
    }    
    
    return mediaTag;
}

QString MediaPrivate::srcFilePath () const {
    QString encodedFilePath = m_origFileUri.toLocalFile ();
    QString filePath = QUrl::fromPercentEncoding (encodedFilePath.toAscii());
    return filePath;
}


bool MediaPrivate::readTrackerInfo() {
    if(m_trackerURI.isEmpty()) {
        qWarning() << "Invalid XML data (missing tracker uri)";
        return false;
    }
    qDebug() << "PERF: TrackerMediaRead START";

    // Normally mime type can be read from the xml file, and once we have the
    // xml file properly filled, we don't really require the original file name
    QString queryString = 
        "SELECT ?state ?startTime ?endTime ?ftUri ?fUri ?mime WHERE {"
        "    ?:tUri a mto:TransferElement. "
        "    OPTIONAL { ?:tUri mto:state ?state . } "
        "    OPTIONAL { ?:tUri mto:startedTime ?startTime . } "
        "    OPTIONAL { ?:tUri mto:completedTime ?endTime . }"
        "    OPTIONAL {"
        "        ?:tUri mto:source ?ftUri. "
        "        OPTIONAL {"
        "            ?ftUri a nie:InformationElement; "
        "                nie:mimeType ?mime . "
        "            ?ftUri a nie:DataObject; "
        "                nie:url ?fUri . "
        "        } "
        "    } "
        "}";
    QSparqlQuery query (queryString);
    query.bindValue ("tUri", QUrl(m_trackerURI));
    QSparqlResult * result;

    result = blockingSparqlQuery (query, true);
    if (result == 0) {
        qCritical() << m_trackerURI << 
            " is not a valid TransferElement uri";
        return false;
    }

    result->first ();

    QVariant stateVariant = result->binding(0).value();
    if (!stateVariant.isNull()) {
        m_state = transferStateEnum(stateVariant.toString());
    } else {
        // State value is not set in tracker for some reason. So considering
        // this to be as yet not uploaded
        m_state = TRANSFER_STATE_PENDING;
    }

    QVariant timeVariant = result->binding(1).value();
    if (timeVariant.isValid() && timeVariant.canConvert<QDateTime> ()) {
        QDateTime timeVal = timeVariant.toDateTime();
        if (timeVal.isValid()) {
            m_startTime = timeVal;
        }
    }

    timeVariant = result->binding(2).value();
    if (timeVariant.isValid() && timeVariant.canConvert<QDateTime> ()) {
        QDateTime timeVal = timeVariant.toDateTime();
        if (timeVal.isValid()) {
            m_completedTime = timeVal;
        }
    }

    if (m_copiedTextData.isEmpty ()) {
        m_origFileTrackerUri = result->binding(3).value().toString();
        if (m_origFileUri.isEmpty ()) {
            qDebug() << "Using old xml file, so need file name from tracker";
            m_origFileUri = result->binding(4).value().toString();
            if (m_origFileUri.isEmpty ()) {
                qWarning() << "Tracker indexing probably not done yet, but"
                    "can't handle that with old xml file";
            }
        }

        QString filePath = srcFilePath ();
        QFileInfo fInfo (filePath);

        if (m_origFileTrackerUri.isEmpty ()) {
            qCritical() << "Source not provided for TransferElement";
            return false;
        } 

        m_fileName = fInfo.fileName();

        if (m_mimeType.isEmpty ()) {
            // If mime info could not be read from the xml file, only then do 
            // we need it from tracker
            m_mimeType = result->binding(5).value().toString();
            if(m_mimeType.isEmpty()) {
                qCritical() << "Media, failed to resolve mime type";
                return false;
            }
        }
    }

    qDebug() << "PERF: TrackerMediaRead END";
    return true;
}

QUrl MediaPrivate::addToTracker() {
    qDebug() << __FUNCTION__ << "start";
    if(!m_trackerURI.isEmpty()) {
        return m_trackerURI;
    }

    QString uuidToUse = QUuid::createUuid().toString().remove('{').remove('}');
    m_trackerURI = QString("urn:uuid:").append (uuidToUse);

    QString insertString ("INSERT { ?:tIri a mto:TransferElement "); 
    if (m_origFileTrackerUri.isEmpty () == false) {
        insertString.append ("; mto:source <");
        insertString.append (m_origFileTrackerUri.toString());
        insertString.append ("> ");
    }

    insertString.append ("; mto:state ?:state ");
    QUrl stateId;
    if (m_state == TRANSFER_STATE_ACTIVE) {
        stateId = transferStateIri(TRANSFER_STATE_PENDING);
    } else {
        stateId = transferStateIri(m_state);
    }

    if(m_startTime.isValid()) {
        insertString.append ("; mto:startedTime <");
        insertString.append (m_startTime.toString(Qt::ISODate));
        insertString.append ("> ");
    }

    if(m_completedTime.isValid()) {
        insertString.append ("; mto:completedTime <");
        insertString.append (m_completedTime.toString(Qt::ISODate));
        insertString.append ("> ");
    }

    if(!m_destUrl.isEmpty()) {
        insertString.append ("; mto:destination <");
        insertString.append (m_destUrl);
        insertString.append ("> ");
    }

    insertString.append (". } ");

    QSparqlQuery insertQuery (insertString, QSparqlQuery::InsertStatement);
    insertQuery.bindValue ("tIri", QUrl (m_trackerURI));
    insertQuery.bindValue ("state", stateId);
    QSparqlResult * result = blockingSparqlQuery (insertQuery);

    if (result == 0) {
        m_trackerURI.clear ();
        return QUrl();
    } 

    delete result;
    return QUrl (m_trackerURI);
}

bool MediaPrivate::updateTracker(bool updateState) {
    qDebug() << "updateTracker begin" << m_trackerURI;

    if(m_trackerURI.isEmpty()) {
        qCritical() << "Missing tracker URI";
        return false;
    }

    QString insertString ("INSERT { ?:teIri a mto:TransferElement ");

    if (!m_origFileTrackerUri.isEmpty ()) {
        insertString.append ("; mto:source <");
        insertString.append (m_origFileTrackerUri.toString());
        insertString.append ("> ");
    }

    QUrl stateId;
    if (updateState) {
        qDebug() << "State needs to be updated";
        insertString.append ("; mto:state ?:state ");
        qDebug () << "insertString now is: " << insertString;
        qDebug() << "State is " << m_state;
        if (m_state == TRANSFER_STATE_ACTIVE) {
            stateId = transferStateIri(TRANSFER_STATE_PENDING);
        } else {
            stateId = transferStateIri(m_state);
        }
        qDebug() << "Setting state to" << stateId << "\n";

        QSparqlQuery delQuery ("DELETE { ?:teIri mto:state ?state . } WHERE { "
            "?:teIri mto:state ?state . } ", QSparqlQuery::DeleteStatement);
        qDebug() << "Binding teIri to " << m_trackerURI;
        delQuery.bindValue ("teIri", QUrl(m_trackerURI));
        qDebug () << "Now making a blocking sparql query";
        QSparqlResult * result = blockingSparqlQuery (delQuery);
        if (result == 0) {
            // Deleting state failed - could be because the state is not set?
            qWarning() << "Delete query " << delQuery.preparedQueryText () <<
                " failed";
        }

        switch (m_state) {
            case TRANSFER_STATE_ACTIVE:
                insertString.append ("; mto:startedTime \"");
                insertString.append (m_startTime.toString (Qt::ISODate));
                insertString.append ("\" ");
                // Fall through to add start time to delete statement
                
            case TRANSFER_STATE_PENDING:
            {
                QSparqlQuery delQuery (
                    "DELETE { ?:teIri mto:startedTime ?sTime. } "
                    "WHERE { ?:teIri  mto:startedTime ?sTime . } ",
                    QSparqlQuery::DeleteStatement);
                delQuery.bindValue ("teIri", QUrl(m_trackerURI));
                QSparqlResult * result = blockingSparqlQuery (delQuery);
                // Deleting startedTime failed
                if (result == 0) {
                    qWarning() << "Delete query " << 
                        delQuery.preparedQueryText () << " failed";
                }
            }
            break;

            case TRANSFER_STATE_DONE:
            {
                insertString.append ("; mto:completedTime \"");
                insertString.append (m_completedTime.toString (Qt::ISODate));
                insertString.append ("\" ");

                QSparqlQuery delQuery (
                    "DELETE { ?:teIri  mto:completedTime ?dTime . } "
                    "WHERE { ?:teIri  mto:completedTime ?dTime . } ",
                    QSparqlQuery::DeleteStatement);
                delQuery.bindValue ("teIri", QUrl(m_trackerURI));
                QSparqlResult * result = blockingSparqlQuery (delQuery);
                if (result == 0) {
                    qWarning() << "Delete query " << 
                        delQuery.preparedQueryText () << " failed";
                }
                break;
            }

            default:
                break;
        }
    }

    if (m_destUrl.isEmpty() == false) {
        insertString.append ("; mto:destination <");
        insertString.append (m_destUrl);
        insertString.append ("> ");
    }

    insertString.append (". } ");
    QSparqlQuery insertQuery (insertString, QSparqlQuery::InsertStatement);
    insertQuery.bindValue ("teIri", QUrl(m_trackerURI));
    if (stateId.isEmpty () == false) {
        insertQuery.bindValue ("state", stateId);
    }
    QSparqlResult * result = blockingSparqlQuery (insertQuery);

    if (result == 0) {
        return false;
    } 
    delete result;

    qDebug() << "updateTracker end";
    return true;
}

bool MediaPrivate::removeFileCopy () {

    if ((m_media->entry() != 0) && 
        (m_media->entry()->canReserialize() == false)) {

        return true;
    }

    // First clean up other related files
    for (int i = 0; i < m_cleanUpFiles.count(); ++i) {
        qDebug() << "Remove file" << m_cleanUpFiles.at(i);
        if(QFile::exists(m_cleanUpFiles.at(i)) == false) {
            qCritical() << "Related file" << m_cleanUpFiles.at(i) << 
                "not found" << "so can not be removed";
        } else if(QFile::remove(m_cleanUpFiles.at(i)) == false) {
            qCritical() << "Failed to remove related file:"
                << m_cleanUpFiles.at(i);
        }
    }
    m_cleanUpFiles.clear();

    // No file to remove
    if (m_copyFileUri.isEmpty()) {
        return true;
    }

    QString copyPath = m_copyFileUri.toLocalFile();

    if(!QFile::exists(copyPath)) {
        qWarning() << "Strange, can't find copy file" << copyPath;
        return false;
    } else if(!QFile::remove(copyPath)) {
        qCritical() << "Can't remove copy file" << copyPath;
        return false;
    }

    qDebug() << "Media file" << copyPath << "removed";
    m_copyFileUri.clear();
    return true;
}

void MediaPrivate::queryTrackerTypes() {
    // No need to query
    if (m_trackerTypes.isEmpty() == false) {
        return;
    }

    // Not file
    if (m_origFileTrackerUri.isEmpty() == true) {
        return;
    }
    
    qDebug() << "Query tracker types for media" << m_fileName;

    QSparqlQuery query ("SELECT ?type WHERE { ?:ftUri rdf:type ?type . }");
    query.bindValue ("ftUri", QUrl(m_origFileTrackerUri));

    QSparqlResult * result = blockingSparqlQuery (query);
    if (result != 0) {
        while (result->next ()) {
            m_trackerTypes << QUrl(result->binding(0).value().toString());
        } 

        delete result;
    }

    return;
}


Media::Type MediaPrivate::type() const {
    qDebug() << m_origFileTrackerUri;
    if (m_origFileTrackerUri.isEmpty() == false) {
        return Media::TYPE_FILE;
    } else if (m_copiedTextData.isEmpty() == false) {
        return Media::TYPE_TEXT_DATA;
    } else {
        qWarning() << "Media can not resolve the type";
        return Media::TYPE_UNDEFINED;
    }
}



Media::CopyResult MediaPrivate::constructTargetFilePath(
    const QString& suggestedTargetDir,
    QString& uniqueFilePath)
{ 
    QString targetDirectory;

    if (suggestedTargetDir.isEmpty()) {
        // TODO: Default path is hardcoded here (need to be read somewhere)    
        targetDirectory = QDir::homePath().append("/MyDocs/.share/");
    } else {
        targetDirectory = suggestedTargetDir;
    }

    qDebug() << "Real target dir:" << targetDirectory;

    QDir directory;

    if (!directory.exists(targetDirectory) &&
        !directory.mkpath(targetDirectory)) {
        qCritical () << "Could not create path" << targetDirectory <<
            "for storing copies";
        return Media::COPY_RESULT_UNDEFINED_FAILURE;
    }

    QString originalFilePath = srcFilePath ();
    QFileInfo fileInfo (originalFilePath);
    QString targetFilenameTemplate = targetDirectory + "attachment-XXXXXX";
    
    // Only copy suffix if it's defined
    if (fileInfo.suffix().isEmpty() == false) {
        targetFilenameTemplate += "." + fileInfo.suffix();
        
    // For images we need to make sure that we have file suffix even when
    // original file does not have it (QImageWriter checks the type from file
    // path)
    } else if (m_mimeType == "image/jpeg") {
        targetFilenameTemplate += ".jpg";
    } else if (m_mimeType == "image/png") {
        targetFilenameTemplate += ".png";
    } else if (m_mimeType == "image/gif") {
        targetFilenameTemplate += ".gif"; 
    }
    
    qDebug() << "temp file template:" << targetFilenameTemplate;
    QTemporaryFile tmpFile(targetFilenameTemplate);

    if (tmpFile.open()) {
        uniqueFilePath = tmpFile.fileName();
        tmpFile.close();
    } else {
        //qCritical () << "Could not open the temporary file";
        //return Media::COPY_RESULT_UNDEFINED_FAILURE;
        uint timestamp = QDateTime::currentDateTime().toTime_t();
        uniqueFilePath = targetDirectory + "attachment-" +
            QString::number(timestamp) + "." + fileInfo.suffix();
    }

    qDebug() << "constructed file path:" << uniqueFilePath;

    return Media::COPY_RESULT_SUCCESS;
}



bool MediaPrivate::checkDiscSpace(const QString& targetDirectory) {
    qDebug() << "checkingDiskSpace" << targetDirectory;
    Q_UNUSED(targetDirectory);

    bool enoughSpace = true;


    if (m_size > 0) {
        QtMobility::QSystemStorageInfo storageInfo;
        qint64 availableSpace =
                storageInfo.availableDiskSpace(targetDirectory);
        qDebug() << "available space: " << availableSpace;
        qint64 spaceRequired = m_size + m_spaceCheckMargin;

        if (spaceRequired > availableSpace) {
            qWarning() << "To make copy you need"
                << spaceRequired - availableSpace << "bytes more disk space";
            enoughSpace = false;
        }
    } else {
        enoughSpace = false;
        qCritical() << "Size unknown, no space check for copy done";
    }



    return enoughSpace;
}



Media::CopyResult MediaPrivate::processImage(const QString& originalFilePath,
    const QString& targetPath, ImageResizeOption imageResizeOption) {

    Q_CHECK_PTR (m_media);
    const WebUpload::Entry * entry = m_media->entry();
    Q_CHECK_PTR (entry);

    QString targetFile = targetPath;
    Media::CopyResult result = Media::COPY_RESULT_UNDEFINED_FAILURE;

    if (imageResizeOption != IMAGE_RESIZE_NONE) {
        qDebug() << "scaling and copying image";

        result = scaleAndSaveImage (originalFilePath, targetFile,
            imageResizeOption);

        if (result == Media::COPY_RESULT_FILETYPE_NOT_ACCEPTED) {
            qDebug() << "Scaling failed, have to make normal copy";
            result = copyFile(originalFilePath, targetFile);
        }
    } else {
        qDebug() << "only copying image";
        result = copyFile(originalFilePath, targetFile);
    }

    if (result == Media::COPY_RESULT_SUCCESS) {
        qDebug() << "image copied to" << targetFile;

        if (QuillMetadata::canRead(originalFilePath)) {
            MetadataFilters filters (entry->metadataFilterOption());
            result = filterAndSyncImageMetadata (originalFilePath,
                targetFile, filters);

            if (result != Media::COPY_RESULT_SUCCESS) {
                qCritical() << "Failed to filter/sync image metadata for"
                    << originalFilePath;
            }
        }

        QFileInfo targetFileInfo (targetFile);
        m_size = targetFileInfo.size();
        qDebug() << "Size after any resizing that might be done is " << m_size;
        m_copyFileUri = QUrl::fromLocalFile (targetFile);
    } else {
        qDebug() << "copying the image failed";
    }

    return result;
}



Media::CopyResult MediaPrivate::processVideo(const QString& originalFilePath,
    const QString& targetPath, VideoResizeOption videoResizeOption) {

    qDebug() << "processVideo()";

    Q_CHECK_PTR (m_media);
    const WebUpload::Entry * entry = m_media->entry();
    Q_CHECK_PTR (entry);

    if (videoResizeOption != VIDEO_RESIZE_NONE) {
        qWarning() << "Video resizing not implemented. Ignoring resize option"
            << videoResizeOption;
    }

    MetadataFilters filters (entry->metadataFilterOption());
    Media::CopyResult result = filterAndSyncVideoMetadata(originalFilePath,
        targetPath, filters);

    if (result != Media::COPY_RESULT_SUCCESS) {
        qDebug() << "filtering failed, making plain copy";
        result = copyFile(originalFilePath, targetPath);
    } 

    if (result == Media::COPY_RESULT_SUCCESS) {
        QFileInfo targetFileInfo (targetPath);
        m_size = targetFileInfo.size();
        qDebug() << "Size after any resizing that might be done is " << m_size;

        m_copyFileUri = QUrl::fromLocalFile (targetPath);
    }

    return result;
}



Media::CopyResult MediaPrivate::processGenericFile(
    const QString& originalFilePath, const QString& targetPath) {

    Media::CopyResult result = copyFile(originalFilePath, targetPath);
    QFileInfo targetFileInfo (targetPath);
    m_size = targetFileInfo.size();
    m_copyFileUri = QUrl::fromLocalFile (targetPath);

    return result;
}



Media::CopyResult MediaPrivate::copyFile(const QString& originalFilePath,
    const QString& targetPath) {

    Media::CopyResult result = Media::COPY_RESULT_SUCCESS;

    if(!QFile::copy(originalFilePath, targetPath)) {
        qWarning() << "Could not copy" << originalFilePath << "to"
            << targetPath;
        result = Media::COPY_RESULT_UNDEFINED_FAILURE;
    }

    return result;
}



Media::CopyResult MediaPrivate::filterAndSyncVideoMetadata(
    const QString& originalFilePath, const QString& targetPath,
    MetadataFilters filters) {

    qDebug() << "Filtering and syncing video metadata";

    const QString metaEditor = "/usr/lib/webupload-engine/metawriter";
    QStringList arguments;
    arguments << "-i" << originalFilePath;
    arguments << "-o" << targetPath;

    if (filters.testFlag(METADATA_FILTER_ALL)) {
        arguments << "-x";
    } else {
        if (filters.testFlag(METADATA_FILTER_AUTHOR_LOCATION)) {
            arguments << "-c";
        }

        arguments << "--title" << m_media->title(true);
        arguments << "--description" << m_media->description(true);
    }

    QProcess metaSyncProcess (this);
    int exitCode = EXIT_FAILURE;

    qDebug() << "running metawriter: " << arguments;
    metaSyncProcess.start (metaEditor, arguments);

    // Timeout in milliseconds. Needs to be quite big number so that
    // processing large files don't time out unintentionally. This is
    // mainly a failsafe value.
    int timeout = 300000;

    qDebug() << "waiting for metawriter to finish";
    metaSyncProcess.waitForFinished(timeout);
    QProcess::ExitStatus exitStatus = metaSyncProcess.exitStatus();

    if (exitStatus == QProcess::NormalExit) {
        exitCode = metaSyncProcess.exitCode();
    }

    qDebug() << "metaprocessing exit status: " << exitCode;
    Media::CopyResult result = Media::COPY_RESULT_SUCCESS;

    if (exitCode != EXIT_SUCCESS) {
        result = Media::COPY_RESULT_UNDEFINED_FAILURE;
    }

    return result;
}



Media::CopyResult MediaPrivate::filterAndSyncImageMetadata(
    const QString& originalFilePath, const QString& targetPath,
    MetadataFilters filters) {

    if (m_mimeType == "image/gif") {
        //TODO: Check with AB about this later.
        qWarning () << "Not supporting handling gif image metadata currently";
        return Media::COPY_RESULT_SUCCESS;
    }

    bool metadataWritten = false;

    QuillMetadata originalMetadata(originalFilePath);
    if (filters.testFlag(METADATA_FILTER_ALL)) {
        QVariant orientation =
                originalMetadata.entry(QuillMetadata::Tag_Orientation);

        QuillMetadata emptyMetadata;
        emptyMetadata.setEntry(QuillMetadata::Tag_Orientation, orientation);
        metadataWritten = emptyMetadata.write(targetPath);
    } else {     
        // These media functions will return empty if filter is on
        originalMetadata.setEntry (QuillMetadata::Tag_Title, 
            m_media->title(true));
        originalMetadata.setEntry (QuillMetadata::Tag_Description,
            m_media->description(true));
        originalMetadata.setEntry (QuillMetadata::Tag_Subject,
            m_media->tags());

        originalMetadata.setEntry (QuillMetadata::Tag_Country,
            m_geotag.country());
        originalMetadata.setEntry (QuillMetadata::Tag_City, m_geotag.city());
        originalMetadata.setEntry (QuillMetadata::Tag_Location, 
            m_geotag.district());

        if (filters.testFlag(METADATA_FILTER_AUTHOR_LOCATION)) {
            qDebug() << "Removing creator and location from the metadata";
            originalMetadata.removeEntry (QuillMetadata::Tag_Creator);
            qDebug() << "Calling removeEntries with TagGroup_GPS";
            originalMetadata.removeEntries (QuillMetadata::TagGroup_GPS);
        }

        metadataWritten = originalMetadata.write(targetPath);
    }

    Media::CopyResult result = Media::COPY_RESULT_SUCCESS;

    if (!metadataWritten) {
        result = Media::COPY_RESULT_METADATA_FAILURE;
    }

    return result;
}
