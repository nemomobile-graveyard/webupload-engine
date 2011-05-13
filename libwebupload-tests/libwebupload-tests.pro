CONFIG += qt debug quillimagefilter link_pkgconfig meegotouch quillmetadata
CONFIG += mdatauri qtsparql
QT += xml testlib network
TEMPLATE = app

# Comment out first two lines or 3rd line
CONFIG += mobility
MOBILITY += systeminfo
#DEFINES += DONT_CHECK_EMPTY_SPACE

DEFINES += UNIT_TESTING

PKGCONFIG += accounts-qt libsignon-qt

TARGET       = libwebupload-tests

DEPENDPATH  += ./src  \
               ../libwebupload/src

INCLUDEPATH += ./src \
               ../libwebupload \
               ../libwebupload/src \
               ../webupload-service

VPATH       += $$DEPENDPATH $$INCLUDEPATH

OBJECTS_DIR  = ./obj
MOC_DIR      = ./moc
DESTDIR      = ./out

QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -Werror

include (../libwebupload/libwebupload-sources.pri)

SOURCES += libwebuploadtests.cpp dummypost.cpp
HEADERS += libwebuploadtests.h dummypost.h

LIBS += -lgcov
LIBS += ../webupload-service/libwebupload-service.so

target.path = /usr/bin

sh.path  = /usr/share/libwebupload-tests
sh.files = tests.xml \
           test.txt \
           *.service.xml \
           empty.xml   \
           mimetypes.xml \
           1.jpg       \
           2.jpg       \
           1200X1088.jpg \
           1280X1160.jpg \
           1468X1330.jpg \
           1536X2048.jpg \
           450X600.jpg \
           500X453.jpg \
           640X580.jpg \
           900X1200.jpg \
           960X1280.jpg \
           480X640.jpg \
           tracker-copy.sh \
           tracker-clean.sh \
           run-test.sh
           
invalid_plugin.path  = /usr/lib/webupload/plugins
invalid_plugin.files = libempty_plugin.so
           
INSTALLS += target \
            sh     \
            invalid_plugin
