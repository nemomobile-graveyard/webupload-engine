 
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

#ifndef _WEBUPLOAD_HTTP_MULTI_CONTENT_IO_H_
#define _WEBUPLOAD_HTTP_MULTI_CONTENT_IO_H_

#include <WebUpload/export.h>
#include <QIODevice>
#include <QString>
#include <QStringList>

namespace WebUpload {

    /*!
      \class HttpMultiContentIO
      \brief This class provides an abstraction over QIODevice to create a
             payload of various kinds of contents to be sent over HTTP

             The QIODevices added to this class are always opened in
             QIODevice::ReadOnly mode - writing is not supported. The QIODevice
             represented by this class is opened in the Write mode only when
             adding new QIODevices to its list.
      \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */

    /* Forward declarations required */
    class HttpMultiContentIOPrivate;

    class WEBUPLOAD_EXPORT HttpMultiContentIO : public QIODevice {

        Q_OBJECT
    
    public:
        /*!
         * \brief Constructor
         * \param parent QObject parent
         */
        HttpMultiContentIO(QObject *parent = 0);
        
        /*!
         * \brief Destructor
         */
        virtual ~HttpMultiContentIO();

        /*!
           \brief This function sets the default template which is used to
                  construct the strings are added to the payload to be sent. 
           \param tmplt The template which needs to be set
           \return Boolean value signifying success or failure
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::createFormDataRequest()
         * \until }
         * \sa defaultTemplate clearDefaultTemplate
         */
        bool setDefaultTemplate(const QString &tmplt);

        /*!
           \brief Method to get the default template set using the
                  setDefaultTemplate method.
           \return Constant reference to the default template string
           \sa setDefaultTemplate clearDefaultTemplate
         */        
        QString defaultTemplate() const;

        /*!
           \brief Clears the string stored as default template
           \sa setDefaultTemplate defaultTemplate
         */
        void clearDefaultTemplate();
        
        /*!
          \brief Clears all data written to HttpMultiContentIO
         */
        void clear();

        /*!
           \brief  This function sets the boundary string that is used in
                   creating the payload. By default a boundary string is
                   created in the constructor of this class. This function is
                   provided in case the user wants to over-ride the generated
                   boundary string with some fixed defined string.
           \param  bdry: The boundary string
           \return Boolean value signifying success or failure
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::setBoundaryString()
         * \until }
         * \sa boundaryString addString
         */
        bool setBoundaryString(const QString &bdry);
        
        /*!
         * \brief Method to get current boundary string used in the payload
         * \return Constant reference to the boundary string
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::getDefaultBoundaryString()
         * \until }
         * \sa setBoundaryString
         */          
        QString boundaryString() const;

        /*!
         * \brief Convenience function provided to add free format strings to
         *        the payload to be sent.
         * \param string String to be added. Will be converted to UTF-8.
         * \param prefixBoundary A boolean value of true if the boundary has
         *                       to be added before the string
         * \return Boolean value signifying success or failure
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::addStringWithBoundary()
         * \until }
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::addStringWithoutBoundary()
         * \until }
         * \sa setBoundaryString boundaryString
         */
        bool addString(const QString &string, bool prefixBoundary);

        /*!
         * \brief  This function adds a string to the payload. It takes as
         *         arguments a list of strings that act as arguments to the
         *         template, the string that needs to be sent, and optionally a
         *         string to over-ride the default template. 
         * \param args A stringlist containing the parameters for the
         *             template
         * \param string String to be added to the payload
         * \param tmplt Optional string parameter to over-ride the default
         *              template string.
         * \return Boolean value signifying success or failure
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::createAtomXmlRequest()
         * \until }
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::createFormDataRequest()
         * \until }
         */
        bool addString(const QStringList &args, const QString &string, 
                        const QString &tmplt = "");

        /*!
         *  \brief  This function adds the contents of a file to the payload. It
         *          takes as arguments a list of strings that act as arguments
         *          to the template, the string that needs to be sent, and
         *          optionally a string to over-ride the default template. 
         * \param args The list of arguments for the template
         * \param filePath Path to file whose content need to be sent
         * \param tmplt Optional string parameter to over-ride the default
         *              template string.
         * \return Boolean value signifying success or failure
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::uploadPhoto()
         * \until }
         */
        bool addFile(const QStringList &args, const QString &filePath,
            const QString &tmplt = QString());

        /*!
           \brief  Function to tell the IODevice that all information required
                   for upload has been added and no more data will be added. No
                   reads are allowed till this function is called, and once
                   this function is called, no more information can be added to
                   the payload.
         * \n \dontinclude exampleio.cpp
         * \skipline ServiceUpload::uploadContentToService()
         * \until }
         */
        void allDataAdded();


        /*! \reimp */
        bool open(OpenMode mode);
        void close();
        bool isSequential () const;
        bool seek (qint64 pos);
        qint64 pos () const;
        qint64 size () const;
        bool atEnd () const;
        /*! \reimp_end */
        
    protected:

        /*! \reimp */
        qint64 readData(char *data, qint64 maxlen);
        qint64 writeData(const char *data, qint64 len);
        /*! \reimp_end */


    private:
        HttpMultiContentIOPrivate * const d_ptr;
    };
}

#endif
