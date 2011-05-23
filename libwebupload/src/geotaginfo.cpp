
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


#include "WebUpload/geotaginfo.h"
using namespace WebUpload;

namespace WebUpload {

    class GeotagInfoPrivate {
    public:
        GeotagInfoPrivate ();
        GeotagInfoPrivate (QString country, QString city, QString district);

        QString m_country;
        QString m_city;
        QString m_district;
    };
}


GeotagInfo::GeotagInfo (QObject * parent, QString country, QString city,
    QString district): QObject (parent), 
    d_ptr (new GeotagInfoPrivate (country, city, district)) {

}

GeotagInfo::GeotagInfo (const GeotagInfo &src) : QObject (0), 
    d_ptr (new GeotagInfoPrivate ()) {

    d_ptr->m_country = src.country ();
    d_ptr->m_city = src.city ();
    d_ptr->m_district = src.district ();
}

GeotagInfo & GeotagInfo::operator= (const GeotagInfo &other) {
    d_ptr->m_country = other.country ();
    d_ptr->m_city = other.city ();
    d_ptr->m_district = other.district ();

    return *this;
}

bool GeotagInfo::operator== (const GeotagInfo &other) const {
    return ((d_ptr->m_country == other.country ()) && 
            (d_ptr->m_city == other.city ()) && 
            (d_ptr->m_district == other.district ()));
}


GeotagInfo::~GeotagInfo () {
    delete d_ptr;
}

const QString & GeotagInfo::country () const {
    return d_ptr->m_country;
}

const QString & GeotagInfo::city () const {
    return d_ptr->m_city;
}

const QString & GeotagInfo::district () const {
    return d_ptr->m_district;
}

void GeotagInfo::setCountry (const QString &country) {
    d_ptr->m_country = country;
}

void GeotagInfo::setCity (const QString &city) {
    d_ptr->m_city = city;
}

void GeotagInfo::setDistrict (const QString &district) {
    d_ptr->m_district = district;
}

bool GeotagInfo::isEmpty () const {
    return (d_ptr->m_country.isEmpty() && d_ptr->m_city.isEmpty() && 
            d_ptr->m_district.isEmpty());
}

void GeotagInfo::clear () {
    d_ptr->m_country.clear ();
    d_ptr->m_city.clear ();
    d_ptr->m_district.clear ();
}


///------------ Private class constructors


GeotagInfoPrivate::GeotagInfoPrivate () {
}

GeotagInfoPrivate::GeotagInfoPrivate (QString country, QString city,
    QString district) : m_country (country), m_city (city),
    m_district (district) {

}

