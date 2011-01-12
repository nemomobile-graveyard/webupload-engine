include (../config-flags.pri)

CONFIG    += qt debug libtuiclient network meegotouchcore link_pkgconfig qmsystem2
TEMPLATE   = app
QT        += dbus network xml
QT        -= gui

PKGCONFIG += accounts-qt

# Need to first copy the interface xml file, because m-servicefwgen currently
# works only on the current directory. Need to raise bug about this
system(cp ../webupload-service/UploadEngine.xml .)
system(m-servicefwgen -a UploadEngine)

TARGET          = webupload-engine

DEPENDPATH     += ./   \
                  ./src
INCLUDEPATH    += ./     \
                  ../libwebupload \
                  ../libwebupload/src 
# for connectionmanager.h
                  
VPATH          += $$DEPENDPATH $$INCLUDEPATH

QMAKE_CXXFLAGS += -O2 -Werror -Wall

OBJECTS_DIR     = ./obj
MOC_DIR         = ./moc
DESTDIR         = ./out
QMAKE_CLEAN += obj/* \
               out/* \
               moc/*

LIBS += ../libwebupload/out/libwebupload.so
LIBS += ../webupload-service/libwebupload-service.so

# profiling
CONFIG(profiling) {
    message(Building with profiling)
    QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs
    LIBS += -lgcov
}

include (webupload-engine-source.pri)
SOURCES += uploadengineadaptor.cpp

QMAKE_CLEAN += UploadEngine.xml \
               uploadengineadaptor.h \
               uploadengineadaptor.cpp 

# For installations
target.path = /usr/bin

service.files += com.meego.sharing.webuploadengine.service
service.path   = /usr/share/dbus-1/services

INSTALLS += target     \
            service

