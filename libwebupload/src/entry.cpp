
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
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


#include "WebUpload/Entry"
#include "entryprivate.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QDebug>
#include "WebUpload/System"
#include "WebUpload/Entry"
#include "WebUpload/PostOption"
#include "WebUpload/CommonListOption"
#include "WebUpload/CommonSwitchOption"
#include "WebUpload/ServiceOption"
#include "WebUpload/Media"
#include "internalenums.h"
#include <QUuid>

#include <QtSparql>
QUrl methodWeb("http://www.tracker-project.org/temp/mto#transfer-method-web");

using namespace WebUpload;
const QString xmlVersion = "1.0";

Entry::Entry (QObject *parent) : QObject(parent),
    d_ptr(new EntryPrivate (this)) {

}

Entry::~Entry() {

    // Check state so we don't leave files behind
    if (!d_ptr->serialized_to.isEmpty() && (isSent() || isCanceled())) {
        qDebug() << "Unserialization when destroying entry";
        d_ptr->removeSerialized();
    }

    // Delete private data
    delete d_ptr;
}

bool Entry::init (const QString & path, bool allowSerialization) {
    return d_ptr->init (path, this, true, allowSerialization);
}

bool Entry::reSerialize() {
    bool success = false;

    if(d_ptr->serialized_to.isEmpty()) {
        qWarning() << "No file set for the reserialization";
        qWarning() << "Serialize the file to a file first";
        success = false;
    } else {
        success = d_ptr->serialize(d_ptr->serialized_to);
    }

    return success;
}

bool Entry::serialize (const QString & path) {
    // Get current post options first - this should only happen during
    // serialization
    d_ptr->syncPostOptions ();

    return d_ptr->serialize(path);
}

void Entry::appendMedia (Media * newMedia) {

    Q_CHECK_PTR(newMedia);
    newMedia->setParent (this);
    d_ptr->media.append(newMedia);
    QObject::connect (newMedia, SIGNAL(stateChanged(const WebUpload::Media*)),
        d_ptr, SLOT(mediaStateChanged(const WebUpload::Media*)));
    QObject::connect (this, SIGNAL (shareOptionsChanged(int)), newMedia,
        SLOT (shareOptionsChange(int)));

    // Clear the tracker types list since it might no longer be valid
    d_ptr->m_allTrackerTypes.clear ();

    Q_EMIT (mediaAdded(newMedia));
}

void Entry::setAccount (const Account * account) {
    if (account == 0) {
        qCritical() << "null account received";
        return;
    }
    
    if (d_ptr->m_account.isNull() == false &&
        d_ptr->m_account.data() != account) {
        
        qWarning() << "Shared account cleared";
        d_ptr->m_account.clear();
    }
    
    d_ptr->setAccountId (account->stringId());
    d_ptr->syncPostOptions ();
}

void Entry::setAccount (SharedAccount account) {
    if (d_ptr->m_account.data() == account.data()) {
        qWarning() << "setAccount called twice with same account";
        return;
    }
    
    d_ptr->m_account = account;
    d_ptr->accountId = d_ptr->m_account->stringId();
    d_ptr->syncPostOptions ();
}

SharedAccount Entry::account () const {
    if (d_ptr->m_account.isNull()) {
        qDebug() << "Account is not set???";
    }

    return d_ptr->m_account;
}

void Entry::setAccountId (const QString & id) {
    d_ptr->setAccountId (id);
}


void Entry::setOption(const QString & id, const QString & value) {
    d_ptr->setOption (id, value);
}

void Entry::clearFailed () {
    if (d_ptr->failed == false) {
        // Error has already been cleared
        return;
    }

    d_ptr->failed = false;

    QVectorIterator<Media *> mediaIter = media();
    for(; mediaIter.hasNext(); ) {
        Media *mediaInst = mediaIter.next();
        if (mediaInst->hasError()) {
            mediaInst->errorFixed();
        }
    }

    // No need to write to tracker, since tracker does not keep track of
    Q_EMIT (stateChanged(this));
    return;
}

void Entry::cancel () {
    if (d_ptr->state == TRANSFER_STATE_CANCELLED) {
        // Transfer has already been canceled
        return;
    }

    d_ptr->state = TRANSFER_STATE_CANCELLED;

    QVectorIterator<Media *> mediaIter = media();
    for(; mediaIter.hasNext(); ) {
        Media *mediaInst = mediaIter.next();
        
        // First disconnect all signal/slot connections between media and
        // entry, since they are not needed. Also, when cancelling media, it
        // emits stateChanged signal, which we would otherwise use here,
        // causing unnecessary calls
        mediaInst->disconnect (this);
        disconnect (mediaInst);

        if (mediaInst->isSent() == false) {
            // Only completely sent items cannot be canceled. Others
            // (including failed items) can be canceled
            mediaInst->setCanceled();
        }
    }

    if (!d_ptr->trackerId.isEmpty()) {
        qDebug() << "Tracker id for entry is " << d_ptr->trackerId;
        QSparqlQuery delQuery ("DELETE { ?:te mto:transferState ?state . } "
            "WHERE { ?:te mto:transferState ?state . }",
            QSparqlQuery::DeleteStatement);
        delQuery.bindValue ("te", QUrl(d_ptr->trackerId));
        QSparqlQuery insQuery ("INSERT { ?:te a mto:Transfer ; "
            "mto:transferState ?:state . }", QSparqlQuery::InsertStatement);
        insQuery.bindValue("te", QUrl(d_ptr->trackerId));
        insQuery.bindValue("state", transferStateIri(TRANSFER_STATE_CANCELLED));

        QSparqlResult * result;
        // First delete any existing value of state, and then insert new value
        result = d_ptr->blockingSparqlQuery (delQuery);
        if (result != 0) {
            delete result;
            result = 0;
        } else {
            qDebug() << "Could not delete state using query" <<
                delQuery.preparedQueryText ();
            qDebug() << "Trying to change state anyways ...";
        }

        result = d_ptr->blockingSparqlQuery (insQuery);
        if (result == 0) {
            qDebug() << "Insertion failed" << insQuery.preparedQueryText ();
        } else {
            delete result;
            result = 0;
        }
    } else {
        qWarning() << "Not writing to tracker, since entry has not been"\
            "added to tracker yet";
    }

    Q_EMIT (stateChanged(this));
    return;
}

QString Entry::trackerIRI() const {
    return d_ptr->trackerId;
}

Account * Entry::loadAccount (QObject * parent) const {
    qCritical() << "Entry::loadAccount used, move to use Entry::account";
    return d_ptr->loadAccount (parent);
}

QString Entry::accountId() const {
    if (d_ptr->m_account.isNull() == false) {
        return d_ptr->m_account->stringId();
    } else {
        return d_ptr->accountId;
    }
}

QString Entry::option(const QString &name) const {
    QMap<QString, QString>::const_iterator i = d_ptr->options.find(name);
    QString ret;

    if(i != d_ptr->options.end()) {
        ret = i.value();
    }

    return ret;
}

bool Entry::canReserialize () const {
    return d_ptr->m_allowSerialize;
}

bool Entry::isPending() const {
    return ((d_ptr->state == TRANSFER_STATE_PENDING) && 
            (d_ptr->failed == false));
}

bool Entry::isActive() const {
    return (d_ptr->state == TRANSFER_STATE_ACTIVE);
}

bool Entry::isSent() const {
    return (d_ptr->state == TRANSFER_STATE_DONE);
}

bool Entry::isCanceled() const {
    return (d_ptr->state == TRANSFER_STATE_CANCELLED);
}

bool Entry::hasError() const {
    return d_ptr->failed;
}

qint64 Entry::totalSize() const {
    return d_ptr->size(true);
}

qint64 Entry::unsentSize() const {
    return d_ptr->size(false);
}

QString Entry::serializedTo() const {
    return d_ptr->serialized_to;
}

int Entry::indexOf (Media * media) const {
    return d_ptr->media.indexOf (media);
}

QVectorIterator<Media *> Entry::media() const {
    QVectorIterator<Media *> iter(d_ptr->media);
    return iter;
}

unsigned int Entry::mediaCount() const {
    return d_ptr->media.size();
}

unsigned int Entry::mediaSentCount() const {
    int count = 0;
    int size = d_ptr->media.size();
    for (int i = 0; i < size ; i++) {
        Media *m = d_ptr->media [i];
        if (m->isSent() == true) {
            ++count;
        }
    }
    
    if(count >= size ) { 
        count = size ;
    }
    
    return count;
}

Media * Entry::mediaAt (int index) const {
    if (index >= d_ptr->media.size()) {
        qWarning() << "No media at index" << index;
        return 0;
    } else {
        return d_ptr->media.at (index);
    }
}

Media * Entry::nextUnsentMedia (bool includeError) const {
    Media *nextUnsentMedia = 0;
    for (int i = 0; i < d_ptr->media.size(); i++) {
        Media *m = d_ptr->media [i];

        if (m->isPending() || m->isPaused()) {
            nextUnsentMedia = m;
            break;
        } else if ((includeError == true) && m->hasError()) {
            nextUnsentMedia = m;
            break;
        }
    }

    return nextUnsentMedia;
}

void Entry::getAllTags (QList<QUrl> & commonTags, 
    QMap<QUrl, int> & partialTags) const {

    // Clear the lists first
    commonTags.clear ();
    partialTags.clear ();

    if (d_ptr->metadataFilter.testFlag (METADATA_FILTER_TAGS)) {
        // If tag filtering is enabled, do not return any tags
        return;
    }

    QMap <QUrl, int> myPartialTags;
    QVectorIterator<Media *> mediaIter = d_ptr->media;
    while (mediaIter.hasNext ()) {
        Media * media = mediaIter.next ();
        QList<QUrl> mediaTags = media->tagUrls ();

        for (int i = 0; i < mediaTags.count(); ++i) {
            if (myPartialTags.contains (mediaTags[i])) {
                ++myPartialTags [mediaTags [i]];
            } else {
                myPartialTags [mediaTags [i]] = 1;
            }
        }
    }

    const int mediaCount = d_ptr->media.count ();
    QMapIterator<QUrl, int> iter(myPartialTags);
    while (iter.hasNext ()) {
        iter.next ();
        if (iter.value () == mediaCount) {
            commonTags.append (iter.key ());
        } else {
            partialTags [iter.key ()] = iter.value();
        }
    }

    return;
}

void Entry::getAllTags (QList<QUrl> & commonTags, 
    QMap<QUrl, int> & partialTags, GeotagInfo & commonGeotag,
    QMap<GeotagInfo, int> & partialGeotags) const {

    // Clear the lists first
    commonTags.clear ();
    partialTags.clear ();
    commonGeotag.clear ();
    partialGeotags.clear ();

    if (d_ptr->metadataFilter.testFlag (METADATA_FILTER_TAGS)) {
        // If tag filtering is enabled, do not return any tags
        return;
    }

    QMap <QUrl, int> myPartialTags;
    QVectorIterator<Media *> mediaIter = d_ptr->media;
    while (mediaIter.hasNext ()) {
        Media * media = mediaIter.next ();
        QList<QUrl> mediaTags = media->tagUrls ();

        for (int i = 0; i < mediaTags.count(); ++i) {
            if (myPartialTags.contains (mediaTags[i])) {
                ++myPartialTags [mediaTags [i]];
            } else {
                myPartialTags [mediaTags [i]] = 1;
            }
        }

        GeotagInfo mediaGeotag;
        mediaGeotag = media->geotag ();
        if (!mediaGeotag.isEmpty()) {
            if (partialGeotags.contains (mediaGeotag)) {
                ++partialGeotags [mediaGeotag];
            } else {
                partialGeotags [mediaGeotag] = 1;
            }
        }
    }

    const int mediaCount = d_ptr->media.count ();
    QMapIterator<QUrl, int> iter(myPartialTags);
    while (iter.hasNext ()) {
        iter.next ();
        if (iter.value () == mediaCount) {
            commonTags.append (iter.key ());
        } else {
            partialTags [iter.key ()] = iter.value();
        }
    }

    if (partialGeotags.size() == 1) {
        QMapIterator<GeotagInfo, int> iter (partialGeotags);
        iter.next ();
        if (iter.value () == mediaCount) {
            commonGeotag = iter.key ();
            partialGeotags.clear ();
        }
    }

    return;
}

void Entry::setTags (QList<QUrl> commonTags, QList<QUrl> partialTags) {
    QVectorIterator<Media *> mediaIter = d_ptr->media;
    while (mediaIter.hasNext ()) {
        Media * media = mediaIter.next ();
        QList<QUrl> mediaTags = media->tagUrls ();

        // Remove every tag that belongs to neither commonTags list nor
        // partialTags list
        for (int i = 0; i < mediaTags.count(); ++i) {
            if (!commonTags.contains (mediaTags[i]) &&
                !partialTags.contains (mediaTags[i])) {

                media->removeTag (mediaTags [i]);
            }
        }

        // Add all the commonTags to the media. The appendTags function does
        // not allow for duplication, so no need to check for duplication here
        for (int i = 0; i < commonTags.count (); ++i) {
            media->appendTag (commonTags[i]);
        }
    }
}

void Entry::setGeotag (GeotagInfo commonGeotag, 
    QList <GeotagInfo> partialGeotags) {

    QVectorIterator<Media *> mediaIter = d_ptr->media;
    if (!commonGeotag.isEmpty ()) {
        while (mediaIter.hasNext ()) {
            Media * media = mediaIter.next ();
            media->setGeotag (commonGeotag);
        }
    } else {
        while (mediaIter.hasNext ()) {
            Media * media = mediaIter.next ();
            if (!partialGeotags.contains (media->geotag())) {
                media->clearGeotag ();
            }
        }
    }
}

QList<QUrl> Entry::commonTags () const {
    QList<QUrl> commonTags;

    // Get the list of common tags, 
    if (!(d_ptr->metadataFilter.testFlag (METADATA_FILTER_TAGS))) {

        QVectorIterator<Media *> mediaIter = d_ptr->media;
        int mediaNumber = 1;

        while (mediaIter.hasNext()) {
            Media * media = mediaIter.next();

            // First media is used as start point
            if (mediaNumber == 1) {
                commonTags = media->tagUrls();
            } else {
                // Check all current commons against next media
                for (int i = 0; i < commonTags.size(); ++i) {
                    bool found = false;

                    QList<QUrl> mediaTags = media->tagUrls();

                    // Try to find match
                    for (int j = 0; j < mediaTags.size(); ++j) {
                        if (commonTags.at (i) == mediaTags.at (j)) {
                            found = true;
                            break;
                        }
                    }

                    // Not found in other media, remove from common tags
                    if (!found) {
                        commonTags.removeAt (i);
                        --i;
                    }
                }
            }

            ++mediaNumber;
        }
    }

    return commonTags;
}

void Entry::appendTagToAllMedia (const QUrl & tag) {
    QVectorIterator<Media *> mediaIter = d_ptr->media;

    while (mediaIter.hasNext()) {
        Media * media = mediaIter.next();
        media->appendTag (tag);
    }
}

void Entry::appendTagToAllMedia (const QString & tag) {
    QSparqlQuery query ("SELECT ?tagUrl WHERE { ?tagUrl a nao:Tag; "
            "nao:prefLabel ?:tagValue . }");
    query.bindValue ("tagValue", QUrl(tag));

    QString tagUrl;
    QSparqlResult * result = d_ptr->blockingSparqlQuery (query);
    if (result == 0) {
        return;
    } else if (result->size () == 0) {
        qDebug() << query.preparedQueryText() << "had no reponse";
        delete result;

        QString uuidToUse =
            QUuid::createUuid().toString().remove('{').remove('}');
        tagUrl = QString("urn:uuid").append (uuidToUse);
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
    } else {
        result->next ();
        tagUrl = result->binding(0).value().toString();
    }

    appendTagToAllMedia (QUrl (tagUrl));
}

void Entry::removeTagFromAllMedia (const QUrl & tag) {
    QVectorIterator<Media *> mediaIter = d_ptr->media;

    while (mediaIter.hasNext()) {
        Media * media = mediaIter.next();
        media->removeTag (tag);
    }
}

void Entry::removeTagFromAllMedia (const QString & tag) {
    QVectorIterator<Media *> mediaIter = d_ptr->media;

    while (mediaIter.hasNext()) {
        Media * media = mediaIter.next();
        media->removeTag (tag);
    }
}

void Entry::getTagsFromTracker () {

    qDebug() << "PERF: Getting tags for all media: START";

    QString trackerUris;
    QMap<QString, Media*> mediaMap;
    foreach (Media *media, d_ptr->media) {
        QString origFileTrackerUri(media->origFileTrackerURI().toString());
        trackerUris.append(QString("'%1',").arg(origFileTrackerUri));
        mediaMap.insert(origFileTrackerUri, media);
    }
    trackerUris.chop(1);

    QString queryString = QString("SELECT ?ieElem ?tagUrl ?tag WHERE { "
        "?tagUrl a nao:Tag; nao:prefLabel ?tag . "
        "?ieElem a nie:InformationElement; nao:hasTag ?tagUrl . "
        "FILTER (str(?ieElem) in (%1)) }").arg(trackerUris);
    QSparqlQuery query (queryString);

    QSparqlResult * result = d_ptr->blockingSparqlQuery (query);
    if (result != 0) {
        // Query can have 0 rows as well - when there are no tags
        while (result->next ()) {
            QString ieElem(result->binding(0).value().toString());
            QString tagUrl(result->binding(1).value().toString());
            QString tag(result->binding(2).value().toString());

            Media *media = mediaMap[ieElem];
            if (media != 0) {
                media->appendTag(tagUrl, tag);
            }
        }

        delete result;
    }

    qDebug() << "PERF: Getting tags for all media: END";

    qDebug() << "PERF: Getting geotags for all media: START";
    QString geotagQueryString = QString(
        "SELECT ?ieElem ?country ?city ?district WHERE { "
        "?ieElem a nie:InformationElement . "
        "    OPTIONAL { ?ieElem slo:location ?loc . "
        "        OPTIONAL { ?loc slo:postalAddress ?pAdd . "
        "            OPTIONAL { ?pAdd nco:country ?country . } "
        "            OPTIONAL { ?pAdd nco:locality ?city . } "
        "            OPTIONAL { ?pAdd nco:region ?district . } "
        "        } "
        "    } "
        "FILTER (str(?ieElem) in (%1)) } ").arg(trackerUris);

    QSparqlQuery geotagQuery (geotagQueryString);

    result = d_ptr->blockingSparqlQuery (geotagQuery);
    if (result != 0) {
        // Query can have 0 rows as well - when there are no tags
        while (result->next ()) {
            GeotagInfo geotagInfo;
            Media *media = mediaMap[result->binding(0).value().toString()];
            if (media != 0) {
                if (!result->binding(1).value().isNull()) {
                    geotagInfo.setCountry (result->binding(1).value().toString());
                }
                if (!result->binding(2).value().isNull()) {
                    geotagInfo.setCity (result->binding(2).value().toString());
                }
                if (!result->binding(3).value().isNull()) {
                    geotagInfo.setDistrict (result->binding(3).value().toString());
                }
                media->setGeotag(geotagInfo);
            }
        }

        delete result;
    }

    qDebug() << "PERF: Getting geotags for all media: END";

}

void Entry::setImageResizeOption (ImageResizeOption resizeOption) {
    if ((resizeOption < IMAGE_RESIZE_NONE) ||
        (resizeOption >= IMAGE_RESIZE_N)) {

        qWarning() << "Invalid resize value" << resizeOption << "given to"
            << __FUNCTION__;
        d_ptr->image_resize_option = IMAGE_RESIZE_NONE;
    } else {
        qDebug() << "Setting image resize option to" << resizeOption;
        d_ptr->image_resize_option = resizeOption;
    }
}

void Entry::setVideoResizeOption (VideoResizeOption resizeOption) {
    qDebug() << "Entry::setVideoResizeOption" << resizeOption;
    if ((resizeOption < VIDEO_RESIZE_NONE) ||
        (resizeOption >= VIDEO_RESIZE_N)) {

        qWarning() << "Invalid resize value" << resizeOption << "given to"
            << __FUNCTION__;
        d_ptr->video_resize_option = VIDEO_RESIZE_NONE;
    } else {
        d_ptr->video_resize_option = resizeOption;
    }
}

void Entry::setMetadataFilter (int metadataFilter) {
    if (d_ptr->metadataFilter!=metadataFilter) {
        // options have changed
        d_ptr->metadataFilter = (MetadataFilter)metadataFilter;
        Q_EMIT (shareOptionsChanged (metadataFilter));
    }
}

int Entry::metadataFilterOption () const {
    return (int)d_ptr->metadataFilter;
}

ImageResizeOption Entry::imageResizeOption () const {
    return d_ptr->image_resize_option;
}

VideoResizeOption Entry::videoResizeOption () const {
    qDebug() << "Entry::videoResizeOption" << d_ptr->video_resize_option;
    return d_ptr->video_resize_option;
}

bool Entry::checkShareFilter (MetadataFilter metad) const {
    bool ret = d_ptr->metadataFilter.testFlag (metad);
    //qDebug() << "Entry check metadata flag" << metad << "against"
    //    << d_ptr->metadataFilter << ret;
    return ret;
}

QList<QUrl> Entry::allTrackerTypes () const {
    if (d_ptr->m_allTrackerTypes.size () == 0) {
        qDebug() << "Query tracker types for all media under transfer" <<
            d_ptr->trackerId;

        // Query will return with no duplicates
        QSparqlQuery query ("SELECT DISTINCT ?type WHERE { "
            "?:tUri a mto:Transfer ; mto:transferList ?te . ?te rdf:type . }");
        query.bindValue ("tUri", QUrl(d_ptr->trackerId));
        QSparqlResult * result = d_ptr->blockingSparqlQuery (query);
        if (result != 0) {
            while (result->next ()) {
                d_ptr->m_allTrackerTypes << 
                    QUrl(result->binding(0).value().toString());
            } 

            delete result;
        }
    }

    return d_ptr->m_allTrackerTypes;
}

bool Entry::cleanUp (const QString & path) {

    EntryPrivate data;
    if (!data.init (path, 0, false)) {
        QFile file (path);
        if (file.exists ()) {
            qWarning() << "Cleaning up unknown file type";
        }
        return false;
    }
    
    return data.removeSerialized();
}

QStringList Entry::mimeTypes () const {

    QStringList mimeList;
    QVectorIterator<Media *> mediaIter = d_ptr->media;
    QString mime;

    while (mediaIter.hasNext()) {
        Media * media = mediaIter.next();
        mime = media->mimeType();
        if (mimeList.contains (mime) == false) {
            mimeList.append (mime);
        }
    }
    
    return mimeList;
}

QDateTime Entry::created() const {
    return d_ptr->m_created;
}

/*******************************************************************************
 * Definition of functions for EntryPrivate
 ******************************************************************************/

EntryPrivate::EntryPrivate (Entry * parent) : QObject (parent),
    publicObject (parent), m_created (QDateTime::currentDateTime()),
    failed (false), state (TRANSFER_STATE_PENDING),
    image_resize_option (IMAGE_RESIZE_NONE), 
    video_resize_option (VIDEO_RESIZE_NONE), 
    metadataFilter (METADATA_FILTER_NONE), m_allowSerialize (true),
    m_sparqlConnection (0) {
    
    if (publicObject != 0) {
        QObject::connect (this, SIGNAL(stateChanged(const WebUpload::Entry*)),
            publicObject, SIGNAL(stateChanged(const WebUpload::Entry*)));
    } else {
        qDebug() << "EntryPrivate initialized without parent";
    }
}

EntryPrivate::~EntryPrivate() {
    if (m_sparqlConnection != 0) {
        delete m_sparqlConnection;
        m_sparqlConnection = 0;
    }
}

bool EntryPrivate::init(const QString &path, Entry * entry, bool demandProper,
    bool allowSerialize) {
    
    if(path.isEmpty()) {
        qCritical() << "Init path not defined";
        return false;
    }

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "File" << path << " can't be read";
        return false;
    }

    QDomDocument doc("newTransfer");
    if(!doc.setContent(&file)) {
        qWarning() << "Invalid entry init file" << path;
        file.close();
        return false;
    }
    file.close();

    serialized_to = path;

    QDomElement docElem = doc.documentElement();
    if(docElem.tagName() != "transfer") {
        qWarning() << "Wrong XML";
        return false;
    }
    
    if (demandProper) {

        trackerId = docElem.attribute("tracker", "");
        if(trackerId.isEmpty()) {
            qWarning() << "Invalid XML data (missing tracker uri)";
            serialized_to = "";
            return false;
        }

        QSparqlQuery query ("SELECT ?m ?a ?c ?s WHERE { ?:t a mto:Transfer ; "
            "mto:method ?m ; mto:account ?a ; mto:created ?c ; "
            "mto:transferState ?s . } ");
        query.bindValue ("t", QUrl (trackerId));

        QSparqlResult * result = blockingSparqlQuery (query, true);
        if (result == 0) {
            qDebug() << "Could not find transfer" << trackerId;
            trackerId.clear();
            return false;
        } else {
            result->next();
            
            if (result->binding(0).value().toUrl() != methodWeb) {
                qWarning() << "Transfer is not a web-upload. It is " << 
                    result->binding(0).value().toString();
                trackerId.clear();
                delete result;
                return false;
            }

            setAccountId (result->binding(1).value().toString());
            QVariant timeVariant = result->binding(2).value();
            if (timeVariant.isValid() && timeVariant.canConvert<QDateTime>()) {
                QDateTime timeVal = timeVariant.toDateTime();
                if (timeVal.isValid()) {
                    m_created = timeVal;
                }
            }

            state = transferStateEnum(result->binding(3).value().toString ());
            delete result;
        }
    } else {
        trackerId = "";
    }

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            if(e.tagName() == "item") {
                Media *newMedia = new Media(entry);
                if(!newMedia->initNoTrackerInfo(e)) {
                    delete newMedia;
                    qWarning() << "Could not fill a file information";
                    // Media init'ing failing means there is something gone
                    // wrong with the xml file. If this happens, we should
                    // treat it as an unrecoverable error
                    return false;
                } else {
                    Q_CHECK_PTR(newMedia);
                    media.append(newMedia);
                    connect (newMedia,
                        SIGNAL(stateChanged(const WebUpload::Media*)), this,
                        SLOT(mediaStateChanged(const WebUpload::Media*)));
                    connect (entry,
                        SIGNAL (shareOptionsChanged(int)), newMedia,
                        SLOT (shareOptionsChange(int)));
                }
                
            } else if (e.tagName() == "image") {
                QString temp = e.attribute ("resize-option", "");

                qDebug() << "resize option is " << temp;
                if (temp == "large") {
                    this->image_resize_option = IMAGE_RESIZE_LARGE;
                } else if (temp == "medium") {
                    this->image_resize_option = IMAGE_RESIZE_MEDIUM;
                } else if (temp == "small") {
                    this->image_resize_option = IMAGE_RESIZE_SMALL;
                } else if (temp == "default") {
                    this->image_resize_option = IMAGE_RESIZE_SERVICE_DEFAULT;
                } else {
                    this->image_resize_option = IMAGE_RESIZE_NONE;
                }
                
            } else if (e.tagName() == "video") {
                QString temp = e.attribute ("resize-option", "");

                qDebug() << "resize option is " << temp;
                if (temp == "vga_qvga") {
                    this->video_resize_option = VIDEO_RESIZE_VGA_QVGA;
                } else if (temp == "qvga_wqvga") {
                    this->video_resize_option = VIDEO_RESIZE_QVGA_WQVGA;
                } else {
                    this->video_resize_option = VIDEO_RESIZE_NONE;
                }
                
            } else if (e.tagName() == "filter-metadata") {
                QString temp;
                int     enabled;
                temp = e.attribute ("flags",
                    QString::number(METADATA_FILTER_AUTHOR_LOCATION, 16));
                bool ok = false;
                enabled = temp.toInt (&ok, 16);
                if (ok == true) {
                    metadataFilter = (MetadataFilter)enabled;
                } else {
                    // To be sure, let's filter personal information
                    qWarning() << "Invalid metadata filter input XML" << temp;
                    metadataFilter = METADATA_FILTER_AUTHOR_LOCATION;
                }
                
            } else if(e.tagName() == "options") {
                QDomNode n1 = e.firstChild();
                while(!n1.isNull()) {
                    QDomElement e1 = n1.toElement();
                    QString optionName = e1.attribute("id");

                    if (!(optionName.isEmpty())) {
                        QString optionValue = e1.text();
                        options.insert(optionName, optionValue);
                    }

                    n1 = n1.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }

    if (!readMediaTrackerInfo())
        return false;
    
    m_allowSerialize = allowSerialize;
    return true;
}

bool EntryPrivate::readMediaTrackerInfo() {

    qDebug() << "PERF: Reading media info from tracker: START";

    QString trackerUris;
    QMap<QString, Media*> mediaMap;
    foreach (Media *m, media) {
        QString trackerIri = m->trackerIri();
        trackerUris.append(QString("'%1',").arg(trackerIri));
        mediaMap.insert(trackerIri, m);
    }
    trackerUris.chop(1);

    // Normally mime type can be read from the xml file, and once we have the
    // xml file properly filled, we don't really require the original file name
    QString queryString = QString(
        "SELECT ?state ?startTime ?endTime ?ftUri ?fUri ?mime ?tUri WHERE {"
        "    ?tUri a mto:TransferElement. "
        "    OPTIONAL { ?tUri mto:state ?state . } "
        "    OPTIONAL { ?tUri mto:startedTime ?startTime . } "
        "    OPTIONAL { ?tUri mto:completedTime ?endTime . }"
        "    OPTIONAL {"
        "        ?tUri mto:source ?ftUri. "
        "        OPTIONAL {"
        "            ?ftUri a nie:InformationElement; "
        "                nie:mimeType ?mime . "
        "            ?ftUri a nie:DataObject; "
        "                nie:url ?fUri . "
        "        } "
        "    } "
        "   FILTER (str(?tUri) in (%1)) "
        "}").arg(trackerUris);
    QSparqlQuery query (queryString);
    QSparqlResult * result = blockingSparqlQuery (query);
    if (result == 0) {
        qCritical() << "Failed to get media info from tracker";
        return false;
    }

    qDebug() << "Number of items in sparql query result:" << result->size();
    if (result->size() != mediaMap.size()) {
        qWarning() << "Info not found for all media. Media count:" << mediaMap.size();

        delete result;
        result = 0;
        return false;
    }

    bool rv = true;

    while (result->next()) {
        QString trackerUri = result->binding(6).value().toString();
        Media *m = mediaMap[trackerUri];
        if (m == 0) {
            qWarning() << "No such tracker URI in media map:" << trackerUri;
            rv = false;
            break;
        }
        else if (!m->readTrackerInfo(result)) {
            media.remove(media.indexOf(m));
            mediaMap.remove(trackerUri);
            delete m;
            m = 0;
            qWarning() << "Could not get media info from tracker";
            rv = false;
            break;
        }
    }

    delete result;
    result = 0;

    qDebug() << "PERF: Reading media info from tracker: END";

    return rv;
}

QSparqlResult * EntryPrivate::blockingSparqlQuery (const QSparqlQuery &query,
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

bool EntryPrivate::removeSerialized () {

    if (serialized_to.isEmpty()) {
        return true;
    }

    if (m_allowSerialize) {
        qDebug() << "Checking if " << serialized_to << 
            " exists and removing it";
        // Remove entry serialization
        QFile file (serialized_to);
        if (file.exists() && !(file.remove())) {
            qCritical () << "Could not remove entry serialization" << 
                serialized_to;
            return false;
        }

        qDebug() << "Removed entry serialization" << serialized_to;
        serialized_to = "";

        // Remove file copies
        for(int i = 0; i < media.size(); ++i) {
            media[i]->removeCopyFile();
        }
    }

    return true;
}

bool EntryPrivate::serialize(const QString & path) {

    // Check that we don't serilized already serilized entry
    if (!serialized_to.isEmpty() && !path.isEmpty() && path != serialized_to) {
        qWarning() << "Entry already serialized to" << serialized_to
            << "remove it before serializing to" << path;
        return false;
    }

    if (media.size() == 0) {
        qWarning() << "Can't serialize empty entry";
        return false;
    }

    // If the transfer has been completed or canceled, delete the xml file,
    // write to transfer and return
    if((state == TRANSFER_STATE_DONE) || (state == TRANSFER_STATE_CANCELLED)) {
       return removeSerialized();
    }

    if(trackerId.isEmpty()) {
        // The entry has not been added to the tracker yet. Do that first.
        if(!addToTracker()) {
            qCritical() << "Could not enter the transfer to tracker";
            return false;
        }
    }

    if (m_allowSerialize == false) {
        qDebug() << "Not allowed to serialize to file";
        return true;
    }

    QFile file(path);
    qDebug() << "Serializing entry to" << path;

    if(!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Can't open" << path << "for writing";
        return false;
    }

    // TODO: needs better name
    QDomDocument doc("transfer");

    QDomElement entryTag = doc.createElement("transfer");
    entryTag.setAttribute("tracker", trackerId);
    entryTag.setAttribute("version", xmlVersion);
    doc.appendChild(entryTag);

    if ((image_resize_option > IMAGE_RESIZE_NONE) &&
        (image_resize_option < IMAGE_RESIZE_N)) {

        QDomElement imageResizeOption = doc.createElement ("image");
        switch (image_resize_option) {
            case IMAGE_RESIZE_LARGE:
                qDebug() << "Saving attribute large to resize-option";
                imageResizeOption.setAttribute ("resize-option", "large");
                entryTag.appendChild(imageResizeOption);
                break;
            case IMAGE_RESIZE_MEDIUM:
                qDebug() << "Saving attribute medium to resize-option";
                imageResizeOption.setAttribute ("resize-option", "medium");
                entryTag.appendChild(imageResizeOption);
                break;
            case IMAGE_RESIZE_SMALL:
                qDebug() << "Saving attribute small to resize-option";
                imageResizeOption.setAttribute ("resize-option", "small");
                entryTag.appendChild(imageResizeOption);
                break;
            case IMAGE_RESIZE_SERVICE_DEFAULT:
                qDebug() << "Saving attribute default to resize-option";
                imageResizeOption.setAttribute ("resize-option", "default");
                entryTag.appendChild(imageResizeOption);
                break;
            default:
                qDebug() << "Not writing resize-option";
                // default needs to be there, since we treat all warnings as
                // errors
                return false;
        }
    }

    if ((video_resize_option > VIDEO_RESIZE_NONE) &&
        (video_resize_option < VIDEO_RESIZE_N)) {

        QDomElement videoResizeOption = doc.createElement ("video");
        switch (video_resize_option) {
            case VIDEO_RESIZE_VGA_QVGA:
                qDebug() << "Saving attribute vga_qvga to video resize-option";
                videoResizeOption.setAttribute ("resize-option", "vga_qvga");
                entryTag.appendChild(videoResizeOption);
                break;
            case VIDEO_RESIZE_QVGA_WQVGA:
                qDebug() << "Saving attribute qvga_wqvga to resize-option";
                videoResizeOption.setAttribute ("resize-option", "qvga_wqvga");
                entryTag.appendChild(videoResizeOption);
                break;
            default:
                qDebug() << "Not writing resize-option";
                // default needs to be there, since we treat all warnings as
                // errors
                return false;
        }
    }

    QDomElement metaFilter = doc.createElement ("filter-metadata");
    // Don't use setAttribute as it used locales
    QString flagAttrb = QString::number((int)metadataFilter, 16);
    metaFilter.setAttribute ("flags", flagAttrb);

    entryTag.appendChild (metaFilter);

    for(int i = 0; i < media.size(); ++i) {
        const Media *m_ptr = media.at(i);
        QDomElement mediaTag = m_ptr->serializeToXML(doc);

        // Write the media tag
        entryTag.appendChild(mediaTag);
    }

    if(options.size() > 0) {
        QDomElement optionsTag = doc.createElement("options");

        QMap<QString, QString>::iterator i = options.begin();
        while(i != options.end()) {
             QDomElement optionTag = doc.createElement("option");
             optionTag.setAttribute("id", i.key());
             optionTag.appendChild(doc.createTextNode(i.value()));
             optionsTag.appendChild(optionTag);
             ++i;
        }

        entryTag.appendChild(optionsTag);
    }

    //qDebug() << "Writing to xml file :" << doc.toByteArray(2);
    file.write(doc.toByteArray(2));
    file.close();
    serialized_to = path;

    return true;
}


qint64 EntryPrivate::size (bool calc_sent) const {
    qint64 size = 0;
    for(int i = 0; i < media.size(); ++i) {
        qint64 media_size = media.at(i)->fileSize();
        if(media_size > 0 &&(calc_sent || !(media.at(i)->isSent()))) {
            size += media_size;
        }
    }

    return size;
}

bool EntryPrivate::addToTracker(void) {
    trackerId = "mtransfer://";
    trackerId.append(QUuid::createUuid().toString().remove('{').remove('}'));

    QString insertString ("INSERT { ?:t a mto:Transfer ; mto:method ?:method ;"
        " mto:created ?:created ; mto:account ?:account ; "
        "mto:transferState ?:state ");

    QString account;
    if (m_account.isNull() == false) {
        account = m_account->stringId();
    } else {
        account = accountId;    
    }
    
    QUrl stateUrl;
    if (state == TRANSFER_STATE_ACTIVE) {
        stateUrl = transferStateIri(TRANSFER_STATE_PENDING);
    } else {
        stateUrl = transferStateIri (state);
    }

    for(int i = 0; i < media.size(); ++i) {
        QUrl mediaURI = media[i]->addToTracker();
        insertString.append ("; mto:transferList <").
            append (mediaURI.toString()).append ("> ");
    }

    insertString.append (". } ");

    QSparqlQuery addQuery (insertString, QSparqlQuery::InsertStatement);
    addQuery.bindValue ("t", QUrl (trackerId));
    addQuery.bindValue ("method", methodWeb);
    addQuery.bindValue ("created", m_created.toString (Qt::ISODate));
    addQuery.bindValue ("account", account);
    addQuery.bindValue ("state", stateUrl);

    QSparqlResult * result = blockingSparqlQuery (addQuery);
    if (result == 0) {
        qDebug() << "Could not add new mto:Transfer with uri" << trackerId
            << "using query" << addQuery.preparedQueryText ();
        return false;
    }

    delete result;
    return true;
}

void EntryPrivate::syncPostOptions () {

    if (m_account.isNull()) {
        qCritical() << "Can't sync without account";
        return;
    }

    if (publicObject != 0) {
        publicObject->setMetadataFilter(METADATA_FILTER_NONE);
    }

    QListIterator<PostOption *> options = m_account->service()->postOptions();
    while (options.hasNext ()) {
        PostOption * option = options.next ();
        switch (option->type ()) {
            case PostOption::OPTION_TYPE_TITLE:
            case PostOption::OPTION_TYPE_DESC:
            case PostOption::OPTION_TYPE_TAGS:
                break;

            case PostOption::OPTION_TYPE_METADATA:
            {
                CommonListOption * opt = 
                    qobject_cast<CommonListOption *>( option);
                if (publicObject != 0 && opt != 0) {
                    publicObject->setMetadataFilter (
                        publicObject->metadataFilterOption() | opt->currentValue());
                }
                break;
            }

            case PostOption::OPTION_TYPE_FACE_TAGS:
            {
                CommonSwitchOption *opt =
                    qobject_cast<CommonSwitchOption *>(option);
                if (publicObject != 0 && opt != 0) {
                    if (!opt->isChecked()) {
                        publicObject->setMetadataFilter(
                            publicObject->metadataFilterOption() | METADATA_FILTER_REGIONS);
                    }
                }
                break;
            }

            case PostOption::OPTION_TYPE_IMAGE_RESIZE:
            {
                CommonListOption * opt = 
                    qobject_cast<CommonListOption *>( option);
                if (publicObject != 0 && opt != 0) {
                    publicObject->setImageResizeOption (
                        (ImageResizeOption)opt->currentValue());
                }
                break;
            }

            case PostOption::OPTION_TYPE_VIDEO_RESIZE:
            {
                CommonListOption * opt = 
                    qobject_cast<CommonListOption *> (option);
                if (publicObject != 0 && opt != 0) {
                    publicObject->setVideoResizeOption (
                        (VideoResizeOption)opt->currentValue());
                }
                break;
            }

            case PostOption::OPTION_TYPE_SERVICE:
            {
                ServiceOption * opt = 
                    qobject_cast<ServiceOption *> (option);
                if (opt) {
                    setOption (opt->id(), opt->activeValueId());
                }
            }

            default:
                break;
        }
    }
}

WebUpload::Account * EntryPrivate::loadAccount (QObject * parent) const {
    WebUpload::Account * account = 0;

    WebUpload::System system;
    account = system.account (accountId, parent);

    return account;
}

void EntryPrivate::setOption (const QString & id, const QString & value) {
    options.insert (id, value);
}

void EntryPrivate::setAccountId (const QString & id) {
    if (accountId == id) {
        qDebug() << "Account id is already" << id;
        return;
    }

    accountId = id;
    qDebug() << "Setting account id to" << id;
    if (m_account.isNull() == false && m_account->stringId() != id) {
        qWarning() << "Replacing already loaded account in entry";
    }

    WebUpload::System system;
    m_account = system.sharedAccount (id);
}

void EntryPrivate::mediaStateChanged (const Media *changedMedia) {
    const Media *media = changedMedia;
    if (sender() != 0) {
        media = qobject_cast<Media *>(sender());
    }

    if (media == 0) {
        qWarning() << "Cannot get media whose state changed";
        return;
    }

    bool changeState = false;
    if (media->isPending()) {
        // This will happen when the error has been cleared. 
        if (failed == true) {
            qWarning() << "Error clearing should always happen through"
                "WebUpload::Entry::clearFailed() function";
            if (publicObject != 0) {
                publicObject->clearFailed();
            }
        }
    } else if (media->isActive()) {
        if (state != TRANSFER_STATE_ACTIVE) {
            state = TRANSFER_STATE_ACTIVE;
            changeState = true;
        }
    } else if (media->isCanceled()) {
        if (state != TRANSFER_STATE_CANCELLED) {
            qWarning() << "Cancel should always happen through"
                "WebUpload::Entry::cancel() function";
            if (publicObject != 0) {
                publicObject->cancel();
            }
        }
    } else if (media->isSent() && publicObject != 0) {
        // Set transfer as done if there are no paused, pending or error
        // transfer elements left
        if (publicObject->nextUnsentMedia (true) == 0) {
            state = TRANSFER_STATE_DONE;
            changeState = true;
            changeState = true;
        } else if (publicObject->nextUnsentMedia() == 0) {
            // The only unsent media left are those with errors. So, can move
            // back to pending state. 
            state = TRANSFER_STATE_PENDING;
            changeState = true;
        }
    } else if (media->isPaused()) {
        if (state != TRANSFER_STATE_PAUSED) {
            state = TRANSFER_STATE_PAUSED;
            changeState = true;
        }
    } else if (media->hasError()) {
        failed = true;
    }

    if (changeState == true) {
        QSparqlQuery delQuery ("DELETE { ?:t mto:transferState ?state . } "
            "WHERE { ?:t mto:transferState ?state . }",
            QSparqlQuery::DeleteStatement);
        delQuery.bindValue ("t", QUrl (trackerId));

        QSparqlResult *result = blockingSparqlQuery (delQuery);
        if (result == 0) {
            qDebug() << "Delete query" << delQuery.preparedQueryText() << 
                "failed. Trying insert anyways ...";
        } else {
            delete result;
        }

        QSparqlQuery insQuery ("INSERT { ?:t mto:transferState ?:state . }",
            QSparqlQuery::InsertStatement);
        insQuery.bindValue ("t", QUrl (trackerId));
        if (state == TRANSFER_STATE_ACTIVE) {
            insQuery.bindValue ("state", transferStateIri (TRANSFER_STATE_PENDING));
        } else {
            insQuery.bindValue ("state", transferStateIri (state));
        }

        result = blockingSparqlQuery (insQuery);
        if (result == 0) {
            qWarning() << "Attempt to change state using" <<
                insQuery.preparedQueryText() << "failed";
            return;
        } 
        delete result;

        Q_EMIT (stateChanged(publicObject));
    }
}
