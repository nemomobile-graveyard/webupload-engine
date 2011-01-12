 
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
 
#include "uploadstatistics.h"
#include <QDebug>

UploadStatistics::UploadStatistics (unsigned int historySize, QObject * parent)
    : QObject (parent), timestamps (historySize), dones (historySize),
    loopIndex (historySize - 1), byteSize (0), samplesReceived (0),
    secondsLeft (-1) {
    
    Q_ASSERT (historySize > 0);
    
    for (unsigned int i = 0; i < historySize; ++i) {
        dones[i] = 0.0;
    }

    connect (this, SIGNAL (startCalculations ()), this, 
            SLOT (calculateEstimates ()), Qt::QueuedConnection);
}

UploadStatistics::~UploadStatistics () {
}

void UploadStatistics::setSize (qint64 size) {
    byteSize = size;
}

bool UploadStatistics::nowDone (float done) {

    // Return if done is same as last time or time is the same as the last time
    // Need to convert to time_t since QDateTime otherwise checks at
    // milliseconds level and does not behave as expected in the check below
    if ((done == dones[loopIndex]) || 
        ((!timestamps[loopIndex].isNull()) && 
         (QDateTime::currentDateTime().toTime_t() == timestamps[loopIndex].toTime_t()))) {
        return false;
    }

    ++samplesReceived;

    dones[nextLoopIndex ()] = done;
    timestamps[loopIndex] = QDateTime::currentDateTime ();    

    Q_EMIT (startCalculations ());
    return true;
}

int UploadStatistics::nextLoopIndex () {
    if (loopIndex >= (dones.size() - 1)) {
        loopIndex = 0;
    } else {
        ++loopIndex;   
    }
    
    return loopIndex;
}

int UploadStatistics::oldestIndex () const {
    int startIndex = loopIndex - 1;
    int lastValidIndex = -1;

    if (loopIndex == 0) {
        startIndex = dones.size() - 1;
    }
    
    for (int i = startIndex; i != loopIndex; --i) {
        if (i == -1) {
            i = dones.size();
            continue;
        } else if (!(timestamps[i].isNull())) {
            lastValidIndex = i;
        }
    }
    
    return lastValidIndex;
}

void UploadStatistics::calculateEstimates () {
    int startIndex = oldestIndex();   
    if (startIndex == -1) {
        return;
    }
    
    int endIndex = loopIndex;
    
    float doneBetween = dones[endIndex] - dones[startIndex];
    
    /*
    qDebug() << "Done between" << endIndex << startIndex << ":"
        << doneBetween;    
    */
    
    //TODO: If we move back in done, what then? For now ignore those.
    if (doneBetween <= 0.0) {
        return;
    }
    
    float timeBetween = timestamps[endIndex].toTime_t() -
        timestamps[startIndex].toTime_t();

    float totalEstimate = timeBetween / doneBetween;
        
    float leftEstimate = totalEstimate * (1.0 - dones[endIndex]);
    
    secondsLeft = leftEstimate;
    Q_EMIT (timeLeftEstimate (secondsLeft));   
}

int UploadStatistics::seconds () const {
    return secondsLeft;
}

float UploadStatistics::lastProgressValue() const {
    float retVal = 0.0;
    if (loopIndex >= 0) {
        retVal = dones[loopIndex];
    }
    return retVal;
}