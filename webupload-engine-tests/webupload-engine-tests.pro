CONFIG      += qt debug libtuiclient link_pkgconfig qtsparql mdatauri
QT          += xml testlib 
TEMPLATE     = app

include (../config-flags.pri)

PKGCONFIG += accounts-qt contentaction-0.1

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

target.path = /opt/tests/webupload-engine

testdefinition.path = /opt/tests/webupload-engine/test-definition
testdefinition.files = tests.xml

sh.path  = /opt/tests/webupload-engine
sh.files = 1.jpg 2.jpg \
           run-test.sh tracker-clean.sh tracker-copy.sh

INSTALLS += target \
            sh     \
