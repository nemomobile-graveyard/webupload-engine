include (../config-flags.pri)

CONFIG(process-split) {
    message(Building for process split)
    TEMPLATE = app
    DEFINES += TEST_PROCESS_SPLIT
} else {
    message(Building normal plugin)
    TEMPLATE = lib
    CONFIG  += plugin
}

CONFIG    += qt debug link_pkgconfig
PKGCONFIG += accounts-qt libsignon-qt
QT += xml

TARGET      = test-plugin
DEFINES    += QT_NO_DEBUG

DEPENDPATH  += . ./src
INCLUDEPATH += . ./src ../libwebupload
VPATH       += $$DEPENDPATH $$INCLUDEPATH

OBJECTS_DIR  = ./obj
MOC_DIR      = ./moc
DESTDIR      = ./out

LIBS  += ../libwebupload/out/libwebupload.so
LIBS += ../webupload-service/libwebupload-service.so

HEADERS += testplugin.h testpost.h testupdate.h
SOURCES += testplugin.cpp testpost.cpp testupdate.cpp

target.path = /usr/lib/webupload/plugins/
INSTALLS    += target

