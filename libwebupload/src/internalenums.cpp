 
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
 
#include "internalenums.h"
#include <QDebug>

const QUrl state_paused_url = 
    QUrl("http://www.tracker-project.org/temp/mto#state-paused");
const QUrl state_active_url = 
    QUrl ("http://www.tracker-project.org/temp/mto#state-active");
const QUrl state_cancelled_url =
    QUrl ("http://www.tracker-project.org/temp/mto#state-cancelled");
const QUrl state_done_url =
    QUrl ("http://www.tracker-project.org/temp/mto#state-done");
const QUrl state_pending_url =
    QUrl ("http://www.tracker-project.org/temp/mto#state-pending");

WebUpload::TransferState WebUpload::transferStateEnum (QUrl trackerStateIri) {
    WebUpload::TransferState state;

    qDebug() << __FUNCTION__ << trackerStateIri;

    if(trackerStateIri == state_pending_url) {
        state = WebUpload::TRANSFER_STATE_PENDING;
    } else if(trackerStateIri == state_active_url) {
        state = WebUpload::TRANSFER_STATE_ACTIVE;
    } else if(trackerStateIri == state_paused_url) {
        state = WebUpload::TRANSFER_STATE_PAUSED;
    } else if(trackerStateIri == state_cancelled_url) {
        state = WebUpload::TRANSFER_STATE_CANCELLED;
    } else if(trackerStateIri == state_done_url) {
        state = WebUpload::TRANSFER_STATE_DONE;
    } else {
        qWarning() << "Invalid state \"" << trackerStateIri.toString() <<
            "\"";
        state = WebUpload::TRANSFER_STATE_PENDING;
    }

    return state;
}


QUrl WebUpload::transferStateIri (WebUpload::TransferState state) {
    QUrl state_iri;

    switch(state) {
        case WebUpload::TRANSFER_STATE_PENDING: 
            state_iri = state_pending_url;
            break;
        case WebUpload::TRANSFER_STATE_ACTIVE:
            state_iri = state_active_url;
            break;
        case WebUpload::TRANSFER_STATE_PAUSED:
            state_iri = state_paused_url;
            break;
        case WebUpload::TRANSFER_STATE_CANCELLED:
            state_iri = state_cancelled_url;
            break;
        case WebUpload::TRANSFER_STATE_DONE:
            state_iri = state_done_url;
            break;
        default:
            state_iri = state_pending_url;
            break;
    }

    qDebug() << __FUNCTION__ << state_iri;
    return state_iri;
}