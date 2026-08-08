/*
based on ofxZipArchive
Created by Jeffrey Crouse on 3/19/15.
Copyright (c) 2015 __MyCompanyName__. All rights reserved.

https://github.com/jeffcrouse/ofxZipArchive/blob/master/LICENSE
The MIT License (MIT)

Copyright (c) 2015 Jeff Crouse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Backed by kuba--/zip (see libs/zip), which replaced the bundled Poco build.
*/
#pragma once

#include "ofMain.h"

#include "zip.h"

// ----------------------------------------------------------
class LatkZip {

    public:
        LatkZip() { }
        ~LatkZip();

        // archives are read into memory, so a LatkZip owns its buffer and can't be copied
        LatkZip(const LatkZip&) = delete;
        LatkZip& operator=(const LatkZip&) = delete;

        bool open(string zipPath);
        bool openBuffer(const ofBuffer& zipData);
        void close();

        vector<string> list();
        ofBuffer getFile(string fileName);
        bool unzipTo(string destination);

        static bool compress(string folderPath, string zipPath, bool recursive=true, bool excludeRoot=true, int level=ZIP_DEFAULT_COMPRESSION_LEVEL);
        static bool compressBuffer(const ofBuffer& data, string entryName, string zipPath, int level=ZIP_DEFAULT_COMPRESSION_LEVEL);

    protected:
        struct zip_t* archive = nullptr;
        ofBuffer buffer; // backs the archive while it's open
        bool bOpened = false;

};
