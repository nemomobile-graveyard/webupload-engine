CONFIG += qt debug meegotouch link_pkgconfig
TEMPLATE = app
QT += dbus xml

PKGCONFIG += accounts-qt

TARGET = webupload-recovery

DEPENDPATH += ./src
INCLUDEPATH += ../webupload-service \
               ../libwebupload \
               ./src
                  
VPATH += $$DEPENDPATH $$INCLUDEPATH

QMAKE_CXXFLAGS += -O2 -Werror -Wall

OBJECTS_DIR = ./obj
MOC_DIR = ./moc
DESTDIR = ./out

# profiling
CONFIG(profiling) {
    message(Building with profiling)
    QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs
    LIBS += -lgcov
}

LIBS += ../webupload-service/libwebupload-service.so
LIBS += ../libwebupload/out/libwebupload.so

HEADERS = recovery.h

SOURCES = main.cpp recovery.cpp

# For installations
target.path = /usr/lib/webupload-engine

INSTALLS += target
