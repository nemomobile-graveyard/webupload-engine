 
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

#include <QDebug>
#include "xmlhelper.h"
#include "WebUpload/CommonListOption"
#include "commonlistoptionprivate.h"
#include "WebUpload/enums.h"
#include "WebUpload/Account"

using namespace WebUpload;

CommonListOption::CommonListOption (QObject * parent) : CommonOption (parent),
    d_ptr (new CommonListOptionPrivate (this)) {

}

CommonListOption::~CommonListOption () {

    delete d_ptr;
}

bool CommonListOption::init (QDomElement & element) {

    if (!CommonOption::init (element)) {
        return false;
    }

    bool initDone;
    switch (type()) {
        case PostOption::OPTION_TYPE_METADATA:
            initDone = d_ptr->initMetadata ();
            break;

        case PostOption::OPTION_TYPE_IMAGE_RESIZE:
        {
            initDone = d_ptr->initImageResize ();

            QStringList mimeList;
            mimeList << "image/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        case PostOption::OPTION_TYPE_VIDEO_RESIZE:
        {
            initDone = d_ptr->initVideoResize ();

            QStringList mimeList;
            mimeList << "video/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        default:
            initDone = false;
    }

    return initDone;
}

bool CommonListOption::init (PostOption::Type optionType, QString caption) {

    if (CommonOption::init (optionType, caption) == false) {
        return false;
    }

    bool initDone;
    switch (optionType) {
        case PostOption::OPTION_TYPE_METADATA:
            initDone = d_ptr->initMetadata ();
            break;

        case PostOption::OPTION_TYPE_IMAGE_RESIZE:
        {
            initDone = d_ptr->initImageResize ();

            QStringList mimeList;
            mimeList << "image/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        case PostOption::OPTION_TYPE_VIDEO_RESIZE:
        {
            initDone = d_ptr->initVideoResize ();

            QStringList mimeList;
            mimeList << "video/*";
            setSupportedMimeTypes (mimeList);

            break;
        }

        default:
            initDone = false;
    }

    return initDone;
}


QStringList CommonListOption::valueList () const {

    return d_ptr->valueList ();
}

int CommonListOption::currentValue () const {
    
    if (d_ptr->m_valueList.isEmpty ()) {
        return -1;
    }

    return d_ptr->m_valueList[d_ptr->m_currentValueIndex]->value;
}


int CommonListOption::currentValueIndex () const {

    return d_ptr->m_currentValueIndex;
}

bool CommonListOption::setCurrentValueIndex (int index) {

    if (d_ptr->m_valueList.isEmpty ()) {
        return false;
    }
    
    if (index < 0 || index > d_ptr->m_valueList.size ()) {
        return false;
    }

    d_ptr->m_currentValueIndex = index;
    switch (type ()) {
        case PostOption::OPTION_TYPE_METADATA:
            account()->setMetadataFilters (currentValue ());
            break;

        case PostOption::OPTION_TYPE_IMAGE_RESIZE:
            account()->setImageResizeOption
                ((ImageResizeOption)currentValue());
            break;

        case PostOption::OPTION_TYPE_VIDEO_RESIZE:
            account()->setVideoResizeOption
                ((VideoResizeOption)currentValue());
            break;
            
        default:
            break;
    }

    return true;
}


/* --- Private functions ---------------------------------------------------- */

CommonListOptionPrivate::CommonListOptionPrivate (CommonListOption * parent) :
    m_parent (parent), m_currentValueIndex (-1) {

}

CommonListOptionPrivate::~CommonListOptionPrivate () {

    while (!m_valueList.isEmpty ()) {
        struct OptionValue * val = m_valueList.takeFirst ();
        delete val;
    }
}


bool CommonListOptionPrivate::initMetadata () {

    if (!m_valueList.isEmpty ()) {
        qWarning() << "Option already been initialized earlier";
        return false;
    }

    if (m_parent->account () == 0) {
        qDebug() << "Account not defined";
        return false;
    }

    struct OptionValue * opt;

    m_currentValueIndex = 0;

    int defVal = m_parent->account()->metadataFilters ();
    opt = new OptionValue ();
    //% "Keep all"
    opt->valueId = qtTrId ("qtn_tui_share_keep_all");
    opt->value = (int)WebUpload::METADATA_FILTER_NONE;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 0;
    }

    opt = new OptionValue ();
    //% "Remove GPS coordinates and creator name"
    opt->valueId = qtTrId ("qtn_tui_share_remove_auth_gps");
    opt->value = (int)WebUpload::METADATA_FILTER_AUTHOR_LOCATION;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 1;
    }

    return true;
}

bool CommonListOptionPrivate::initImageResize () {

    if (!m_valueList.isEmpty ()) {
        qWarning() << "Option already been initialized earlier";
        return false;
    }

    if (m_parent->account () == 0) {
        qDebug() << "Account not defined";
        return false;
    }

    m_currentValueIndex = 0;
    int defVal = m_parent->account()->imageResizeOption ();

    OptionValue * opt;
    opt = new OptionValue ();
    //% "Original"
    opt->valueId = qtTrId ("qtn_tui_share_size1");
    opt->value = (int)WebUpload::IMAGE_RESIZE_NONE;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 0;
    }

    opt = new OptionValue ();
    //% "Large (%L1 px)"
    opt->valueId = qtTrId ("qtn_tui_share_size4").arg (1920);
    opt->value = (int)WebUpload::IMAGE_RESIZE_LARGE;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 1;
    }

    opt = new OptionValue ();
    //% "Medium (%L1 px)"
    opt->valueId = qtTrId ("qtn_tui_share_size2").arg (1280);
    opt->value = (int)WebUpload::IMAGE_RESIZE_MEDIUM;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 2;
    }

    opt = new OptionValue ();
    //% "Small (%L1 px)"
    opt->valueId = qtTrId ("qtn_tui_share_size3").arg (640);
    opt->value = (int)WebUpload::IMAGE_RESIZE_SMALL;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 3;
    }

    return true;
}

bool CommonListOptionPrivate::initVideoResize () {

    if (!m_valueList.isEmpty ()) {
        qWarning() << "Option already been initialized earlier";
        return false;
    }

    if (m_parent->account () == 0) {
        qDebug() << "Account not defined";
        return false;
    }

    m_currentValueIndex = 0;
    int defVal = m_parent->account()->videoResizeOption ();

    OptionValue * opt;
    opt = new OptionValue ();
    //% "Original"
    opt->valueId = qtTrId ("qtn_tui_share_video_size1");
    opt->value = (int)WebUpload::VIDEO_RESIZE_NONE;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 0;
    }

    opt = new OptionValue ();
    //% "Medium (480 px)"
    opt->valueId = qtTrId ("qtn_tui_share_video_size2");
    opt->value = (int)WebUpload::VIDEO_RESIZE_VGA_QVGA;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 2;
    }

    opt = new OptionValue ();
    //% "Small (240 px)"
    opt->valueId = qtTrId ("qtn_tui_share_video_size3");
    opt->value = (int)WebUpload::VIDEO_RESIZE_QVGA_WQVGA;
    m_valueList << opt;
    if (defVal == opt->value) {
        m_currentValueIndex = 2;
    }

    return true;
}

QStringList CommonListOptionPrivate::valueList () const {

    QStringList ret;
    for (int i = 0; i < m_valueList.size(); ++i) {
        // Translation should already have been done at initialization in the
        // various init functions
        ret << m_valueList[i]->valueId;
    }

    return ret;
}
