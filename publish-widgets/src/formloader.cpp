
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
 
#include <PublishWidgets/FormLoader>
#include "formloaderprivate.h"
#include <QPluginLoader>
#include <QDebug>
#include <QFile>

using namespace PublishWidgets;

FormLoader::FormLoader (QObject * parent) : QObject (parent),
    d_ptr (new FormLoaderPrivate (this)) {
    
    qDebug() << "FormLoader created" << this << d_ptr;

}

FormLoader::~FormLoader() {
    delete d_ptr;
}

FormWidgetInterface * FormLoader::formWidget (const QString & pluginName,
    WebUpload::Entry * entry) {
    
    PublishWidgets::FormWidgetInterface * ret = 0;    
    
    QString pluginPath = QLatin1String (
        "/usr/lib/webupload-engine/publish-plugins/lib");
    pluginPath.append (pluginName);
    pluginPath.append (".so");

    qDebug() << "Load form from" << pluginPath;

    if (QFile::exists (pluginPath) == false) {
        qCritical() << "Failed to find form plugin:" << pluginPath;
        return 0;
    }
    
    QPluginLoader * loader = new QPluginLoader ();
    d_ptr->m_loaders.append (loader);
    qDebug() << "Loading form implementation from" << pluginPath;
    loader->setFileName (pluginPath);
    loader->setLoadHints (QLibrary::ExportExternalSymbolsHint);
    
    if (loader->load() == true) {
        
        QObject * obj = loader->instance();
        qDebug() << "Received object" << obj;
        
        if (obj != 0) {
            ret = dynamic_cast <PublishWidgets::FormWidgetInterface *> (obj);
            qDebug() << "dynamic casting" << ret;
            if (ret == 0) {
                qWarning() << "Casting loader instance to"
                    "PublishWidgets::FormWidgetInterface resulted in NULL ptr";
                return 0;
            }
            ret->setEntry (entry);
        } else {
            qWarning() << "Did not receive QObject instance from plugin";
        }
    } else {
        qWarning() << "Failed to load plugin:" << loader->errorString();
    }    
    
    return ret;
}

// -- private class ------------------------------------------------------------

FormLoaderPrivate::FormLoaderPrivate (FormLoader * parent) : QObject (parent),
    m_publicObject (parent) {
 
}

FormLoaderPrivate::~FormLoaderPrivate () {
    for (int i = 0; i < m_loaders.count(); ++i) {
        delete m_loaders.at(i);
    }
}