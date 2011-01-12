TEMPLATE        = subdirs
CONFIG         += ordered 

#Enable the line below if you want to build for process-split
CONFIG         += process-split

SUBDIRS        += webupload-service      \
                  libwebupload           \
                  libwebupload/translations \
                  webupload-engine       \
                  webupload-engine/translations \
                  test-plugin            \
                  webupload-recovery     \
                  libwebupload-tests     \
                  webupload-engine-tests \
                  metawriter \
                  publish-widgets
