include (../config-flags.pri)

# Macro to disable space check. This does not work correctly right now
DEFINES += DONT_CHECK_EMPTY_SPACE

HEADERS += WebUpload/account.h \
           accountprivate.h \
           WebUpload/entry.h \
           entryprivate.h \
           WebUpload/media.h \
           mediaprivate.h \
           WebUpload/postoption.h \
           WebUpload/commonoption.h \
           WebUpload/commontextoption.h \
           WebUpload/commonlistoption.h \
           WebUpload/serviceoption.h \
           WebUpload/serviceoptionvalue.h \
           WebUpload/serviceoptionsupport.h \
           src/serviceoptionsupportprivate.h \
           WebUpload/service.h \
           serviceprivate.h \
           WebUpload/system.h \
           systemprivate.h \
           internalenums.h \
           WebUpload/error.h \
           WebUpload/httpmulticontentio.h \
           WebUpload/pluginbase.h \
           WebUpload/postinterface.h \
           WebUpload/updateinterface.h \
           WebUpload/updatebase.h \
           updatebaseprivate.h \
           WebUpload/postbase.h \
           postbaseprivate.h \
           WebUpload/authbase.h \
           authbaseprivate.h \
           WebUpload/postsimplehttp.h \
           WebUpload/updatesimplehttp.h \
           xmlhelper.h \
           internalenums.h \
           WebUpload/PluginApplication \
           WebUpload/pluginapplication.h \
           pluginapplicationprivate.h \
           WebUpload/processexchangedata.h \
           processexchangedataprivate.h \
           WebUpload/pluginprocess.h \
           WebUpload/updateprocess.h \
           updateprocessprivate.h \
           connectionmanager.h \
           connectionmanagerprivate.h
           

SOURCES += account.cpp \
           entry.cpp \
           media.cpp \
           postoption.cpp \
           commonoption.cpp \
           commontextoption.cpp \
           commonlistoption.cpp \
           serviceoption.cpp \
           serviceoptionvalue.cpp \
           src/serviceoptionsupport.cpp \
           service.cpp \
           system.cpp \
           xmlhelper.cpp \
           internalenums.cpp \
           error.cpp \
           httpmulticontentio.cpp \
           pluginbase.cpp \
           postinterface.cpp \
           updateinterface.cpp \
           updatebase.cpp \
           postbase.cpp \
           authbase.cpp \
           postsimplehttp.cpp \
           updatesimplehttp.cpp \
           pluginapplication.cpp \
           processexchangedata.cpp \
           pluginprocess.cpp \
           updateprocess.cpp \
           connectionmanager.cpp
           
