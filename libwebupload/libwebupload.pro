CONFIG += qt debug link_pkgconfig meegotouchcore network
CONFIG += quillmetadata mdatauri qtsparql
QT += xml network 
TEMPLATE = lib
PKGCONFIG += accounts-qt libsignon-qt 

# Comment either two following lines or the 3rd line
CONFIG += mobility
MOBILITY += systeminfo
#DEFINES += DONT_CHECK_EMPTY_SPACE

DEFINES += WEBUPLOAD_EXPORTS
QMAKE_CXXFLAGS += -O2 -Werror -Wall

VER_MAJ=0
VER_MIN=2
VER_PAT=0

TARGET = webupload

DEPENDPATH  += ./src

# Use just the local root so paths WebUpload/Class will work as they do when
# those are installed.
INCLUDEPATH += ./ \
               ../webupload-service \

VPATH += $$DEPENDPATH $$INCLUDEPATH

OBJECTS_DIR  = ./obj
MOC_DIR      = ./moc
DESTDIR      = ./out
QMAKE_CLEAN += obj/* \
               out/* \
               moc/*

include (libwebupload-sources.pri)

LIBS += ../webupload-service/libwebupload-service.so

# Install binary application
target.path = /usr/lib
INSTALLS += target

pubheaders.path = /usr/include/WebUpload
pubheaders.files = WebUpload/*
INSTALLS += pubheaders

# Install prf file for others to use
prf.path = $$[QT_INSTALL_DATA]/mkspecs/features
prf.files = libwebupload.prf webupload-plugin.prf
INSTALLS += prf

# API Documentation with Doxygen
dox.path        = /usr/share/doc/libwebupload
dox.commands    = doxygen doxygen.cfg
dox.depends     = WebUpload
dox.files       = doc/html/*
dox.CONFIG     += no_check_exist
INSTALLS       += dox
QMAKE_EXTRA_TARGETS += dox

pkgconfigpc.path = /usr/lib/pkgconfig/
pkgconfigpc.files = libwebupload.pc
INSTALLS += pkgconfigpc

