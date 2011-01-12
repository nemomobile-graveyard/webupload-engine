 
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

#include "xmlhelper.h"
#include <QDebug>
#include <QRegExp>

using namespace WebUpload;

XmlHelper::XmlHelper (QObject * parent) : QObject (parent) {

}

XmlHelper::~XmlHelper () {
}

QString XmlHelper::getLocalizatedContent (const QDomElement & element) {

    QString plain = element.text();

    QString key = element.attribute ("key");    
    if (!key.isEmpty()) {    
        
        QString val = qtTrId (key.toAscii().constData());
        
        //Way to check that we didn't get this value from the catalogs
        if (val == key) {
            val = plain;
        }
        
        return val;
        
    } else {
        return plain;
    }
}

bool XmlHelper::attributeValueToBool (QDomElement & elem,
    const QString & attribute, bool defValue) {
    
    QString str = elem.attribute (attribute, "");
    
    if (str.isEmpty()) {
        return defValue;
    } else if (str == "1" || str == "yes" || str == "true") {
        return true;
    } else if (str == "0" || str == "no" || str == "false") {
        return false;
    } else {
        return defValue;
    }
}

QStringList XmlHelper::readMimeTypes (const QDomElement & elem) {
    
    QStringList retList;
    QDomNode mimeNode = elem.firstChild();
        
    while (!mimeNode.isNull()) {
    
        if (mimeNode.isElement() == false) {
            continue;
        }
    
        QDomElement mimeElem = mimeNode.toElement();

        if (mimeElem.tagName() == "mime") {
            QString mime = mimeElem.text();
            if (!mime.isEmpty()) {
                retList.append (mime);
            }
        }

        mimeNode = mimeNode.nextSibling();
    } 
    
    return retList;
}

bool XmlHelper::mimeTypeListCheck (const QString & mimeType,
    const QStringList & mimeTypeList) {
    
    if (mimeTypeList.isEmpty() == true) {
        return true;
    }
    
    bool accept = false;

    QRegExp rx("*");
    rx.setPatternSyntax(QRegExp::Wildcard);
 
    for (int i = 0; i < mimeTypeList.count(); ++i) {
        rx.setPattern (mimeTypeList.at(i));
        if (rx.exactMatch(mimeType)) {
            accept = true;
            break;
        }
    }

    return accept;    
}