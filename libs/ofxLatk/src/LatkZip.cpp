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
#include "LatkZip.h"

// ------------------------------------------------------
namespace {

    // gathers (absolute path, entry name) pairs for everything under folderPath
    void collectFiles(const string& folderPath, const string& prefix, bool recursive, vector<pair<string, string>>& files) {
        ofDirectory dir(folderPath);
        if (!dir.exists()) {
            ofLogError("LatkZip") << folderPath << " doesn't exist";
            return;
        }
        dir.listDir();
        dir.sort();

        for (int i = 0; i < (int) dir.size(); i++) {
            ofFile file = dir.getFile(i);
            string name = file.getFileName();

            if (file.isDirectory()) {
                if (recursive) collectFiles(file.getAbsolutePath(), prefix + name + "/", true, files);
            } else {
                files.push_back(make_pair(file.getAbsolutePath(), prefix + name));
            }
        }
    }

    int onExtractEntry(const char* fileName, void* arg) {
        ofLogNotice("LatkZip") << "Unzipped: " << fileName;
        (*(int*) arg)++;
        return 0;
    }

}

// ------------------------------------------------------
LatkZip::~LatkZip() {
    close();
}

// ----------------------------------------------------------
bool LatkZip::open(string zipPath) {
    zipPath = ofToDataPath(zipPath);
    ofLogNotice("LatkZip") << "Opening " << zipPath;

    ofBuffer zipData = ofBufferFromFile(zipPath, true);
    if (zipData.size() == 0) {
        ofLogError("LatkZip") << "Couldn't open " << zipPath;
        return false;
    }

    return openBuffer(zipData);
}

// ----------------------------------------------------------
bool LatkZip::openBuffer(const ofBuffer& zipData) {
    close();

    buffer = zipData;
    archive = zip_stream_open(buffer.getData(), buffer.size(), 0, 'r');
    if (!archive) {
        ofLogError("LatkZip") << "Couldn't read zip archive";
        buffer.clear();
        return false;
    }

    bOpened = true;
    return true;
}

// ----------------------------------------------------------
void LatkZip::close() {
    if (archive) {
        zip_stream_close(archive);
        archive = nullptr;
    }
    buffer.clear();
    bOpened = false;
}

// ----------------------------------------------------------
vector<string> LatkZip::list() {
    vector<string> files;
    if (!bOpened) {
        ofLogWarning("LatkZip") << "Archive not opened";
        return files;
    }

    ssize_t total = zip_entries_total(archive);
    for (ssize_t i = 0; i < total; i++) {
        if (zip_entry_openbyindex(archive, (size_t) i) < 0) continue;

        if (!zip_entry_isdir(archive)) {
            string fname = zip_entry_name(archive);
            files.push_back(fname);
            ofLogNotice("LatkZip") << fname;
        }
        zip_entry_close(archive);
    }
    return files;
}

// ----------------------------------------------------------
ofBuffer LatkZip::getFile(string fileName) {
    if (!bOpened) {
        ofLogWarning("LatkZip") << "Archive not opened";
        return ofBuffer();
    }

    int err = zip_entry_open(archive, fileName.c_str());
    if (err < 0) {
        ofLogError("LatkZip") << fileName << " doesn't exist in archive: " << zip_strerror(err);
        return ofBuffer();
    }

    ofLogNotice("LatkZip") << "Uncompressing " << fileName << " size = " << zip_entry_size(archive);

    void* data = nullptr;
    size_t size = 0;
    ssize_t read = zip_entry_read(archive, &data, &size);
    zip_entry_close(archive);

    if (read < 0) {
        ofLogError("LatkZip") << "Failed to read " << fileName << ": " << zip_strerror((int) read);
        return ofBuffer();
    }

    ofBuffer buf((const char*) data, size);
    free(data);
    return buf;
}

// ----------------------------------------------------------
bool LatkZip::unzipTo(string destination) {
    destination = ofToDataPath(destination);

    if (!bOpened) {
        ofLogWarning("LatkZip") << "Archive not opened";
        return false;
    }

    ofLogNotice("LatkZip") << "Unzipping archive to " << destination;

    int extracted = 0;
    int err = zip_stream_extract(buffer.getData(), buffer.size(), destination.c_str(), onExtractEntry, &extracted);
    if (err < 0) {
        ofLogError("LatkZip") << "Failed to unzip: " << zip_strerror(err);
        return false;
    }

    return extracted > 0;
}

// ------------------------------------------------------
bool LatkZip::compress(string folderPath, string zipPath, bool recursive, bool excludeRoot, int level) {

    folderPath = ofToDataPath(folderPath);
    zipPath = ofToDataPath(zipPath);

    ofLogNotice("LatkZip") << "Compressing " << folderPath << " to " << zipPath;

    string prefix = "";
    if (!excludeRoot) prefix = ofFilePath::getFileName(ofFilePath::removeTrailingSlash(folderPath)) + "/";

    vector<pair<string, string>> files;
    collectFiles(folderPath, prefix, recursive, files);
    if (files.size() == 0) {
        ofLogError("LatkZip") << "Nothing to compress in " << folderPath;
        return false;
    }

    struct zip_t* zip = zip_open(zipPath.c_str(), level, 'w');
    if (!zip) {
        ofLogError("LatkZip") << "Couldn't open " << zipPath;
        return false;
    }

    bool returns = true;
    for (int i = 0; i < (int) files.size(); i++) {
        int err = zip_entry_open(zip, files[i].second.c_str());
        if (err < 0) {
            ofLogError("LatkZip") << "Failed to add " << files[i].second << ": " << zip_strerror(err);
            returns = false;
            continue;
        }

        err = zip_entry_fwrite(zip, files[i].first.c_str());
        if (err < 0) {
            ofLogError("LatkZip") << "Failed to compress " << files[i].first << ": " << zip_strerror(err);
            returns = false;
        } else {
            ofLogNotice("LatkZip") << "Zipped " << files[i].second;
        }
        zip_entry_close(zip);
    }

    zip_close(zip);
    return returns;
}

// ------------------------------------------------------
bool LatkZip::compressBuffer(const ofBuffer& data, string entryName, string zipPath, int level) {

    zipPath = ofToDataPath(zipPath);

    ofLogNotice("LatkZip") << "Compressing " << entryName << " to " << zipPath;

    struct zip_t* zip = zip_open(zipPath.c_str(), level, 'w');
    if (!zip) {
        ofLogError("LatkZip") << "Couldn't open " << zipPath;
        return false;
    }

    bool returns = true;
    int err = zip_entry_open(zip, entryName.c_str());
    if (err < 0) {
        ofLogError("LatkZip") << "Failed to add " << entryName << ": " << zip_strerror(err);
        returns = false;
    } else {
        err = zip_entry_write(zip, data.getData(), data.size());
        if (err < 0) {
            ofLogError("LatkZip") << "Failed to compress " << entryName << ": " << zip_strerror(err);
            returns = false;
        }
        zip_entry_close(zip);
    }

    zip_close(zip);
    return returns;
}
