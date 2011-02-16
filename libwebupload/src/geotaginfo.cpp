
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


#include "WebUpload/geotaginfo.h"
using namespace WebUpload;


GeotagInfo::GeotagInfo (QObject * parent, QString country, QString city,
    QString district): QObject (parent), m_country (country), m_city (city),
    m_district (district) {

}

GeotagInfo::GeotagInfo (const GeotagInfo &src) : QObject (0) {
    m_country = src.country ();
    m_city = src.city ();
    m_district = src.district ();
}

GeotagInfo & GeotagInfo::operator= (const GeotagInfo &other) {
    m_country = other.country ();
    m_city = other.city ();
    m_district = other.district ();

    return *this;
}

bool GeotagInfo::operator== (const GeotagInfo &other) const {
    return ((m_country == other.country ()) && (m_city == other.city ()) && 
            (m_district == other.district ()));
}


GeotagInfo::~GeotagInfo () {
}

const QString & GeotagInfo::country () const {
    return m_country;
}

const QString & GeotagInfo::city () const {
    return m_city;
}

const QString & GeotagInfo::district () const {
    return m_district;
}

void GeotagInfo::setCountry (const QString &country) {
    m_country = country;
}

void GeotagInfo::setCity (const QString &city) {
    m_city = city;
}

void GeotagInfo::setDistrict (const QString &district) {
    m_district = district;
}

bool GeotagInfo::isEmpty () const {
    return (m_country.isEmpty() && m_city.isEmpty() && m_district.isEmpty());
}

void GeotagInfo::clear () {
    m_country.clear ();
    m_city.clear ();
    m_district.clear ();
}


