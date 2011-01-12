#!/bin/sh
# Clean up files copied for tracker

if [ "$(/usr/bin/id -u)" != "0" ]; then
    PATH=$HOME
else
    PATH=/home/user
fi

/bin/rm $PATH/MyDocs/.images/webupload-engine-test1.jpg || /bin/echo Failed to remove webupload-engine-test1.jpg
/bin/rm $PATH/MyDocs/.images/webupload-engine-test2.jpg || /bin/echo Failed to remove webupload-engine-test2.jpg
