 
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

#include "WebUpload/PostOption"
#include "postoptionprivate.h"
#include "WebUpload/Account"
#include "WebUpload/Service"
#include <QDomDocument>
#include <QDebug>
#include "xmlhelper.h"
#include <limits.h>

using namespace WebUpload;

PostOption::PostOption(QObject * parent) : QObject (parent),
    d_ptr (new PostOptionPrivate(this)) {
}

PostOption::~PostOption() {
    delete d_ptr;
}

bool PostOption::init (enum PostOption::Type optionType, QString caption) {
    d_ptr->m_optionType = optionType;
    d_ptr->m_caption = caption;

    return true;
}

QString PostOption::caption () const {
    return d_ptr->m_caption;
}

enum PostOption::Type PostOption::type () const {
    return d_ptr->m_optionType;
}

int PostOption::order () const {
    return d_ptr->m_order;
}

bool PostOption::validForMimeType (const QString & mimeType) const {
    return XmlHelper::mimeTypeListCheck (mimeType, d_ptr->m_mimeTypes);
}

Account * PostOption::account () const {
    return d_ptr->m_account;
}

void PostOption::setType (enum PostOption::Type type) {
    d_ptr->m_optionType = type;
}

void PostOption::setCaption (const QString caption) {
    d_ptr->m_caption = caption;
}

void PostOption::setSupportedMimeTypes (const QStringList mimeList) {
    d_ptr->m_mimeTypes = mimeList;
}

void PostOption::setOrder (int order) {
    d_ptr->m_order = order;
}


/* --- Private class functions ---------------------------------------------- */
PostOptionPrivate::PostOptionPrivate (PostOption * parent) : m_parent (parent),
    m_order (INT_MAX), m_optionType (PostOption::OPTION_TYPE_N), 
    m_account (0) {
    
    if (parent != 0) {
        Service * service = qobject_cast<Service *>(parent->parent());
        if (service != 0) {
            m_account = service->account();
        }
    }
}

PostOptionPrivate::~PostOptionPrivate () {
}