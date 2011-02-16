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

#ifndef WEBUPLOAD_GEOTAG_INFO_H_
#define WEBUPLOAD_GEOTAG_INFO_H_

#include <WebUpload/export.h>
#include <QString>
#include <QObject>

namespace WebUpload {

    /*!
      \class GeotagInfo
      \brief Class storing geotag information
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT GeotagInfo : public QObject {

        Q_OBJECT

    public: 

        /*!
          \brief Alternate constructor
          \param parent
          \param country String with country values for this geotag
          \param city String with city value for this geotag
          \param district String with district value for this geotag
         */
        GeotagInfo (QObject *parent = 0, QString country = QString (),
            QString city = QString (), QString district = QString ());

        /*!
          \brief Copy constructor
         */
        GeotagInfo (const GeotagInfo &src);

        /*!
          \brief Assignment operator
         */
        GeotagInfo & operator= (const GeotagInfo &other);

        /*!
          \brief Comparison operator
         */
        bool operator== (const GeotagInfo &other) const;

        /*!
          \brief Destructor
         */
        virtual ~GeotagInfo ();

        /*!
          \brief Get the country represented by this geotag
          \return Country if it is set, else an empty string.
         */
        const QString & country () const;

        /*!
          \brief Get the city represented by this geotag
          \return City if it is set, else an empty string.
         */
        const QString & city () const;

        /*!
          \brief Get the district represented by this geotag
          \return District if it is set, else an empty string.
         */
        const QString & district () const;

        /*!
          \brief Set the country for the geotag
          \param country String containing the name of the country for the
                    geotag. If it is an empty string, any country already
                    represented by the geotag would be cleared.
         */
        void setCountry (const QString &country);

        /*!
          \brief Set the city for the geotag
          \param city String containing the name of the city for the
                    geotag. If it is an empty string, any city already
                    represented by the geotag would be cleared.
         */
        void setCity (const QString &city);

        /*!
          \brief Set the district for the geotag
          \param district String containing the name of the district for the
                    geotag. If it is an empty string, any district already
                    represented by the geotag would be cleared.
         */
        void setDistrict (const QString &district);

        /*!
          \brief Check if the geotag represents anything at all
          \return <code>true</code> if none of country, city or district are
                    set in the geotag, else <code>false</code>
         */
        bool isEmpty () const;

        /*!
          \brief Clear any geotag information represented by this instance. 
                 After a call to this function, \sa isEmpty will definitely
                 return <code>true</code>
         */
        void clear ();

    private:
        QString m_country;
        QString m_city;
        QString m_district;
    };
}

inline bool operator< (const WebUpload::GeotagInfo &e1, 
    const WebUpload::GeotagInfo &e2) {

    Q_UNUSED (e1)
    Q_UNUSED (e2)

    return false;
    if (e1.country () == e2.country ()) {
        if (e1.city () == e2.city ()) {
            if (e1.district () == e2.district ()) {
                return false;
            } else {
                return e1.district () < e2.district ();
            } 
        } else {
            return e1.city () < e2.city ();
        } 
    } else {
        return e1.country() < e2.country ();
    }
}

#endif // WEBUPLOAD_GEOTAG_INFO_H_
