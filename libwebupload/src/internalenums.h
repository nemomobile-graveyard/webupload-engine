 
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

#ifndef _INTERNAL_ENUMS_H_
#define _INTERNAL_ENUMS_H_

#include <QUrl>

namespace WebUpload {

    enum TransferState {
        TRANSFER_STATE_START = 0,
        TRANSFER_STATE_UNINITIALIZED = TRANSFER_STATE_START,

        TRANSFER_STATE_PENDING,
        TRANSFER_STATE_ACTIVE,
        TRANSFER_STATE_CANCELLED,
        TRANSFER_STATE_DONE,

        //! Not used currently, since webuploads do not support pause
        TRANSFER_STATE_PAUSED,

        TRANSFER_STATE_MAX
    };

    TransferState transferStateEnum (QUrl trackerStateIri);
    QUrl transferStateIri (TransferState state);

};

#endif