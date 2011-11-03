TEMPLATE = lib
QT += xml
CONFIG += debug meegotouch link_pkgconfig libwebupload share-widgets
PKGCONFIG += accounts-qt libsignon-qt 

TARGET = publishwidgets

DEPENDPATH += ./src \
              ./
               
INCLUDEPATH += ./PublishWidgets \
               ./src \
               ../libwebupload

VPATH += $$DEPENDPATH $$INCLUDEPATH

QMAKE_CXXFLAGS += -O2 -Werror -Wall -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic

# profiling
CONFIG(profiling) {
    message(Building with profiling)
    QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs
    LIBS += -lgcov
}

OBJECTS_DIR = ./obj
MOC_DIR     = ./moc
DESTDIR     = ./out

include (publish-widgets-sources.pri)

VER_MAJ=0
VER_MIN=2
VER_PAT=0
           
target.path = /usr/lib
INSTALLS += target

pkgconfig.path = /usr/lib/pkgconfig/
pkgconfig.files = publish-widgets.pc
INSTALLS += pkgconfig

# API Documentation with Doxygen
dox.path        = /usr/share/doc/publish-widgets
!contains( DEFINES, NO_DOCS ) {
    dox.commands    = doxygen doxygen.cfg
}
dox.depends     = PublishWidgets       
dox.files       = doc/html/*
dox.CONFIG     += no_check_exist
INSTALLS       += dox
QMAKE_EXTRA_TARGETS += dox

example.path  = /usr/share/doc/webupload-engine/libwebuploadpui/example
example.files = example/*.cpp \
                example/*.pro \
                example/*.h
INSTALLS     += example

# Install prf file for others to use 
prf.path = $$[QT_INSTALL_DATA]/mkspecs/features
prf.files = publish-widgets.prf
INSTALLS += prf

pubheaders.path = /usr/include/PublishWidgets
pubheaders.files = PublishWidgets/*
INSTALLS += pubheaders


