#!/bin/sh

if [ "$(/usr/bin/id -u)" != "0" ]; then
    PATH=$HOME
else
    PATH=/home/user
fi

# Create the destination path if it does not exist
/bin/mkdir -p $PATH/MyDocs/.images

# Images for testing other libsharing functionality
/bin/cp /usr/share/libwebupload-tests/1.jpg $PATH/MyDocs/.images/libwebupload-test1.jpg || /bin/echo Failed to copy 1.jpg
/bin/cp /usr/share/libwebupload-tests/2.jpg $PATH/MyDocs/.images/libwebupload-test2.jpg || /bin/echo Failed to copy 2.jpg

# Images to test resizing options
/bin/cp /usr/share/libwebupload-tests/1200X1088.jpg $PATH/MyDocs/.images/libwebupload-test-1200X1088.jpg || /bin/echo Failed to copy 1200X1088.jpg
/bin/cp /usr/share/libwebupload-tests/1280X1160.jpg $PATH/MyDocs/.images/libwebupload-test-1280X1160.jpg || /bin/echo Failed to copy 1280X1160.jpg
/bin/cp /usr/share/libwebupload-tests/1468X1330.jpg $PATH/MyDocs/.images/libwebupload-test-1468X1330.jpg || /bin/echo Failed to copy 1468X1330.jpg
/bin/cp /usr/share/libwebupload-tests/1536X2048.jpg $PATH/MyDocs/.images/libwebupload-test-1536X2048.jpg || /bin/echo Failed to copy 1536X2048.jpg
/bin/cp /usr/share/libwebupload-tests/450X600.jpg $PATH/MyDocs/.images/libwebupload-test-450X600.jpg || /bin/echo Failed to copy 450X600.jpg
/bin/cp /usr/share/libwebupload-tests/500X453.jpg $PATH/MyDocs/.images/libwebupload-test-500X453.jpg || /bin/echo Failed to copy 500X453.jpg
/bin/cp /usr/share/libwebupload-tests/640X580.jpg $PATH/MyDocs/.images/libwebupload-test-640X580.jpg || /bin/echo Failed to copy 640X580.jpg
/bin/cp /usr/share/libwebupload-tests/900X1200.jpg $PATH/MyDocs/.images/libwebupload-test-900X1200.jpg || /bin/echo Failed to copy 900X1200.jpg
/bin/cp /usr/share/libwebupload-tests/960X1280.jpg $PATH/MyDocs/.images/libwebupload-test-960X1280.jpg || /bin/echo Failed to copy 960X1280.jpg
/bin/cp /usr/share/libwebupload-tests/480X640.jpg $PATH/MyDocs/.images/libwebupload-test-480X640.jpg || /bin/echo Failed to copy 480X640.jpg
/bin/cp /usr/share/libwebupload-tests/test.txt $PATH/MyDocs/test.txt || /bin/echo Failed to copy test.txt

#/bin/echo Give some time for tracker to index...
/bin/sleep 300
