
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

#ifndef _PUBLISH_WIDGETS_FORM_LOADER_PRIVATE_H_
#define _PUBLISH_WIDGETS_FORM_LOADER_PRIVATE_H_

#include <QObject>
#include <QList>
#include <QPluginLoader>

namespace PublishWidgets {
    class FormLoader;
    
    /*!
      \class FormLoaderPrivate
      \brief Private class for FormLoader
     */
    class FormLoaderPrivate : public QObject {
    Q_OBJECT
    public:
        FormLoaderPrivate (FormLoader * parent);
        virtual ~FormLoaderPrivate();
        
        FormLoader * m_publicObject; //!< parent/public object
        
        QList <QPluginLoader*> m_loaders; //!< Loaders
    
    };
}

#endif
