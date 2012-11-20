#!/bin/sh
# Copy test file so that it can be found with tracker

if [ "$(/usr/bin/id -u)" != "0" ]; then
    PATH=$HOME
else
    PATH=/home/user
fi

# Create the destination path if it does not exist
/bin/mkdir -p $PATH/MyDocs/.images

/bin/cp /opt/tests/webupload-engine/1.jpg $PATH/MyDocs/.images/webupload-engine-test1.jpg || /bin/echo Failed to copy file
/bin/cp /opt/tests/webupload-engine/2.jpg $PATH/MyDocs/.images/webupload-engine-test2.jpg || /bin/echo Failed to copy file

/bin/echo Give some time for tracker to index...
/bin/sleep 300
