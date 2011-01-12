
/*
 * This file is part of Web Upload Engine for MeeGo social networking uploads
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,     
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS  
 * IN THE SOFTWARE. 
 */




/*!
 * \example testplugin.cpp
 * Example plugin class implementation
 */ 

#include "testplugin.h"
#include "testpost.h"
#include "testupdate.h"
#include <WebUpload/PluginApplication>

// This macro is needed to get main function generated for your plugin. Given
// argument is name of your plugin class.
PLUGIN_MAIN_FUNCTION (TestPlugin)

// Remember to call parent class constructor
TestPlugin::TestPlugin(QObject * parent) : WebUpload::PluginBase(parent) {
}

TestPlugin::~TestPlugin() {
}

bool TestPlugin::init() {
    // No need to reimplement init if your plugin can fail at init or if it
    // does not need any initialization functionality. This function here
    // always return true (base class does that already) just as an example.
    return true;
}

WebUpload::PostInterface * TestPlugin::getPost() {
    // Just make your own post class instance and return pointer to it. Parency
    // takes care that post option is destroyed when plugin is deleted.
    return new TestPost(this);
}

WebUpload::UpdateInterface * TestPlugin::getUpdate() {
    // Just make your own update class instance and return pointer to it
    // Null is enough if you don't need update functionality with your service.
    // Parency takes care that post option is destroyed when plugin is deleted.
    return new TestUpdate(this);
}