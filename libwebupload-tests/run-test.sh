#!/bin/sh
# Make sure that test isn't run as a root (needed for tracker tests)

if [ "$(/usr/bin/id -u)" != "0" ]; then
    /bin/echo "Already user"
    /bin/echo "Running test case $1"
    /usr/bin/libwebupload-tests $1
else
    /bin/echo User run test $1
    /bin/su user -c /usr/bin/libwebupload-tests $1
fi
