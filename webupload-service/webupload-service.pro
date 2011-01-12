TEMPLATE    = lib
TARGET      = webupload-service
CONFIG     += console meegotouch
CONFIG     -= app_bundle
QT         += dbus
QT         -= gui

system(m-servicefwgen -p UploadEngine)

QMAKE_CXXFLAGS += -O2 -Werror -Wall
OBJECTS_DIR = ./obj
MOC_DIR     = ./moc

SOURCES    += uploadengineproxy.cpp uploadengine.cpp
HEADERS    += uploadengine.h uploadengineproxy.h

QMAKE_CLEAN += *.cpp *.h *~ *.so*

# target installation
target.path = /usr/lib

# install headers
headers.path       += /usr/include/webupload-engine/service/
headers.files      += *.h

INSTALLS           += target headers 

