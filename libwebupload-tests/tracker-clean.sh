#!/bin/sh
# Clean up files copied for tracker

if [ "$(/usr/bin/id -u)" != "0" ]; then
    PATH=$HOME
else
    PATH=/home/user
fi

/bin/rm $PATH/MyDocs/.images/libwebupload-test1.jpg || /bin/echo Failed to remove libwebupload-test1.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test2.jpg || /bin/echo Failed to remove libwebupload-test2.jpg

#Removing images used to test resize options
/bin/rm $PATH/MyDocs/.images/libwebupload-test-1200X1088.jpg || /bin/echo Failed to remove libwebupload-test-1200X1088.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-1280X1160.jpg || /bin/echo Failed to remove libwebupload-test-1280X1160.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-1468X1330.jpg || /bin/echo Failed to remove libwebupload-test-1468X1330.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-1536X2048.jpg || /bin/echo Failed to remove libwebupload-test-1536X2048.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-450X600.jpg || /bin/echo Failed to remove libwebupload-test-450X600.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-480X640.jpg || /bin/echo Failed to remove libwebupload-test-480X640.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-500X453.jpg || /bin/echo Failed to remove libwebupload-test-500X453.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-640X580.jpg || /bin/echo Failed to remove libwebupload-test-640X580.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-900X1200.jpg || /bin/echo Failed to remove libwebupload-test-900X1200.jpg
/bin/rm $PATH/MyDocs/.images/libwebupload-test-960X1280.jpg || /bin/echo Failed to remove libwebupload-test-960X1280.jpg

/bin/rm $PATH/MyDocs/test.txt || /bin/echo Failed to remove test.txt
