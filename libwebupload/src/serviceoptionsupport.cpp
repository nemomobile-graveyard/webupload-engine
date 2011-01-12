 
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

#include "WebUpload/serviceoptionsupport.h"
#include "serviceoptionsupportprivate.h"
#include <QDebug>

using namespace WebUpload;

UpdateValueSettings::UpdateValueSettings (const QString & command) :
    d_ptr (new UpdateValueSettingsPrivate ()) {

    d_ptr->m_command = command;
}


UpdateValueSettings::~UpdateValueSettings () {
    delete d_ptr;
}


void UpdateValueSettings::setCommand (const QString & command) {
    if (!d_ptr->m_command.isEmpty () && command != d_ptr->m_command) {
        qWarning() << "Overwriting existing update command" <<
            d_ptr->m_command;
    } 

    d_ptr->m_command = command;
}


const QString & UpdateValueSettings::command () const {
    return d_ptr->m_command;
}


//------------------------- AddValueSettings -------------------
AddValueSettings::AddValueSettings () : 
    d_ptr (new AddValueSettingsPrivate ()) {

    d_ptr->m_maxLength = 0;
}


AddValueSettings::~AddValueSettings () {
    delete d_ptr;
}


void AddValueSettings::setCommand (const QString & command) {
    if (!d_ptr->m_command.isEmpty () && command != d_ptr->m_command) {
        qWarning() << "Overwriting existing add command" <<
            d_ptr->m_command;
    } 

    d_ptr->m_command = command;
}


const QString & AddValueSettings::command () const {
    return d_ptr->m_command;
}

void AddValueSettings::setCaption (const QString & caption) {
    if (!d_ptr->m_caption.isEmpty () && caption != d_ptr->m_caption) {
        qWarning() << "Overwriting existing add caption" <<
            d_ptr->m_caption;
    } 

    d_ptr->m_caption = caption;
}


const QString & AddValueSettings::caption () const {
    return d_ptr->m_caption;
}


void AddValueSettings::setTooltip (const QString & tooltip) {
    if (!d_ptr->m_tooltip.isEmpty () && tooltip != d_ptr->m_tooltip) {
        qWarning() << "Overwriting existing add tooltip" <<
            d_ptr->m_tooltip;
    } 

    d_ptr->m_tooltip = tooltip;
}


const QString & AddValueSettings::tooltip () const {
    return d_ptr->m_tooltip;
}

void AddValueSettings::setNote (const QString & note) {
    if (!d_ptr->m_note.isEmpty () && note != d_ptr->m_note) {
        qWarning() << "Overwriting existing add note" <<
            d_ptr->m_note;
    } 

    d_ptr->m_note = note;
}


const QString & AddValueSettings::note () const {
    return d_ptr->m_note;
}


void AddValueSettings::setMaxLength (int maxLength) {
    d_ptr->m_maxLength = maxLength;
}

int AddValueSettings::maxLength () const {
    return d_ptr->m_maxLength;
}

void AddValueSettings::setAddButton (const QString & addButton) {
    if (!d_ptr->m_addButton.isEmpty () && addButton != d_ptr->m_addButton) {
        qWarning() << "Overwriting existing add addButton" <<
            d_ptr->m_addButton;
    } 

    d_ptr->m_addButton = addButton;
}


const QString & AddValueSettings::addButton () const {
    return d_ptr->m_addButton;
}