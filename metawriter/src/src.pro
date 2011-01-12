PKGCONFIG += exempi-2.0
SOURCES += main.cpp \
           metaapplication.cpp \
           atom.cpp \
           mp4backend.cpp \
           xmphandler.cpp \
           mpeg4atomutility.cpp
TEMPLATE = app
CONFIG += warn_on thread qt link_pkgconfig
TARGET = metawriter
DESTDIR = ../bin
QT -= gui
HEADERS += backendinterface.h \
           metaapplication.h \
           magic.h \
           metamandatatypes.h \
           atom.h \
           mp4backend.h \
           xmphandler.h \
           mpeg4atomutility.h

QMAKE_CXXFLAGS += -O2 -Werror -Wall

# For installations
target.path = /usr/lib/webupload-engine

INSTALLS += target

