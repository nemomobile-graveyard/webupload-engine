 
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

#include "WebUpload/ServiceOptionValue"
#include "serviceoptionvalueprivate.h"
#include <QDebug>

using namespace WebUpload;

const QChar ServiceOptionValue::FIELD_SEPARATOR(QChar::LineSeparator);

ServiceOptionValue::ServiceOptionValue (const QString & id,
    const QString & name) : d_ptr (new ServiceOptionValuePrivate (id, name)) {
    
}

ServiceOptionValue::ServiceOptionValue () :
    d_ptr (new ServiceOptionValuePrivate ()) {
}

ServiceOptionValue::ServiceOptionValue (const ServiceOptionValue & src) :
    d_ptr (new ServiceOptionValuePrivate ()) {
    
    d_ptr->id = src.id();
    d_ptr->name = src.name();
}

ServiceOptionValue & ServiceOptionValue::operator = (
    const ServiceOptionValue & src) {

    if(this != &src) {
        d_ptr->id = src.id();
        d_ptr->name = src.name();    
    }

    return *this;
}


ServiceOptionValue::~ServiceOptionValue () {
    delete d_ptr;
}

const QString & ServiceOptionValue::id() const {
    return d_ptr->id;
}

const QString & ServiceOptionValue::name() const {
    return d_ptr->name;
}

void ServiceOptionValue::setId (const QString & id) {
    d_ptr->id = id;
}

void ServiceOptionValue::setName (const QString & name) {
    d_ptr->name = name;
}

bool ServiceOptionValue::isEmpty() const {
    return (id().isEmpty() || name().isEmpty());
}

// -- Private class ----------------------------------------------------------
ServiceOptionValuePrivate::ServiceOptionValuePrivate (const QString & iId,
    const QString & iName) : id(iId), name(iName) {
}
