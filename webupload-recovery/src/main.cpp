
/*
 * This file is part of Web Upload Engine for MeeGo social networking uploads
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,     
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS  
 * IN THE SOFTWARE. 
 */




#include <QDebug>
#include <QCoreApplication>
#include "recovery.h"
#include <QTimer>
#include <iostream>

int main (int argc, char ** argv) {
    
    bool clean = false;
    bool showHelp = false;
    
    for (int i = 1; i < argc; ++i) {
        QString args = argv[i];
        if (args == "--clean" || args == "-c") {
            clean = true;
        } else if (args == "--help" || args == "-h") {
            showHelp = true;
        }
    }
    
    if (showHelp == true) {
        std::cout << "### WebUpload Engine Recovery Tool ###" << std::endl
            << "no arguments    Load undone entries and send to engine"
            << std::endl 
            << "-h, --help      Show this help"
            << std::endl
            << "-c, --clean     Just clean entries. Do not sent to engine"
            << std::endl;
        return EXIT_SUCCESS;
        
    }
    
    qDebug() << "Webupload engine recovery check";    
    QCoreApplication coreApp (argc, argv);
    Recovery recoveryInst (clean);

    QObject::connect (&recoveryInst, SIGNAL(done()), &coreApp, SLOT(quit()),
        Qt::QueuedConnection);
    QTimer::singleShot (100, &recoveryInst, SLOT (recover()));

    return coreApp.exec ();
}