 
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

#ifndef _UPLOAD_STATISTICS_H_
#define _UPLOAD_STATISTICS_H_

#include <QObject>
#include <QVector>
#include <QDateTime>

/*!
   \class  UploadStatistics
   \brief  Class to calculate statistics for upload
   \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
 */
class UploadStatistics : public QObject {

    Q_OBJECT
    
public:
    /*!
      \brief Create new statistics class
      \param historySize How many samples are stored to calculate estimates
      \param parent QObject parent
     */
    UploadStatistics (unsigned int historySize = 5, QObject * parent = 0);
    ~UploadStatistics ();
    
    /*!
      \brief Define size of transfer followed (used to calculate speed
             estimates)
      \param size Size of transfer in bytes
     */
    void setSize (qint64 size);
    
    /*!
      \brief How many seconds left (current estimate). This doesn't have
             time fix. It's only updated to seconds left when last nowDone
             was called. So use signals if possible.
      \return Current estimation or -1 if there isn't estimate
     */
    int seconds () const;
    
    /*!
      \brief return last progress sent value.
      \return last progress sent
    */
    float lastProgressValue() const;

    /*!
      \brief Reset upload statistics
     */
    void reset();
    
public Q_SLOTS:
    
    /*!
      \brief Slot to add new done level
      \param done How much of transfer is done
      \return Returns true if the done value is to be used, false otherwise
     */
    bool nowDone (float done);
    
Q_SIGNALS:

    /*!
      \brief Signal emitted when estimate has changed
      \param seconds How many seconds is the current estimate
     */
    void timeLeftEstimate (int seconds);
    
    /*!
      \brief Signal emitted when speed has changed
      \param bytesInSecond How many bytes is send in second
     */
    void speedEstimate (int bytesInSecond);

    //! \brief Internal signal to start calculation of estimates
    void startCalculations ();

private Q_SLOTS:
    
    //! \brief Slot to calculate estimates and emits signals
    void calculateEstimates ();

private:

    /*!
      \brief Get next index. Also updates loopIndex variable.
      \return New index value
     */
    int nextLoopIndex ();
    
    /*!
      \brief What is the oldest index in loop
      \return Oldest index value or -1 is there is no samples in memory
     */
    int oldestIndex () const;
    
    QVector <QDateTime> timestamps; //!< List of timestamps
    QVector <float> dones; //!< List of done levels
    int loopIndex; //!< Index in loop        
    qint64 byteSize; //!< Size of transfer
    int samplesReceived; //!< Do we have enough data in loop
    int secondsLeft; //!< Stored seconds left value

};

#endif //#ifndef _UPLOAD_STATISTICS_H_
