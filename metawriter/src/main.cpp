 
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




#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include "metaapplication.h"
#include <getopt.h>
#include <QByteArray>
#include <QString>
#include "metamandatatypes.h"
#include <iostream>

using namespace Metaman;

void printUsage(char* appName)
{
    std::cout << std::endl;
    std::cout << "Usage: " << appName << " -i inputfile -o outputfile [OTHER OPTIONS]";
    std::cout << std::endl << std::endl;
    std::cout << "-i --input              input file" << std::endl;
    std::cout << "-o --output             output file" << std::endl;
    std::cout << "-t --title              set/remove title" << std::endl;
    std::cout << "-d --description        set/remove description" << std::endl;
    std::cout << "-k --keyword            add keyword" << std::endl;
    std::cout << "-g --geotag             set geotag" << std::endl;
    std::cout << "-x --erase-all          erase all metadata" << std::endl;
    std::cout << "-c --erase-author-gps   erase author and location information" << std::endl;
    std::cout << "--itunes                use itunes-style tags" << std::endl;
    std::cout << "--standard              use standard tags" << std::endl;
    std::cout << "--xmp                   use xmp metadata" << std::endl;
    std::cout << "-h --help               print this message" << std::endl;
    std::cout << std::endl << std::endl;
}



void readInputActions(int argc,
                      char* argv[],
                      bool& setTitle,
                      bool& setDescription,
                      bool& setGeotag,
                      bool& eraseAll,
                      bool& eraseAuthorAndGps,
                      bool& printHelp,
                      int& standardTagFlag,
                      int& itunesTagFlag,
                      int& xmpFlag,
                      QString& inputFile,
                      QString& outputFile,
                      QByteArray& title,
                      QByteArray& description,
                      QByteArray& geotag,
                      QList<QByteArray>& keywords)
{


    static struct option longOptions[] =
    {
        {"standard", no_argument, &standardTagFlag, 1},
        {"itunes", no_argument, &itunesTagFlag, 1},
        {"xmp", no_argument, &xmpFlag, 1},
        {"title", required_argument, 0, 't'},
        {"description", required_argument, 0, 'd'},
        {"geotag", required_argument, 0, 'g'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"keyword", required_argument, 0, 'k'},
        {"help", no_argument, 0, 'h'},
        {"erase-all", no_argument, 0, 'x'},
        {"erase-author-gps", no_argument, 0, 'c'},

        {0, 0, 0, 0}
    };

    int optionChar;
    int index;

    while (true) {

        optionChar=getopt_long(argc,
                               argv,
                               "i:t::d::o:k:g::hxc",
                               longOptions,
                               &index);

        if (optionChar == -1) {
            break;
        }

        switch ((char)optionChar) {

            case 0:
            {
                /* If this option set a flag, do nothing else now. */
                //if (longOptions[index].flag != 0) {
                //    break;
                //}
                
                break;
            }
            
            case 't':
            {
                if (!setTitle) {
                    setTitle = true;

                    if (optarg != 0) {
                        title = QByteArray(optarg);
                    }
                }
                else {
                    qWarning() << "Error: Title conflict";
                    exit(EXIT_FAILURE);
                }
                
                break;
            }

            case 'd':
            {
                if (!setDescription) {
                    setDescription = true;

                    if (optarg != 0) {
                        description = QByteArray(optarg);
                    }
                }
                else {
                    qWarning() << "Error: Description conflict";
                    exit(EXIT_FAILURE);
                }
                
                break;
            }
            case 'g':
            {
                if (!setGeotag) {
                    setGeotag = true;

                    if (optarg != 0) {
                        geotag = QByteArray(optarg);
                    }
                }
                else {
                    qWarning() << "Error: Geotag conflict";
                    exit(EXIT_FAILURE);
                }
                
                break;
            }

            case 'k':
            {
                if (optarg != 0) {
                    keywords << QByteArray(optarg);
                }
                
                break;
            }

            case 'i':
            {
                if (inputFile.isEmpty()) {
                    inputFile = QString(optarg);
                }
                else {
                    qWarning() << "Error: Input file conflict";
                    exit(EXIT_FAILURE);
                }

                break;
            }

            case 'o':
            {
                if (outputFile.isEmpty()) {
                    outputFile = QString(optarg);
                }
                else {
                    qWarning() << "Error: Output file conflict";
                    exit(EXIT_FAILURE);
                }

                break;
            }

            case 'h':
            {
                printHelp = true;
                break;
            }

            case 'x':
            {
                eraseAll = true;
                break;
            }

            case 'c':
            {
                eraseAuthorAndGps = true;
                break;
            }

            case '?':
            default:
            {
                break;
            }

        }
    }

    if (eraseAll && (setTitle || setDescription)) {
        qWarning() << "Conflicting options";
        exit(EXIT_FAILURE);
    }
}



int processActions(MetaApplication* metaApp,
                   char* appName,
                   bool setTitle,
                   bool setDescription,
                   bool setGeotag,
                   bool eraseAll,
                   bool eraseAuthorAndGps,
                   bool printHelp,
                   int standardTagFlag,
                   int itunesTagFlag,
                   int xmpFlag,
                   QString inputFile,
                   QString outputFile,
                   QByteArray title,
                   QByteArray description,
                   QByteArray geotag,
                   QList<QByteArray>& keywords)
{
    bool noActionsSpecified =
        !(eraseAll              || 
          setTitle              || 
          setDescription        || 
          eraseAuthorAndGps     || 
          setGeotag             || 
          !keywords.isEmpty());

    if (printHelp ||
        inputFile.isEmpty() ||
        outputFile.isEmpty() ||
        noActionsSpecified) {

        printUsage(appName);
        return EXIT_FAILURE;
    }

    int operationMode = 0;

    if (standardTagFlag) {
        operationMode |= OPERATION_MODE_STANDARD;
    }

    if (itunesTagFlag) {
        operationMode |= OPERATION_MODE_ITUNES;
    }

    if (xmpFlag) {
        operationMode |= OPERATION_MODE_XMP;
    }

    metaApp = new MetaApplication(OperationMode(operationMode));

    if (metaApp == 0) {
        qDebug() << "failed to initialize metadata handler";
        return EXIT_FAILURE;
    }

    metaApp->setInputFile(inputFile);
    metaApp->setOutputFile(outputFile);

    if (!metaApp->ableToProcess()) {
        qDebug() << "unknown filetype";
        return EXIT_FAILURE;
    }

    OperationResult readFileResult = metaApp->readFile();

    if (readFileResult != OPERATION_OK) {
        qDebug() << "reading the file failed";
        return EXIT_FAILURE;
    }

    if (eraseAll) {
        qDebug() << "erasing all metadata";
        OperationResult eraseMetaDataResult = metaApp->eraseMetaData();

        if (eraseMetaDataResult != OPERATION_OK) {
            return EXIT_FAILURE;
        }
    }
    else {
        OperationResult setTitleResult = OPERATION_OK;

        if (setTitle) {
            qDebug() << "setting title";
            setTitleResult = metaApp->setTitle(title);
        }

        OperationResult setDescriptionResult = OPERATION_OK;

        if (setDescription) {
            qDebug() << "setting description";
            setDescriptionResult = metaApp->setDescription(description);
        }

        OperationResult setGeotagResult = OPERATION_OK;

        if (setGeotag) {
            qDebug() << "setting geotag";
            setGeotagResult = metaApp->setGeotag(geotag);
        }

        OperationResult addKeywordsResult = OPERATION_OK;
        
        if (!keywords.isEmpty()) {
            qDebug() << "adding keywords";
            addKeywordsResult = metaApp->addKeywords(keywords);
        }

        OperationResult eraseAuthorAndGpsResult = OPERATION_OK;

        if (eraseAuthorAndGps) {
            qDebug() << "removing auth & gps";
            eraseAuthorAndGpsResult = metaApp->eraseAuthorAndGps();
        }

        if (setTitleResult != OPERATION_OK ||
            setDescriptionResult != OPERATION_OK ||
            setGeotagResult != OPERATION_OK ||
            addKeywordsResult != OPERATION_OK ||
            eraseAuthorAndGpsResult != OPERATION_OK) {
            qDebug() << "one or more of the requested operations failed";
            return EXIT_FAILURE;
        }
    }

    OperationResult writeFileResult = metaApp->writeFile();

    if (writeFileResult != OPERATION_OK) {
        qDebug() << "writing the file failed";
        return EXIT_FAILURE;
    }

    qDebug() << "all done, everything's ok";
    return EXIT_SUCCESS;
}



int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    bool setTitle = false;
    bool setDescription = false;
    bool setGeotag = false;
    bool eraseAll = false;
    bool eraseAuthorAndGps = false;
    bool printHelp = false;    
    int itunesTagFlag = 0;
    int standardTagFlag = 0;
    int xmpFlag = 0;
    QByteArray title;
    QByteArray description;
    QByteArray geotag;
    QString inputFile;
    QString outputFile;
    QList<QByteArray> keywords;

    readInputActions(argc,
                     argv,
                     setTitle,
                     setDescription,
                     setGeotag,
                     eraseAll,
                     eraseAuthorAndGps,
                     printHelp,
                     itunesTagFlag,
                     standardTagFlag,
                     xmpFlag,
                     inputFile,
                     outputFile,
                     title,
                     description,
                     geotag,
                     keywords);
    char* appName = argv[0];
    
    MetaApplication* metaApp = 0;
    int processingResult = processActions(metaApp,
                                           appName,
                                           setTitle,
                                           setDescription,
                                           setGeotag,
                                           eraseAll,
                                           eraseAuthorAndGps,
                                           printHelp,
                                           itunesTagFlag,
                                           standardTagFlag,
                                           xmpFlag,
                                           inputFile,
                                           outputFile,
                                           title,
                                           description,
                                           geotag,
                                           keywords);

    delete metaApp;
    metaApp = 0;

    return processingResult;
}