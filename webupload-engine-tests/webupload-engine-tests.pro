CONFIG      += qt debug libtuiclient link_pkgconfig qtsparql
QT          += xml testlib 
TEMPLATE     = app

include (../config-flags.pri)

PKGCONFIG += accounts-qt

QMAKE_CXXFLAGS += -Werror -Wall

TARGET       = webupload-engine-tests

DEPENDPATH  += ./src  \
               ../libwebupload/src

INCLUDEPATH += ../webupload-engine/src \
               ../libwebupload/src \
               ../libwebupload \
               ../webupload-service

VPATH       += $$DEPENDPATH $$INCLUDEPATH

OBJECTS_DIR  = ./obj
MOC_DIR      = ./moc
DESTDIR      = ./out

QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs

HEADERS +=  WUEngineTests.h        \
            uploaditem.h           \
            uploadqueue.h           \
            uploadstatistics.h 

SOURCES +=  WUEngineTests.cpp      \
            uploaditem.cpp         \
            uploadqueue.cpp        \
            uploadstatistics.cpp  

LIBS += -lgcov 
LIBS += ../libwebupload/out/libwebupload.so
LIBS += ../webupload-service/libwebupload-service.so

target.path = /usr/bin

sh.path  = /usr/share/webupload-engine-tests
sh.files = tests.xml 1.jpg 2.jpg \
           run-test.sh tracker-clean.sh tracker-copy.sh

INSTALLS += target \
            sh     \
