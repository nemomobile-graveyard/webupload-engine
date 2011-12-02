 
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

#include <QDebug>
#include <MLocale>
#include "WebUpload/Error"
#include "errorprivate.h"
#include <QDataStream>

using namespace WebUpload;

Error::Error () : d_ptr (new ErrorPrivate()) {
}

Error::Error (Code code, const QString & title, const QString & description,
    const QString &recoverMsg) : d_ptr (new ErrorPrivate(code, title,
    description, recoverMsg)) {
}

Error::Error (Code code, bool canContinue, bool repairable) : 
    d_ptr (new ErrorPrivate (code, "", "", "")) {

    d_ptr->m_canContinue = canContinue;
    d_ptr->m_repairable = repairable;
}

Error::Error (QByteArray & byteStream) : 
    d_ptr (new ErrorPrivate (byteStream)) {
}

Error::~Error () {
    delete d_ptr;
}

Error::Error (const Error & src) : d_ptr (new ErrorPrivate ()) {
    d_ptr->m_code = src.code();
    if (d_ptr->m_code == CODE_SERVICE_ERROR) {
        d_ptr->m_description = src.serviceErrorString ();
    } else {
        d_ptr->m_title = src.customTitle();
        d_ptr->m_description = src.customDescription();
        d_ptr->m_recoverMsg = src.recoverMsg();
    }

    d_ptr->m_repairable = src.repairable();
    d_ptr->m_canContinue = src.canContinue();
    d_ptr->m_data = src.data();
    d_ptr->m_count = src.failedCount();
    d_ptr->m_totalCount = src.transferFileCount ();
    d_ptr->m_accountName = src.accountName();
}

Error & Error::operator = (const Error & src) {
    if(this != &src) {
        d_ptr->m_code = src.code();
        if (d_ptr->m_code == CODE_SERVICE_ERROR) {
            d_ptr->m_description = src.serviceErrorString ();
        } else {
            d_ptr->m_title = src.customTitle();
            d_ptr->m_description = src.customDescription();
            d_ptr->m_recoverMsg = src.recoverMsg();
        }
        d_ptr->m_repairable = src.repairable();
        d_ptr->m_canContinue = src.canContinue();
        d_ptr->m_data = src.data();
        d_ptr->m_count = src.failedCount();
        d_ptr->m_totalCount = src.transferFileCount ();
        d_ptr->m_accountName = src.accountName();
    }

    return *this;
}

void Error::merge (const Error & other) {
    if (d_ptr->m_code == CODE_NO_ERROR) {
        d_ptr->m_code = other.code();
        if (d_ptr->m_code == CODE_SERVICE_ERROR) {
            d_ptr->m_description = other.serviceErrorString ();
        } else {
            d_ptr->m_title = other.customTitle();
            d_ptr->m_description = other.customDescription();
            d_ptr->m_recoverMsg = other.recoverMsg();
        }
        d_ptr->m_repairable = other.repairable();
        d_ptr->m_canContinue = other.canContinue();
        d_ptr->m_data = other.data();
        d_ptr->m_count = other.failedCount();
        d_ptr->m_accountName = other.accountName();
    } else {
        if (!repairable () && other.repairable ()) {
            d_ptr->m_code = other.code ();
            if (d_ptr->m_code == CODE_CUSTOM) {
                d_ptr->m_title = other.customTitle();
                d_ptr->m_description = other.customDescription();
                d_ptr->m_recoverMsg = other.recoverMsg ();
            } else if (d_ptr->m_code == CODE_SERVICE_ERROR) {
                d_ptr->m_description = other.serviceErrorString ();
            }
            d_ptr->m_data = other.data ();
            d_ptr->m_repairable = true;
        }

        d_ptr->m_canContinue = other.canContinue ();
        d_ptr->m_count += other.failedCount ();
    }
}

void Error::clearError () {
    d_ptr->clear ();
}

void Error::setFailedCount (unsigned int count) {
    d_ptr->m_count = count;
}

void Error::setTransferFileCount (unsigned int count) {
    // Set only if it hasn't been set yet
    if (d_ptr->m_totalCount == 0) 
        d_ptr->m_totalCount = count;
}

Error Error::noConnection () {
    /* code, can continue?, repairable? */
    Error error (CODE_NO_CONNECTION, false, true);
    return error;
}

Error Error::connectFailure() {
    /* code, can continue?, repairable? */
    Error error (CODE_CONNECT_FAILURE, false, true);
    return error;
}

Error Error::serviceTimeOut () {
    /* code, can continue?, repairable? */
    Error error (CODE_SERVICE_TIME_OUT, false, true);
    return error;
}

Error Error::authorizationFailed() {
    /* code, can continue?, repairable? */
    Error error (CODE_AUTH_FAILED, false, true);
    return error;
}

Error Error::uploadLimitExceeded() {
    /* code, can continue?, repairable? */
    Error error (CODE_UPLOAD_LIMIT_EXCEEDED, false, true);
    return error;
}

Error Error::invalidFileType() {
    /* code, can continue?, repairable? */
    Error error (CODE_INV_FILE_TYPE, true, false);
    return error;
}

Error Error::fileTooBig() {
    /* code, can continue?, repairable? */
    Error error (CODE_FILE_SIZE_ERROR, true, false);
    return error;
}

Error Error::fileError() {
    /* code, can continue?, repairable? */
    Error error (CODE_FILE_SIZE_TYPE_ERROR, true, false);
    return error;
}

Error Error::dateTimeError() {
    /* code, can continue?, repairable? */
    Error error (CODE_INV_DATE_TIME, false, true);
    return error;
}

Error Error::serviceError (const QString & errString) {
    /* Save the service error string to description - actual description will
     * use this string, along with the standard strings that depend on failed
     * count etc 
     * Stop with service error and it would typically not be repairable as well
     */
    Error error (CODE_SERVICE_ERROR, QString(), errString);
    return error;
}

Error Error::targetDoesNotExist() {
    /* code, can continue?, repairable? */
    Error error (CODE_TARGET_DOES_NOT_EXIST, false, true);
    return error;
}

Error Error::accountRemoved() {
    /* code, can continue?, repairable? */
    Error error (CODE_ACCOUNT_REMOVED, false, false);
    return error;
}

Error Error::accountDisabled() {
    /* code, can continue?, repairable? */
    Error error (CODE_ACCOUNT_DISABLED, false, true);
    return error;
}

Error Error::transferFailed() {
    /* code, can continue?, repairable? */
    Error error (CODE_TRANSFER_FAILED, false, false);
    return error;
}

Error Error::missingFiles() {
    /* code, can continue?, repairable? */
    Error error (CODE_MISSING_FILES, true, false);
    return error;
}

Error Error::outOfMemory () {
    /* code, can continue?, repairable? */
    Error error (CODE_OUT_OF_MEMORY, true, false);
    return error;
}

Error Error::unimplemented() {
    /* code, can continue?, repairable? */
    Error error (CODE_UNIMPLEMENTED, false, false);
    return error;
}

Error Error::secureConnection() {
    /* code, can continue?, repairable? */
    Error error (CODE_SECURE_CONNECTION_ERROR, false, true);
    return error;
}

Error Error::custom (const QString & title, const QString & description,
    const QString & retryMsg, bool canContinue) {
    
    Error error (CODE_CUSTOM, title, description, retryMsg);
    error.d_ptr->m_canContinue = canContinue;
    return error;
}

void Error::setData (const QVariant &info) {
    d_ptr->m_data = info;
}

const QVariant & Error::data () const {
    return d_ptr->m_data;
}

Error::Code Error::code() const {
    return d_ptr->m_code;
}

QString Error::title () const {
    if (!d_ptr->m_title.isEmpty()) {
        qDebug() << "Returning title" << d_ptr->m_title;
        return d_ptr->m_title;
    }

    QString str;
    switch (d_ptr->m_code) {
        case CODE_NO_ERROR:
        case CODE_NO_CONNECTION:
            str = "";
            break;

        case CODE_AUTH_FAILED: 
            // From section 4.6.1 of UI specs. Section 5.3.1 does not give any
            // string ids
            //% "Wrong username or password"
            str = qtTrId ("qtn_comm_share_wrong_credentials");
            break;

        case CODE_CONNECT_FAILURE:
            // Section 5.3.2 of UI specs
            //% "Unable to connect"
            str = qtTrId ("qtn_tui_unable_to_connect");
            break;

        case CODE_TARGET_DOES_NOT_EXIST:
            //% "Target album doesn't exist"
            str = qtTrId ("qtn_comm_share_no_album_banner");
            break;

        case CODE_UPLOAD_LIMIT_EXCEEDED:
        case CODE_INV_FILE_TYPE:
        case CODE_FILE_SIZE_ERROR:
        case CODE_FILE_SIZE_TYPE_ERROR:
        case CODE_INV_DATE_TIME:
        case CODE_TRANSFER_FAILED:
        case CODE_SERVICE_ERROR:
        case CODE_MISSING_FILES:
        case CODE_OUT_OF_MEMORY:
        case CODE_SECURE_CONNECTION_ERROR:
        {
            // Sections 5.3.3, 5.3.4 and 5.3.5 of UI specs
            qDebug() << "Failed count = " << d_ptr->m_count;
            //% "Unable to share file"
            str = qtTrId ("qtn_tui_unable_to_share_file", d_ptr->m_count);
            break;
        }

        case CODE_ACCOUNT_REMOVED:
            // Section 7.1.3.4 of UI specs
            //% "Account has been removed"
            str = qtTrId ("qtn_comm_share_removed_account");
            break;

        case CODE_ACCOUNT_DISABLED:
            // Section 7.1.3.5 of UI specs (version 1.4)
            //% "Account has been disabled"
            str = qtTrId ("qtn_comm_share_disabled_account");
            break;

        case CODE_UNIMPLEMENTED:
            str = "!!Unimplemented!!";
            break;

        case CODE_SERVICE_TIME_OUT:
            //% "Service connection lost" 
            str = qtTrId ("qtn_tui_service_connection_lost");
            break;

        default:
            qDebug() << "Could not get error title string";
            break;
    }

    return str;
}
        
QString Error::description () const {

    if (!d_ptr->m_description.isEmpty() && 
        d_ptr->m_code != CODE_SERVICE_ERROR) {

        return d_ptr->m_description;
    }

    QString str; // String that will be returned
    
    switch (d_ptr->m_code) {
        case CODE_NO_ERROR:
        case CODE_NO_CONNECTION:
            str = "";
            break;

        case CODE_AUTH_FAILED: 
            // Section 7.1.3.1
            //% "Unable to share because of wrong username or password"
            str = qtTrId ("qtn_tui_wrong_credentials_dialog");
            break;

        case CODE_CONNECT_FAILURE:
            //% "%1 is currently unavailable"
            str = qtTrId ("qtn_tui_unable_to_conn_dialog_info").
                arg (d_ptr->m_accountName);
            break;

        //  Below cases are from section 5.3.3
        case CODE_UPLOAD_LIMIT_EXCEEDED:
            //% "Service upload limit is exceeded"
            str = qtTrId ("qtn_tui_unable_to_share_upload_lim");
            break;

        case CODE_INV_FILE_TYPE:
            //% "Service does not accept the file type"
            str = qtTrId ("qtn_tui_unable_to_share_file_type");
            break;

        case CODE_FILE_SIZE_ERROR:
            //% "Some of the files were too big for the service"
            str = qtTrId ("qtn_tui_unable_share_file_size", d_ptr->m_count);
            break;

        case CODE_FILE_SIZE_TYPE_ERROR:
            //% "Service does not accept file type or file was too big"
            str = qtTrId ("qtn_tui_unable_share_file_sizetype", 
                d_ptr->m_count);
            break;

        case CODE_INV_DATE_TIME:
            //% "Check your device time and date are correct and try again"
            str = qtTrId ("qtn_tui_inv_date_time");
            break;

        case CODE_TRANSFER_FAILED:
            //% "Transfer failed and cannot be continued"
            str = qtTrId ("qtn_tui_unable_to_share_info3");
            break;

        case CODE_SERVICE_ERROR:
            //% "Transfer failed due to service error. %1"
            str = qtTrId ("qtn_tui_serv_error_title").
                arg (serviceErrorString());
            break;

        case CODE_TARGET_DOES_NOT_EXIST:
            //% "Target album does not exist anymore"
            str = qtTrId ("qtn_tui_unable_to_share_album_err");
            str.append ("<br>");
            //% "Share to default album?"
            str.append (qtTrId ("qtn_tui_unable_to_share_album_err2"));
            break;

        case CODE_ACCOUNT_REMOVED:
            // Section 5.3.6 of UI specs
            //% "Transfer cannot continue"
            str = qtTrId ("qtn_tui_unable_to_share_acc_rem1");
            break;

        case CODE_ACCOUNT_DISABLED:
            // Section 5.3.7 of UI specs
            //% "Account has been disabled. Enable the account and try again"
            str = qtTrId ("qtn_tui_account_disabled");
            break;

        case CODE_MISSING_FILES:
            //% "Selected file can't be found"
            str = qtTrId ("qtn_tui_file_removed_error", d_ptr->m_count);
            break;

        case CODE_UNIMPLEMENTED:
            str = "!!Feature not implemented yet!!";
            break;

        case CODE_SERVICE_TIME_OUT:
            //% "Service is not responding."
            str = qtTrId ("qtn_tui_service_not_responding");
            break;

        case CODE_OUT_OF_MEMORY:
            //% "There is not enough free memory to share content.  Please delete some files to free up more memory"
            str = qtTrId ("qtn_tui_low_system_memory_error");
            break;

        case CODE_SECURE_CONNECTION_ERROR:
            //% "Secure connection failed."
            str = qtTrId ("qtn_tui_ssl_connection_failed");
            break;

        default:
            qDebug() << "Could not get error description string";
            break;
    }

    return str;
}

void Error::setTitle (const QString & title) {
    d_ptr->m_title = title;
}

void Error::setDescription (const QString & description) {
    d_ptr->m_description = description;
}


QString Error::serviceErrorString () const {
    if (d_ptr->m_code == CODE_SERVICE_ERROR) {
        return d_ptr->m_description;
    } else {
        return QString();
    }
}
        
QString Error::recoverMsg () const {
    QString msg;
    if (d_ptr->m_repairable == true) {
        if (d_ptr->m_code == CODE_CUSTOM) {
            msg = d_ptr->m_recoverMsg;
        } else if (d_ptr->m_code == CODE_TARGET_DOES_NOT_EXIST) {
            //% "Share"
            msg = qtTrId ("qtn_comm_command_share");
        } else {
            //% "Retry now"
            msg = qtTrId ("qtn_comm_command_retry_now");
        }
    } 

    return msg;
}
        
bool Error::repairable() const {
    return d_ptr->m_repairable;
}

bool Error::canContinue() const {
    return d_ptr->m_canContinue;
}

unsigned int Error::failedCount () const {
    return d_ptr->m_count;
}

unsigned int Error::transferFileCount () const {
    return d_ptr->m_totalCount;
}

QByteArray Error::serialize () const {
    return d_ptr->serialize();
}

QString Error::accountName () const {
    return d_ptr->m_accountName;
}

void Error::setAccountName (const QString & accountName) {
    d_ptr->m_accountName = accountName;
}

QString Error::customTitle () const {
    return d_ptr->m_title;
}

QString Error::customDescription () const {
    if (d_ptr->m_code != CODE_SERVICE_ERROR) {
        return d_ptr->m_description;
    } else {
        return QString();
    }
}

// - Private class functions -------------------------------------------------
ErrorPrivate::ErrorPrivate () : m_code (Error::CODE_NO_ERROR),
    m_repairable (false), m_canContinue (false), m_count (0), 
    m_totalCount (0) {

}

ErrorPrivate::ErrorPrivate (Error::Code iCode, const QString & iTitle,
    const QString & iDescription, const QString & iRecoverMsg) : 
    m_code (iCode), m_title (iTitle), m_description (iDescription),
    m_recoverMsg (iRecoverMsg), m_count (1), m_totalCount (0) {
    
    if (m_recoverMsg.isEmpty()) 
        m_repairable = false;    
    else 
        m_repairable = true;
    m_canContinue = false;
}

ErrorPrivate::ErrorPrivate (const ErrorPrivate * src) : m_code (src->m_code),
    m_title (src->m_title), m_description (src->m_description),
    m_recoverMsg (src->m_recoverMsg), m_repairable (src->m_repairable),
    m_canContinue (src->m_canContinue), m_accountName (src->m_accountName),
    m_count (src->m_count), m_totalCount (src->m_totalCount), 
    m_data (src->m_data) {
}

ErrorPrivate::ErrorPrivate (QByteArray & byteStream) : 
    m_code (Error::CODE_NO_ERROR), m_title (QString()), 
    m_description (QString()), m_recoverMsg (QString()), 
    m_accountName (QString()), m_count (1), m_totalCount (1) {

    QDataStream ds (&byteStream, QIODevice::ReadOnly);

    qint32 i;
    ds >> i;
    m_code = (Error::Code)i;

    ds >> m_repairable;
    ds >> m_canContinue;
    ds >> m_count;
    ds >> m_totalCount;
    ds >> m_data;
    ds >> m_accountName;
    ds >> m_title;
    ds >> m_description;
    qDebug() << "ErrorPrivate::ErrorPrivate" << m_title << m_description;
    if (m_repairable) {
        ds >> m_recoverMsg;
    }
}

void ErrorPrivate::clear() {
    m_code = Error::CODE_NO_ERROR;
    m_title.clear ();
    m_description.clear();
    m_recoverMsg.clear ();
    m_accountName.clear ();
    m_data.clear ();
    m_count = 0;
    m_totalCount = 0;
}

QByteArray ErrorPrivate::serialize () const {
    QByteArray retVal;
    QDataStream ds (&retVal, QIODevice::WriteOnly);

    ds << (qint32) m_code;
    ds << m_repairable;
    ds << m_canContinue;
    ds << m_count;
    ds << m_totalCount;
    ds << (QVariant)m_data;
    ds << m_accountName;
    qDebug() << "ErrorPrivate::serialize" << m_title << m_description;
    ds << (QString)m_title;
    ds << (QString)m_description;
    if (m_repairable) {
        ds << (QString) m_recoverMsg;
    }

    return retVal;
}
