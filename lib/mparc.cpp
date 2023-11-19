/**
 * @brief The new C++11 (with a bit of C++17 if available) rewrite of MPARC from the spaghetti C99 code. Source file.
 * @author MXPSQL
 * @file mparc.hpp
 * 
 * @copyright
 * 
 * MIT License
 * 
 * Copyright (c) 2022-2023 MXPSQL
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * 
 * MPARC, A rewrite of MPAR IN C, a dumb archiver format
 * Copyright (C) 2022-2023 MXPSQL
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * 
 * Copyright 2022-2023 MXPSQL
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mparc.hpp"
#include "nlohmann/json.hpp"
#include "base64.hpp"


// OS dependent stuff
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__unix) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__CYGWIN__)
    #define PLATFORM_POSIX
#elif defined(__hpux)
    #define PLATFORM_POSIX
#elif defined(_AIX)
    #define PLATFORM_POSIX
#elif defined(__sun) && defined(__SVR4)
    #define PLATFORM_POSIX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
    #define PLATFORM_POSIX
#else
    #define PLATFORM_NONE
#endif

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <fileapi.h>
#include <pathcch.h>
#elif defined(PLATFORM_POSIX)
#include <unistd.h>
#include <sys/stat.h>
#include "altnftw.h" // alternative nftw with context. Based of dirent.
#include <fcntl.h>
#include <libgen.h>
#endif


// DEFS
#define UNUSED(expr) (static_cast<void>(expr))
#define make_parsereturn(v, m) (std::make_pair<std::vector<Status>, std::map<std::string, Status>>(v, m))


// NAMESPACE ALIASING
namespace MPARC11 = MXPSQL::MPARC11;
namespace Utils = MPARC11::Utils;
using ByteArray = MPARC11::ByteArray;
using Status = MPARC11::Status;
using ParseReturn = MPARC11::ParseReturn;
using MPARC = MPARC11::MPARC;
using Entry = MPARC11::Entry;


using json = nlohmann::json;
namespace b64 = base64;


#ifdef MXPSQL_MPARC_FSLIB
    #if MXPSQL_MPARC_FSLIB == 1
        namespace stdfs = std::filesystem;
        namespace fslib = stdfs;
    #elif MXPSQL_MPARC_FSLIB == 2
        namespace ghcfs = ghc::filesystem;
        namespace fslib = ghcfs;
    #elif MXPSQL_MPARC_FSLIB == 3
        namespace boostfs = boost::filesystem;
        namespace fslib = boostfs;
    #endif
#endif



// Enforce function prototype check for utils
// This will make sure that the Utils function matches the prototype in MPARC.
static MPARC::file_splitter _split = Utils::fileSplitterDefaultImplementation;
static MPARC::directory_maker _mkdirer = Utils::makeDirectoryDefaultImplementation;
static MPARC::directory_checker _mkcheck = Utils::isDirectoryDefaultImplementation;
static MPARC::directory_scanner _scanner = Utils::scanDirectoryDefaultImplementation;


// prototyping
static Status construct_entries(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_header(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_footer(MPARC& archive, std::string& output, MPARC::version_type ver);

static ParseReturn parse_header(MPARC& archive, std::string header_input);
static ParseReturn parse_entries(MPARC& archive, std::string entry_input);
static ParseReturn parse_footer(MPARC& archive, std::string footer_input);

static std::string encrypt_xor(std::string input, std::string key);
static std::string encrypt_rot(std::string input, std::vector<int> key);

[[noreturn]] static inline void my_unreachable(...);



// Include checksum and hashes
#include "sum.hpp"

// Include ciphers
#include "ciphers.hpp"


namespace{ // Good place to store global voids (unused stuff)
    void do_nothing_store(){
        do_nothing_crypt();
    }
}






// Utilities
ByteArray Utils::StringToByteArray(std::string content){
    return ByteArray(content.begin(), content.end());
}

std::string Utils::ByteArrayToString(ByteArray bytearr){
    return std::string(bytearr.begin(), bytearr.end());
}

bool Utils::VersionTypeToString(MPARC::version_type input, std::string& output){
    output = std::to_string(input);
    return true;
}

bool Utils::StringToVersionType(std::string input, MPARC::version_type& output){
    try{
        output = std::stoull(input);
        return true;
    }
    catch(...){
        return false;
    }
}

Status::Code Utils::isDirectoryDefaultImplementation(std::string path){
    #ifdef MXPSQL_MPARC_FSLIB
        #if MXPSQL_MPARC_FSLIB == 1 || MXPSQL_MPARC_FSLIB == 2 || MXPSQL_MPARC_FSLIB == 3 // C++17 fs
        try{
            return (
                fslib::is_directory(fslib::path(path)) ?
                Status::Code::OK :
                Status::Code::ISDIR
            );
        }
        catch(fslib::filesystem_error){
            return Status::Code::FERROR;
        }
        #else
        UNUSED((path));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #else
        #if defined(PLATFORM_WINDOWS)

        DWORD attributes = GetFileAttributesA(path.c_str());
        return (
            (attributes != INVALID_FILE_ATTRIBUTES) ?
            (
                ( (attributes & FILE_ATTRIBUTE_DIRECTORY) ? Status::Code::ISDIR : Status::Code::OK )
            ) : Status::Code::FERROR
        );

        #elif defined(PLATFORM_POSIX)

        struct stat path_stat;
        if(stat(path.c_str(), &path_stat) != 0) return Status::Code::FERROR;
        return (
            (S_ISDIR(path_stat.st_mode)) ? Status::Code::ISDIR : Status::Code::OK
        );

        #else

        UNUSED((path));

        return Status::Code::NOT_IMPLEMENTED;

        #endif
    #endif
}

Status::Code Utils::scanDirectoryDefaultImplementation(std::string path, std::vector<std::string>& out, bool recursive, bool absolute){
    #ifdef MXPSQL_MPARC_FSLIB
    // UNUSED((absolute));
        #if MXPSQL_MPARC_FSLIB == 1 || MXPSQL_MPARC_FSLIB == 2 || MXPSQL_MPARC_FSLIB == 3 // C++17 fs
        fslib::path pef;
        try{
            if(recursive){
                for(const fslib::directory_entry& dir_entry : fslib::recursive_directory_iterator(path)){
                    pef = dir_entry.path();
                    if(absolute) pef = fslib::absolute(pef);
                    else pef = fslib::proximate(pef, fslib::current_path());
                    pef = fslib::canonical(pef);
                    out.push_back(pef.string());
                }
            }
            else{
                for(const fslib::directory_entry& dir_entry : fslib::directory_iterator(path)){
                    pef = dir_entry.path();
                    if(absolute) pef = fslib::absolute(pef);
                    else pef = fslib::proximate(pef, fslib::current_path());
                    pef = fslib::canonical(pef);
                    out.push_back(pef.string());
                }
            }
        }
        catch(fslib::filesystem_error){
            return Status::Code::FERROR;
        }
        return Status::Code::OK;
        #else
        // No Std FS implementation
        UNUSED((path));
        UNUSED((out));
        UNUSED((recursive));
        UNUSED((absolute));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #else
    
    UNUSED((absolute));

    std::function<Status::Code(std::string)> readDirRecursive;

        #if defined(PLATFORM_WINDOWS)

        readDirRecursive = [&](const std::string& dir) {
            std::string searchPath = "";
            
            { // Join path
                std::wstring wpath = std::wstring(path.begin(), path.end());
                std::wstring wsearch = L"*";
                PWSTR wsearchPath = NULL;
                HRESULT res = PathAllocCombine(wpath.c_str(), wsearch.c_str(), 0x00, &wsearchPath);
                if(res != S_OK){
                    return Status::Code::FERROR;
                }
                std::wstring wsearchPathC = wsearchPath;
                if(LocalFree(wsearchPath) != NULL){ // NULL return indicates success. A live handle returned is a failure.
                    return Status::Code::INTERNAL; // This is bad. We can't deallocate memory. Abort to prevent more leaks.
                }
                searchPath = std::string(wsearchPathC.begin(), wsearchPathC.end());
            }
            WIN32_FIND_DATAA fileData;
            HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fileData);
    
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    std::string fileName = fileData.cFileName;
                    if (fileName != "." && fileName != "..")
                    {
                        std::string fullPath = "";
                        { // Soon
                            std::wstring wdir = std::wstring(dir.begin(), dir.end());
                            std::wstring wfile = std::wstring(fileName.begin(), fileName.end());
                            PWSTR wfullNam = NULL;
                            HRESULT res = PathAllocCombine(wdir.c_str(), wfile.c_str(), 0x00, &wfullNam);
                            if(res != S_OK){
                                return Status::Code::FERROR;
                            }
                            std::wstring wfullPath = wfullNam;
                            if(LocalFree(wfullNam) != NULL){ // NULL return indicates success. A live handle returned is a failure.
                                return Status::Code::INTERNAL; // This is bad. We can't deallocate memory. Abort to prevent more leaks.
                            }
                            fullPath = std::string(wfullPath.begin(), wfullPath.end());
                        }

                        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        {
                            if (recursive)
                            {
                                readDirRecursive(fullPath);
                            }
                        }
                        else
                        {
                            out.push_back(fullPath);
                        }
                    }
                } while (FindNextFileA(hFind, &fileData));
    
                FindClose(hFind);

                return Status::Code::OK;
            }
            else{
                return Status::Code::FERROR;
            }
        };

        #elif defined(PLATFORM_POSIX)

        // old implementation
        // readDirRecursive = [&](const std::string& dir) {
        //     DIR* dirp = opendir(dir.c_str());
        //     if (dirp == nullptr) {
        //         return Status::FERROR;
        //     }
        // 
        //     struct dirent* entry;
        //     while ((entry = readdir(dirp)) != nullptr) {
        //         std::string fileName = entry->d_name;
        //         if (fileName != "." && fileName != "..") {
        //             std::string fullPath = dir + "/" + fileName;
        //             if (entry->d_type == DT_DIR) {
        //                 if (recursive) {
        //                     Status::Code status = readDirRecursive(fullPath);
        //                     if (status != Status::Code::OK) {
        //                         closedir(dirp);
        //                         return status;
        //                     }
        //                 }
        //             }
        //             else {
        //                 out.push_back(fullPath);
        //             }
        //         }
        //     }
        // 
        //     closedir(dirp);
        //     return Status::Code::OK;
        // };

        readDirRecursive = [&](const std::string& dir) {
            errno = 0;

            std::pair<std::vector<std::string>*, bool> recpair = std::make_pair(&out, recursive); // pass multiple info

            auto cb = [](const char* path, const struct stat* statinfo, int info, struct FTW* ftwinfo, void* ctx) -> int {
                UNUSED(statinfo);
                std::pair<std::vector<std::string>*, bool>* recypair = static_cast<std::pair<std::vector<std::string>*, bool>*>(ctx);
                if(!recypair->second && ftwinfo->level > 0) return 1;
                std::vector<std::string>* outy = recypair->first;
                switch(info){
                    case FTW_D:
                    case FTW_DNR:
                    case FTW_NS:
                    case FTW_SLN:
                        break;
                    case FTW_F:
                        outy->push_back(std::string(path));
                };
                return 0;
            };

            int stat = altnftw(dir.c_str(), cb, 2, 0, static_cast<void*>(&recpair));
            if(stat < 0 && errno != 0){
                return Status::Code::FERROR;
            }

            return Status::Code::OK;
        };

        #else

        readDirRecursive = [&](const std::string& dir){
            UNUSED((dir));
            UNUSED((absolute));
            UNUSED((recursive));
            return Status::Code::NOT_IMPLEMENTED;
        };
        
        #endif

    return readDirRecursive(path);

    #endif
}

Status::Code Utils::makeDirectoryDefaultImplementation(std::string path, bool overwrite){
    UNUSED((overwrite));
    // return Status::Code::NOT_IMPLEMENTED; // DISABLE THIS

    #ifdef MXPSQL_MPARC_FSLIB
        UNUSED((overwrite));
        #if MXPSQL_MPARC_FSLIB == 1 || MXPSQL_MPARC_FSLIB == 2 || MXPSQL_MPARC_FSLIB == 3 // C++17 fs
        try{
            return (
                fslib::create_directory(fslib::path(path)) ?
                Status::Code::OK :
                Status::Code::FERROR
            );
        }
        catch(fslib::filesystem_error){
            return Status::Code::FERROR;
        }
        #else
        UNUSED((path));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #else
        #if defined(PLATFORM_WINDOWS)

        int status = CreateDirectoryA(path.c_str(), NULL);
        return (
            (status == 0) ? Status::Code::FERROR :
            Status::Code::OK
        );

        #elif defined(PLATFORM_POSIX)

        return (
            (mkdir(path.c_str(), 0777) == 0) ? Status::Code::OK :
            Status::Code::FERROR
        );

        #else
        UNUSED((path));
        UNUSED((overwrite));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #endif
}

Status::Code Utils::fileSplitterDefaultImplementation(std::string path, std::string& dname, std::string& bname){
    #ifdef MXPSQL_MPARC_FSLIB
        #if MXPSQL_MPARC_FSLIB == 1 || MXPSQL_MPARC_FSLIB == 2 || MXPSQL_MPARC_FSLIB == 3 // C++17 fs
        try{
            return (
                fslib::create_directory(fslib::path(path)) ?
                Status::Code::OK :
                Status::Code::FERROR
            );
        }
        catch(fslib::filesystem_error){
            return Status::Code::FERROR;
        }
        #else
        UNUSED((path));
        UNUSED((dname));
        UNUSED((bname));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #else
        #if defined(PLATFORM_WINDOWS)

        // Convert path to wide string for Windows API
        std::wstring wpath(path.begin(), path.end());

        // Use Windows API to split the path
        wchar_t drive[_MAX_DRIVE];
        wchar_t dir[_MAX_DIR];
        wchar_t fname[_MAX_FNAME];
        wchar_t ext[_MAX_EXT];

        if (_wsplitpath_s(wpath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT) != 0) {
            return Status::Code::FERROR; // Error
        }

        // Convert wide strings back to regular strings
        std::wstring wdname = std::wstring(drive) + std::wstring(dir);
        std::wstring wbname = std::wstring(fname) + std::wstring(ext);

        dname = std::string(wdname.begin(), wdname.end());
        bname = std::string(wbname.begin(), wbname.end());

        return Status::Code::OK;

        #elif defined(PLATFORM_POSIX)

        try{
            // Using smart pointers for dirname and basename
            std::unique_ptr<char[]> dnamePtr(new char[path.size() + 1]);
            std::unique_ptr<char[]> bnamePtr(new char[path.size() + 1]);

            // Copy path to dnamePtr using a C++ loop (not strcpy)
            for (size_t i = 0; i < path.size(); ++i) {
                dnamePtr[i] = path[i];
                bnamePtr[i] = path[i];
            }
            dnamePtr[path.size()] = '\0';
            bnamePtr[path.size()] = '\0';

            // Use POSIX dirname and basename functions
            char* dir = dirname(dnamePtr.get());
            char* base = basename(bnamePtr.get());

            // Copy the result to the output strings
            dname = dir;
            bname = base;
        }
        catch(std::bad_alloc& ba){
            return Status::Code::FERROR;
        }

        return Status::Code::OK;

        #else

        UNUSED((path));
        UNUSED((dname));
        UNUSED((bname));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #endif
}


// STATUS FUNCTIONS
bool Status::isOK(){
    return getCode() == Status::Code::OK;
}

void Status::assertion(bool throw_err=true){
    if(!isOK()){
        if(throw_err){
            throw std::runtime_error(str() + ": " + std::to_string(getCode()));
        }
        else{
            std::cerr << "MPARC11[ABORT] " << str() << " (" << std::to_string(getCode()) << ")" << std::endl;
            std::abort();
        }
    }
}

std::string Status::str(Status::Code code, Status::StrFilter filt){
    Status::Code cod = Status::Code::OK;

    cod = code;

    if((filt & CONSTRUCT_FAILS) && (cod & CONSTRUCT_FAIL)){ // CONSTRUCT_FAIL stripout
        cod = static_cast<Status::Code>(cod ^ CONSTRUCT_FAIL);
    }

    if(cod & OK){
        return "OK";
    }
    else if(cod & GENERIC) {
        return "A generic error has been detected.";
    }
    else if(cod & INTERNAL) {
        return "An internal error has been detected.";
    }
    else if(cod & NOT_IMPLEMENTED) {
        return "Not implemented.";
    }
    else if(cod & FALSEV) {
        return "False";
    }
    else if(cod & INVALID_VALUE) {
        if(cod & NULL_VALUE) return "A null value has been provided at an inappropriate time.";
        return "An invalid value has been provided.";
    }
    else if(cod & KEY) {
        if(cod & KEY_EXISTS) return "The key exists.";
        if(cod & KEY_NOEXISTS) return "The key does not exist.";
        return "A generic/unknown key related error has been detected.";
    }
    else if(cod & FERROR) {
        return "A File I/O related error has been detected.";
    }
    else if(cod & ISDIR) {
        return "The object is a directory.";
    }
    else if(cod & CONSTRUCT_FAIL) {
        if(cod & CONSTRUCT_HEADER) return "Archive header failed during construction.";
        if(cod & CONSTRUCT_ENTRIES) return "Archive entries failed during construction.";
        if(cod & CONSTRUCT_FOOTER) return "Archive footer failed during construction.";
        return "Archive failed during construction.";
    }
    else if (cod & PARSE_FAIL) {
        if(cod & NOT_MPAR_ARCHIVE) return "What was parsed is not an MPAR archive.";
        if(cod & CHECKSUM_ERROR) return "The checksum did not match or it was not obtained successfully.";
        if(cod & VERSION_ERROR) return "The version either was too new, invalid or it was not obtained successfully.";
        return "A generic parsing error has been detected.";
    }
    if(cod & CRYPT_ERROR){
        if(cod & CRYPT_NONE) return "No encryption was set.";
        if(cod & CRYPT_FAIL) return "Failure was detected during cryptography.";
        return "A generic cryptography error has been detected.";
    }
    else{
        my_unreachable();
        return "Unknown code";
    }
}

std::string Status::str(Status::Code cod){
    return Status::str(cod, Status::StrFilter::NONE);
}

std::string Status::str(){
    return Status::str(getCode());
}

Status::Code Status::getCode(){
    return stat_code;
}

Status::operator bool(){
    return isOK();
}


// PARSE RETURN CLASS things
bool MPARC11::isParseReturnOk(ParseReturn parret){
    std::vector<Status> leftvec = parret.first;
    std::map<std::string, Status> rightmap = parret.second;
    for(auto s : leftvec){
        if(!s.isOK()){
            return false;
        }
    }
    for(auto kv : rightmap){
        Status statsec = kv.second;
        if(!statsec.isOK()){
            return false;
        }
    }
    return true;
}




// MAIN ARCHIVE STRUCTURE
const std::string MPARC::filename_field = "filename"; // Compatibility with the C99 library
const std::string MPARC::content_field = "blob"; // Compatibility with the C99 library
const std::string MPARC::checksum_field = "crcsum"; // Compatibility with the C99 library
const std::string MPARC::fletcher_checksum_field = "fletcher32sum";
const std::string MPARC::processed_checksum_field = "crcsum.processed";
const std::string MPARC::md5_checksum_field = "md5sum";
const std::string MPARC::sha256_checksum_field = "sha256sum";
const std::string MPARC::meta_field = "metadata";

const std::string MPARC::encrypt_meta_field = "encrypt"; // Compatibility with the C99 library
const std::string MPARC::extra_meta_field = "extra";

const std::string MPARC::magic_number = "MXPSQL's Portable Archive"; // Compatibility with the C99 library


std::map<std::string, std::string> MPARC::dummy_extra_metadata{{"Key", "Value"}};

MPARC::MPARC(){
    // Unused static var stuff go here
    UNUSED((_split));
    UNUSED((_mkdirer));
    UNUSED((_mkcheck));
    UNUSED((_scanner));
};
MPARC::MPARC(std::vector<std::string> entries){
    Status stat;
    for(std::string entry : entries){
        if(!(stat = push(entry, true))){
            stat.assertion(true);
        }
    }
}
MPARC::MPARC(MPARC& other){
    std::vector<std::string> entries;
    Status stat;
    if(!(stat = other.list(entries))){
        stat.assertion(true);
    }
}

void MPARC::init(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    do_nothing_store();
}


Status MPARC::clear(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::vector<std::string> ee;
    Status stat;
    if(!(stat = list(ee))){
        return stat;
    }

    for(auto e : ee){
        if(!(stat = pop(e))){
            return stat;
        }
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::exists(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return Status(
        (this->my_code = (
                static_cast<Status::Code>
                ( 
                    (entries.count(name) == 0 || entries.find(name) == entries.end()) ?
                    (Status::Code::KEY | Status::Code::KEY_NOEXISTS) :
                    (Status::Code::OK)
                )
            )
        )
    );
}


Status MPARC::push(std::string name, Entry entry, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(!overwrite && exists(name)){
        return Status(
            (this->my_code = static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS))
        );
    }

    entries[name] = entry;

    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::push(std::string name, ByteArray content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Entry entreh;
    entreh.content = content;
    return push(name, entreh, overwrite);
}

Status MPARC::push(std::string name, std::string content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return push(name, Utils::StringToByteArray(content), overwrite);
}

Status MPARC::push(std::string name, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);

    std::string content;

    {
        std::ifstream ifs(name, std::ios::binary);
        if(!ifs.is_open() || !ifs.good()){
            return Status(Status::Code::FERROR);
        }

        std::stringstream ssbuf;
        ssbuf << ifs.rdbuf();


        content = ssbuf.str();
    }
    
    return push(name, content, overwrite);
}


Status MPARC::pop(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    entries.erase(entries.find(name));
    
    return Status(
        (this->my_code = Status::Code::OK)  
    );
}


Status MPARC::peek(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}

Status MPARC::peek(std::string name, Entry& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = peek(name);
    if(!stat) return stat;

    if(!(stat = peek(name))) return stat;

    try{
        output = entries.at(name);
    }
    catch(...){
        return (stat = Status(
            (this->my_code = 
                static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_NOEXISTS)
            )
        ));
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}

Status MPARC::peek(std::string name, std::string* output_str, ByteArray* output_ba, std::map<std::string, std::string>* output_meta){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = peek(name);
    if(!stat) return stat;

    if(!(stat = peek(name))) return stat;

    Entry entreh;
    ByteArray ba;
    std::map<std::string, std::string> meta;

    if(!(stat = peek(name, entreh))){
        return stat;
    }

    ba = entreh.content;
    meta = entreh.metadata;
    
    if(output_ba){
        *output_ba = ba;
    }
    if(output_str){
        *output_str = Utils::ByteArrayToString(ba);
    }
    if(output_meta){
        *output_meta = meta;
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}

Status MPARC::pull(std::string name, std::string path){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = peek(name);
    if(!stat) return stat;

    if(!(stat = peek(name))) return stat;

    if(path.empty()) path = name;

    {
        Entry ent;
        if(!(stat = peek(name, ent))) return stat;

        {
            std::ofstream f;
            f.open(path, std::ios::binary | std::ios::out);
            if(!(f.is_open() && f.good())) return Status((this->my_code = Status::Code::FERROR));    
            f.write(reinterpret_cast<char*>(&ent.content[0]), ent.content.size());
            f.close();
        }
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}


Status MPARC::swap(std::string name, std::string name2){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    if(
        !(
        (stat = peek(name)) &&
        (stat = peek(name2))
        )
    ) return stat;

    Entry entreh, entreh2;

    if(
        !(
            (
                stat = peek(name, entreh)
            ) &&
            (
                stat = peek(name2, entreh2)
            )
        )
    ) return stat;

    if(
        !(
            (
                stat = pop(name)
            ) &&
            (
                stat = pop(name2)
            )
        )
    ) return stat;

    if(
        !(
            (
                stat = push(name, entreh2, true)
            ) &&
            (
                stat = push(name2, entreh, true)
            )
        )
    ) return stat;

    return stat;
}

Status MPARC::copy(std::string name, std::string name2, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    {
        Status e = (
            (
                (stat = peek(name)) && !overwrite
            ) ?
            static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS) :
            Status::Code::OK
        );

        if(
            !(
                (stat = peek(name)) &&
                (stat = e)
            )
        ) return stat;
    }

    Entry entreh;

    if(
        !(stat = peek(name, entreh))
    ) return stat;

    if(
        !(stat = pop(name2))
    ) return stat;  

    if(
        !(stat = push(name2, entreh, overwrite))
    )  return stat;

    return stat;
}

Status MPARC::rename(std::string name, std::string name2, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    {
        Status e = (
            (
                (stat = peek(name)) && !overwrite
            ) ?
            static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS) :
            Status::Code::OK
        );

        if(
            !(
                (stat = peek(name)) &&
                (stat = e)
            )
        ) return stat;
    }

    Entry entreh;

    if(
        !(stat = peek(name, entreh))
    ) return stat;

    if(
        !(
            (stat = pop(name)) &&
            (stat = pop(name2))
        )
    ) return stat;  

    if(
        !(
            (stat = push(name2, entreh, overwrite))
        )
    )  return stat;

    return stat;
}


Status MPARC::list(std::vector<std::string>& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    output.clear();

    for(auto pair : entries){
        output.push_back(pair.first);
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::extra_metadata(std::map<std::string, std::string>& output, std::map<std::string, std::string>& input){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(std::addressof(input) != std::addressof(MPARC::dummy_extra_metadata)) this->extra_meta_data = input;
    
    if(std::addressof(output) != std::addressof(MPARC::dummy_extra_metadata)) output = this->extra_meta_data;

    return Status(
        this->my_code = Status::Code::OK
    );
}


Status MPARC::construct(std::string& output, MPARC::version_type ver){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::stringstream archive;
    Status stat;

    if(ver < 1 || ver > MPARC::mpar_version){
        return Status(
            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::VERSION_ERROR)
        );
    }

    { // Construct the header
        std::string header = "";
        if(!(stat = construct_header(*this, header, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | Status::Code::CONSTRUCT_HEADER | stat.getCode()));
        }
        archive << header;
    }
    { // Construct the tnreis
        std::string entries = "";
        if(!(stat = construct_entries(*this, entries, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | Status::Code::CONSTRUCT_ENTRIES | stat.getCode()));
        }
        archive << entries;
    }
    { // Construct the footer
        std::string footer = "";
        if(!(stat = construct_footer(*this, footer, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | Status::Code::CONSTRUCT_FOOTER | stat.getCode()));
        }
        archive << footer;
    }

    output = archive.str();


    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::construct(std::string &output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return construct(output, MPARC::mpar_version);
}

Status MPARC::write(std::ostream& strem){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::string file;
    Status code = construct(file);
    if(code.isOK()){
        strem << file;
    }
    return Status(
        (this->my_code = code.getCode())
    );
}

Status MPARC::write(std::string filepath){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::ofstream fstrem(filepath, std::ios::binary);
    if(!fstrem.is_open() || !fstrem.good()) {
        fstrem.close();
        return Status(
            (this->my_code = Status::Code::FERROR)  
        );
    }

    Status code = write(fstrem);
    if(!code.isOK()){
        return Status(
            (this->my_code = code.getCode())  
        );
    }

    if(!fstrem.is_open() || !fstrem.good()) {
        fstrem.close();
        return Status(
            (this->my_code = Status::Code::FERROR)  
        );
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::extract(bool absolute, std::string directory, process_handler handler, file_splitter splitter, directory_maker mkdirer, directory_checker dirchk){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    UNUSED((absolute));
    UNUSED((directory));

    Status stat;

    std::vector<std::string> entries;
    stat = list(entries);
    if(!stat.isOK()){
        return Status(
            (this->my_code = stat.getCode())
        );
    }

    for(auto filename : entries){
        if(handler) handler(filename, false); // Invoke the handler pre-processing.

        ByteArray fbytes;
        stat = peek(filename, nullptr, &fbytes, nullptr);
        if(!stat.isOK()){
            return Status(
                (this->my_code = stat.getCode())
            );
        }

        { // Directory creation thing
            std::function<Status::Code(std::string)> direr;
            
            direr = [&direr, &splitter, &mkdirer, &dirchk](std::string fdir) -> Status::Code {
                std::string dname, bname;
                Status::Code Result = splitter(fdir, dname, bname);
                if (Result != Status::Code::OK) {
                    return Result; // Return error code
                }

                UNUSED((bname));

                if(!dname.empty()){
                    Result = dirchk(dname);
                    if(Result == Status::Code::ISDIR || Result == Status::Code::OK){
                        return Status::Code::OK;
                    }
                    else if(Result == Status::Code::KEY_NOEXISTS){
                        ;
                    }
                    else{
                        return Status::Code::FERROR;
                    }

                    Result = direr(dname);
                    if(Result != Status::Code::OK){
                        return Result;
                    }
                }

                Result = mkdirer(dname, true);
                if(Result != Status::Code::OK){
                    return Result;
                }

                return Status::Code::OK; // Success
            };

            { // The actual thing
                std::string dirnam;
                std::string _bname;

                stat = splitter(filename, dirnam, _bname);
                if(!stat.isOK()){
                    return Status(
                        (this->my_code = stat.getCode())
                    );
                }
                
                UNUSED((_bname));

                if(!dirnam.empty()){
                    stat = direr(dirnam);
                    if(!stat.isOK()){
                        return Status(
                            (this->my_code = stat.getCode())
                        );
                    }
                }
            }
        }

        // Create the file
        {
            std::ofstream f(filename, std::ios::binary);
            if(!f.good() || !f.is_open()){
                return Status(
                    (this->my_code = Status::Code::FERROR)
                );
            }

            f.write(reinterpret_cast<char*>(&fbytes[0]), fbytes.size());

            f.close();

            if(!f.good() || !f.is_open()){
                return Status(
                    (this->my_code = Status::Code::FERROR)
                );
            }            
        }

        if(handler) handler(filename, true); // Invoke the handler post-writing
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


ParseReturn MPARC::parse(std::string input){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);

    ParseReturn stat;

    std::string header = "";
    std::string entries = "";
    std::string footer = "";

    // This section would be handled by a function that would perform steganography by searching for the archive if embedded in files, but it is not yet implemented.
    // Maybe never will, maybe will be implemented, I don't know, lets see later.
    // I realized that the function would require to be extremly smart at finding the archive, it cannot be some "find a character" thing.
    // "Scan a block to see if it is an archive, then move one character if not an archive, put it in if its an archive" would be nice, but inefficient.
    // I really need a good/better search function.
    // For now, it will just be a cranker input.
    std::string searchInput = input;

    // find the header and footer marks
    size_t header_marksep_pos = searchInput.find(MPARC::post_header_separator);
    size_t footer_marksep_pos = searchInput.find_last_of(MPARC::end_of_entries_separator);

    // Check if either both marker exist and that the header marker position is before the footer marker position
    if(header_marksep_pos == std::string::npos || footer_marksep_pos == std::string::npos || header_marksep_pos >= footer_marksep_pos){
        return make_parsereturn(
        {
            Status( 
                (this->my_code = static_cast<Status::Code>(
                    Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE
                    )
                )
            )
        }, {});
    }

    // Slice the strings into their parts
    header = searchInput.substr(0, header_marksep_pos);
    entries = searchInput.substr(header_marksep_pos+1, (footer_marksep_pos-header_marksep_pos-1));
    footer = searchInput.substr(footer_marksep_pos+1, (input.size()-footer_marksep_pos));

    bool is_ok = true;
    // Parse the header
    {
        stat = parse_header(*this, header);
        is_ok = isParseReturnOk(stat);
        if(!is_ok){
            this->my_code = Status::Code::PARSE_FAIL;
            return stat;
        }
    }

    // Parse the entries
    {
        stat = parse_entries(*this, entries);
        is_ok = isParseReturnOk(stat);
        if(!is_ok){
            this->my_code = Status::Code::PARSE_FAIL;
            return stat;
        }
    }

    // Parse the footer
    {
        stat = parse_footer(*this, footer);
        is_ok = isParseReturnOk(stat);
        if(!is_ok){
            this->my_code = Status::Code::PARSE_FAIL;
            return stat;
        }
    }

    return make_parsereturn({
            Status(
                (this->my_code = Status::Code::OK)
            )
        }, {});
}

ParseReturn MPARC::read(std::istream& stram){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::ostringstream foss;
    foss << stram.rdbuf();
    ParseReturn code = parse(foss.str());
    return code;
}

ParseReturn MPARC::read(std::string filepath){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::ifstream fstrem(filepath, std::ios::binary);
    if(!fstrem.is_open() || !fstrem.good()) {
        fstrem.close();
        return make_parsereturn({Status(
            (this->my_code = Status::Code::FERROR)  
        )}, {});
    }

    ParseReturn code = read(fstrem);
    if(!isParseReturnOk(code)){
        return code;
    }

    if(!fstrem.is_open() || !fstrem.good()) {
        fstrem.close();
        return make_parsereturn({Status(
            (this->my_code = Status::Code::FERROR)  
        )}, {});
    }

    return make_parsereturn({Status(
        (this->my_code = Status::Code::OK)
    )}, {});
}

Status MPARC::scan(bool absolute, std::string directory, process_handler handler, bool recursive, bool overwrite, directory_scanner scanner, directory_checker dirchk){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::vector<std::string> pafs;
    Status stat = scanner(directory, pafs, recursive, absolute);

    if(!stat.isOK()){
        return Status(
            (this->my_code = stat.getCode())
        );
    }

    for(auto paf : pafs){
        Status::Code dchk = dirchk(paf);
        if(dchk == Status::Code::OK){
            if(handler) handler(paf, false);
            stat = push(paf, overwrite);
            if(!stat.isOK()){
                return Status(
                    (this->my_code = stat.getCode())
                );
            }
            if(handler) handler(paf, true);
        }
        else if(dchk == Status::Code::ISDIR){;}
        else{
            return dchk;
        }
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::get_status(Status& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    output = this->my_code;
    return Status::Code::OK;
}

Status MPARC::isOK(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    get_status(stat);
    return(
        (stat.isOK()) ?
        Status::Code::OK :
        Status::Code::FALSEV
    );
}


Status MPARC::set_xor_encryption(std::string key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->XOR_key = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_xor_encryption(std::string& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->XOR_key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::set_rot_encryption(std::vector<int> key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->ROT_key = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_rot_encryption(std::vector<int>& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->ROT_key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::set_camellia_encryption(std::string key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(false){ // Not now, but main stuff there
        return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::CRYPT_NONE); // Unavailable
    }
    int l = key.size();
    if(camellia_keysize(&l) == CRYPT_INVALID_KEYSIZE) return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);
    this->Camellia_k = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_camellia_encryption(std::string& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->Camellia_k;
    return (this->my_code = Status::Code::OK);
}


Status MPARC::set_locale(std::locale loc){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->locale = loc;
    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::get_locale(std::locale& locput){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    locput = this->locale;
    return Status(
        (this->my_code = Status::Code::OK)
    );
}



MPARC::operator bool(){
    return ((this->isOK() == Status::Code::OK) ? true : false);
}

MPARC::operator void*(){
    return ((this->isOK() == Status::Code::OK) ? const_cast<MPARC*>(this) : nullptr);
}

bool MPARC::operator <(MPARC& other) {
    std::vector<std::string> others;
    std::vector<std::string> mines;
    try{
        Status stat = other.list(others);
        stat.assertion(true);
        stat = this->list(mines);
        stat.assertion(true);

        return (mines.size() < others.size());
    }
    catch(...){
        return false;
    }
}

bool MPARC::operator >(MPARC& other){
    return (other < this);
}

bool MPARC::operator ==(MPARC& other) {
    std::vector<std::string> others;
    std::vector<std::string> mines;
    try{
        Status stat = other.list(others);
        stat.assertion(true);
        stat = this->list(mines);
        stat.assertion(true);

        return (mines.size() == others.size());
    }
    catch(...){
        return false;
    }
}

bool MPARC::operator !=(MPARC& other){
    return !(this == other);
}



// UTILITIES
[[noreturn]] static inline void my_unreachable(...){
    #if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
        std::unreachable(); // Very standard
    #else

        #ifdef __GNUC__ // GCC, Clang, ICC
            __builtin_unreachable(); // NO
        #elif defined(_MSC_VER) // MSVC
            __assume(false); // FALSE
        #else // NULL / ABORT
            int stupid = 21; // Stupid number for reinterpret_cast
            int* f = static_cast<int*>(NULL); // Undefined 1: Null dereference
            *f = reinterpret_cast<long int*>(&stupid); // Undefined 2+3: Pointer aliasing? By missusing C++'s reinterpret_cast? Yep, with missusing reinterpret_cast. Double uh-oh's. That other one is assigning to NULL.
            std::abort(); // Undefined '4': ABRT if not possible.
        #endif

    #endif
}



// ARCHIVE BUILDER
static Status construct_header(MPARC& archive, std::string& output, MPARC::version_type ver){
    Status stat;
    // String builder
    std::stringstream ssb;

    // Build the initial metadata
    ssb << MPARC::magic_number << MPARC::magic_number_separator;
    {
        std::string shadow_ver = "";
        Utils::VersionTypeToString(ver, shadow_ver);
        ssb << shadow_ver;
    }

    // Build the extra JSON metadata
    {
        json j = json::object();

        // Encryption field
        {
            std::string encrypt_field = ((ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION)
                ? b64::to_base64(MPARC::encrypt_meta_field)
                : MPARC::encrypt_meta_field
            );
            j[encrypt_field] = json::array();

            {
                std::string XOR_k;
                std::vector<int> ROT_k;
                std::string Camellia_ks;
                ByteArray Camellia_k;

                archive.get_xor_encryption(XOR_k);
                archive.get_rot_encryption(ROT_k);
                archive.get_camellia_encryption(Camellia_ks);

                Camellia_k = Utils::StringToByteArray(Camellia_ks);
                UNUSED((Camellia_k));

                std::vector<std::string> encrypt;

                // Initial encryption
                if(!XOR_k.empty()){
                    encrypt.push_back("XOR");
                }
                if(!ROT_k.empty()){
                    encrypt.push_back("ROT");
                }

                // Extended encryption: Camellia
                if(ver >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty()){
                    encrypt.push_back("Camellia");
                }

                j[encrypt_field] = encrypt;
            }
        }

        if(ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION){// Extra user defined metadata
            j[b64::to_base64(MPARC::extra_meta_field)] = json::object();
            std::map<std::string, std::string> extras;
            archive.extra_metadata(extras, MPARC::dummy_extra_metadata);
            for(auto extra_pair : extras){
                j[b64::to_base64(MPARC::extra_meta_field)][b64::to_base64(extra_pair.first)] = b64::to_base64(extra_pair.second);
            }
        }

        // Finally put the JSON
        ssb << MPARC::header_meta_magic_separator << j.dump();
    }
    // Cap it off
    ssb << MPARC::post_header_separator;

    output = ssb.str();
    return stat;
}

static Status construct_entries(MPARC& archive, std::string& output, MPARC::version_type ver){
    std::stringstream ssb;
    std::vector<std::string> entries;
    Status stat;

    // A typedef to make it less verbose
    using jty = std::pair<json, crc_t>; // The first side is the JSON document. The second side is the checksum of the JSON document (CRC32).

    // Sort function
    static auto sortcmp = [](const jty& lh, const jty rh){
        json jlh = lh.first;
        json jrh = rh.first;

        return jlh.at(MPARC::filename_field) > jrh.at(MPARC::filename_field);
    };

    // List the archive
    {
        stat = archive.list(entries);
        if(!stat){
            return stat;
        }
    }

    // Loop over the entries
    std::vector<jty> jentries;
    for(std::string entry : entries){
        json jentry;
        Entry entreh;
        // Read the entry
        {
            stat = archive.peek(entry, entreh);
            if(!stat){
                return stat;
            }
        }

        // Get the contents
        std::string strcontent = Utils::ByteArrayToString(entreh.content);

        { // Unprocessed checksum calculate (CRC32)
            std::string csum = "";

            crc_t crc = crc_init();

            crc = crc_update(crc, strcontent.c_str(), strcontent.length());

            crc = crc_finalize(crc);

            csum = crc_t_to_string(crc);

            jentry[MPARC::checksum_field] = csum;
        }

        if(ver >= MPARC::FLETCHER32_INITIAL_UPDATE_VERSION){ // Unprocessed checksum calculate (Fletcher32)
            std::string fsum;

            fletcher32_t fletcher = 0;

            {
                std::vector<uint16_t> uv16(strcontent.begin(), strcontent.end());

                fletcher = fletcher_t_update(&uv16[0], uv16.size());
            }

            fsum = fletcher_t_to_string(fletcher);

            jentry[MPARC::fletcher_checksum_field] = fsum;
        }

        if(ver >= MPARC::SECURE_UPDATE_VERSION){ // Calculate MD5 and SHA256
            std::string md5sum = "";
            std::string sha256sum = "";

            {
                SHA256 shasummer;
                shasummer.add(&strcontent[0], strcontent.size());
                sha256sum = shasummer.getHash();
            }

            {
                MD5 md5summer;
                md5summer.add(&strcontent[0], strcontent.size());
                md5sum = md5summer.getHash();
            }

            jentry[MPARC::md5_checksum_field] = md5sum;
            jentry[MPARC::sha256_checksum_field] = sha256sum;
        }

        std::string estrcontent = strcontent;
        {
            std::string XOR_k;
            std::vector<int> ROT_k;
            std::string Camellia_ks;
            ByteArray Camellia_k;


            archive.get_xor_encryption(XOR_k);
            archive.get_rot_encryption(ROT_k);
            archive.get_camellia_encryption(Camellia_ks);

            Camellia_k = Utils::StringToByteArray(Camellia_ks);

            ByteArray::size_type Camellia_kbl = Camellia_k.size();


            if(!XOR_k.empty()){
                estrcontent = encrypt_xor(estrcontent, XOR_k);
            }
            if(!ROT_k.empty()){
                estrcontent = encrypt_rot(estrcontent, ROT_k);
            }

            if(ver >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty() && !estrcontent.empty()){
                int ckbl = Camellia_kbl;
                int ret = camellia_keysize(&ckbl);
                if(ret == CRYPT_INVALID_KEYSIZE) return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);

                int num_rounds_camellia  = ((ckbl == 16) ? 18 : 24);

                // Make key
                symmetric_key camellia_key;
                memset(&camellia_key, '\0', sizeof(camellia_key));
                int setup_result = camellia_setup(&Camellia_k[0], ckbl, num_rounds_camellia, &camellia_key);
                if (setup_result != CRYPT_OK) {
                    return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_FAIL);
                }

                {
                    ByteArray ciphertext;
                    camellia_encrypt(estrcontent, ciphertext, &camellia_key, ckbl);
                    estrcontent = Utils::ByteArrayToString(ciphertext);
                }

                // Old jank
                /* { // Encryption magic
                    ByteArray raw;
                    raw = Utils::StringToByteArray(estrcontent);

                    const static ByteArray::size_type allignment = 4;
                    ByteArray::size_type pad_raw = raw.size() + (raw.size() % allignment); // Pad up the vector
                    raw.reserve(pad_raw+CAMELLIA_MAX_PAD_PLUS);

                    ByteArray ec;
                    ec.resize(pad_raw+CAMELLIA_MAX_PAD_PLUS);
                    std::fill(ec.begin(), ec.end(), '\0');

                    for(ByteArray::size_type i = 0; i < pad_raw; i += allignment){
                        Camellia_EncryptBlock(Camellia_kbl, &raw[i], camellia_tl, &ec[i]);
                    }

                    { // Jank packing
                        ByteArray::size_type pack_raw = ec.size() - raw.size();
                        for(ByteArray::size_type j = 0; j < pack_raw; j++){
                            if(!ec.empty()){
                                ec.pop_back();
                            }
                            else{
                                return (stat = static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_FAIL));
                            }
                        }
                    }

                    estrcontent = Utils::ByteArrayToString(ec);
                } */

                camellia_done(&camellia_key);
            }
        }

        // Put in the obvious ones (filename, contents)
        jentry[MPARC::filename_field] = b64::to_base64(entry);
        jentry[MPARC::content_field] = b64::to_base64(estrcontent);


        if(ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Put in the checksum after it is processed
            std::string csum = "";
            std::string b64content = jentry.at(MPARC::content_field);

            crc_t crc = crc_init();

            crc = crc_update(crc, b64content.c_str(), b64content.length());

            crc = crc_finalize(crc);

            csum = crc_t_to_string(crc);

            jentry[MPARC::processed_checksum_field] = csum;
        }

        if(ver >= MPARC::CAMELLIA_UPDATE_VERSION){ // Add the user defined metadata thing
            jentry[MPARC::meta_field] = json::object();
            for(auto meta : entreh.metadata){
                jentry[MPARC::meta_field][b64::to_base64(meta.first)] = b64::to_base64(meta.second);
            }
        }

        jty jentriy;
        { // Calculate the JSON's checksum, not the invidual content
            crc_t crc = crc_init();

            crc = crc_update(crc, jentry.dump().c_str(), strlen(jentry.dump().c_str()));

            crc = crc_finalize(crc);

            jentriy = std::make_pair(jentry, crc);
        }

        jentries.push_back(jentriy);
    }

    try{ // Sort the JSONs
        std::sort(jentries.begin(), jentries.end(), sortcmp);
    }
    catch(...){
        // Do nothing
    }

    {
        // Dump the entries
        for(jty jenty : jentries){
            ssb << jenty.second << MPARC::entry_checksum_content_separator << jenty.first.dump() << MPARC::entries_entry_separator;
        }
    }

    // Cap it
    ssb << MPARC::end_of_entries_separator;

    output = ssb.str();

    return stat;
}

static Status construct_footer(MPARC& archive, std::string& output, MPARC::version_type ver){
    UNUSED((archive));
    UNUSED((ver));

    std::stringstream ssb;

    // Just one character
    ssb << MPARC::end_of_archive_marker;

    output = ssb.str();

    return Status(Status::Code::OK);
}



// ARCHIVE PARSER
static ParseReturn parse_header(MPARC& archive, std::string header_input){
    ((void)archive);

    std::string magic = "";
    std::string vf_other = "";

    { // Separate the magic number and the usable content and then test it
        size_t magic_separator_pos = header_input.find(MPARC::magic_number_separator); // Grab the magic separator
        if(magic_separator_pos == std::string::npos){ // Check if it exists
            return make_parsereturn({Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            )}, {});
        }
        // Extract the magic number and the other useful ones
        magic = header_input.substr(0, magic_separator_pos);
        vf_other = header_input.substr(magic_separator_pos+1, std::string::npos);

        if(magic != MPARC::magic_number){
            return make_parsereturn({Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            )}, {});
        }
    }

    {
        std::string version = "";
        std::string meta = "";
        { // Separate the version and the metadata and then grab it
            size_t vm_header_meta_separator_pos = vf_other.find(MPARC::header_meta_magic_separator); // Grab the metadata and version separator
            if(vm_header_meta_separator_pos == std::string::npos){ // Check if it exists
                return make_parsereturn({Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                )}, {});
            }

            // Extract the version and the metadata
            version = vf_other.substr(0, vm_header_meta_separator_pos);
            meta = vf_other.substr(vm_header_meta_separator_pos+1, std::string::npos);
        }

        { // test the version
            MPARC::version_type ck_ver = 0;
            if(!Utils::StringToVersionType(version, ck_ver) || (ck_ver > MPARC::mpar_version)){
                return make_parsereturn({Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::VERSION_ERROR)
                )}, {});
            }

            archive.loaded_version = ck_ver;
        }

        { // Parse the metadata
            json j = json::parse(meta, nullptr, false);
            if(j.is_discarded()){
                return make_parsereturn({Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                )}, {});
            }

            {
                std::string encrypt_field = ((archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION)
                    ? b64::to_base64(MPARC::encrypt_meta_field)
                    : MPARC::encrypt_meta_field
                );

                if(j.contains(encrypt_field)){ // encryption thingy
                    std::vector<std::string> encrypt_algos = j.at(encrypt_field);
                    { // Check for encryption presence

                    }
                }
                else{
                    return make_parsereturn({Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    )}, {});
                }
            }

            if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Extra global metadata, only if version 2 is reached
                if(!j.contains(b64::to_base64(MPARC::extra_meta_field))){ // check if the field is available
                    return make_parsereturn({Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    )}, {});
                }

                std::map<std::string, std::string> extras;

                for(auto meta : j[b64::to_base64(MPARC::extra_meta_field)].items()){
                    extras[b64::from_base64(meta.key())] = b64::from_base64(meta.value());
                }
                archive.extra_metadata(MPARC::dummy_extra_metadata, extras);
            }
        }
    }
    return make_parsereturn({Status::Code::OK}, {});
}

static ParseReturn parse_entries(MPARC& archive, std::string entry_input){
    std::vector<std::string> lines;
    { // Read line by line
        std::stringstream ss(entry_input);
        std::string str;
        while(std::getline(ss, str, '\n')){
            lines.push_back(str);
        }
    }

    std::vector<Status> noentryerr;
    std::map<std::string, Status> entryerr;

    { // Loop over each line        
        for(auto line : lines){
            { // Trim early whitespace
                std::locale locale;
                archive.get_locale(locale);
                auto it =  std::find_if_not(line.begin(), line.end(), 
                    [&locale](char ch){ 
                        return std::isspace<char>(ch, locale); 
                    }
                );
                line.erase(line.begin(), it);
            }

            if(
                (line[0] == MPARC::comment_marker) // skip comments
                ||
                (line.empty() || line.length() < 1) // skip empty line
            )
            {
                continue;
            }

            std::string entry = "";
            { // Parse the checksum of the current JSON entry (not including the checksum)
                crc_t crc = 0;

                // Check for the presence of the checksum and get its position
                size_t checksum_entry_marksep_pos = line.find(MPARC::entry_checksum_content_separator);
                if(checksum_entry_marksep_pos == std::string::npos){
                    noentryerr.push_back(Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    ));
                    continue;
                }

                // Grab the checksum and the actual entry
                std::string checksum = line.substr(0, checksum_entry_marksep_pos);
                entry = line.substr(checksum_entry_marksep_pos+1, (line.size()-checksum_entry_marksep_pos));

                // Grab the actual checksum value
                if(!scan_crc_t_from_string(checksum, crc)){
                    noentryerr.push_back(Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    ));
                    continue;
                }

                { // Calculate and check the checksum
                    crc_t crc_check_now = crc_init();
                    crc_check_now = crc_update(crc_check_now, entry.c_str(), strlen(entry.c_str()));
                    crc_check_now = crc_finalize(crc_check_now);

                    if(crc != crc_check_now){
                        noentryerr.push_back(Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        ));
                        continue;
                    }
                }
            }

            { // Parse the JSON entry

                // Parse it and check for errors
                json j = json::parse(entry, nullptr, false);
                if(j.is_discarded()){
                    noentryerr.push_back(Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    ));
                    continue;
                }

                if(
                    !(
                        j.contains(MPARC::content_field) && 
                        j.contains(MPARC::checksum_field) &&
                        j.contains(MPARC::filename_field)
                    )
                ){
                    noentryerr.push_back(Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    ));
                    continue;
                }

                // grab the filename for the other parsing
                std::string filename = j.at(MPARC::filename_field);

                // Grab the processed base64 string
                std::string processed_b64_str = j.at(MPARC::content_field);

                if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Check if archive is version 2 or more
                    if(!j.contains(MPARC::processed_checksum_field)){ // check if the field is available
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
                        continue;
                    }

                    { // Calculate the processed (base64'd) string's checksum
                        crc_t processed_checksum = 0;
                        crc_t calculated_checksum = crc_init();

                        // Calculate
                        std::string pstr = processed_b64_str;

                        calculated_checksum = crc_update(calculated_checksum, pstr.c_str(), pstr.length());

                        calculated_checksum = crc_finalize(calculated_checksum);


                        { // Grab the checksum from the entry and parse it
                            std::string strsum = j.at(MPARC::processed_checksum_field);

                            if(!scan_crc_t_from_string(strsum, processed_checksum)){
                                entryerr[filename] = Status(
                                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                                );
                                continue;
                            }
                        }

                        // Check
                        if(processed_checksum != calculated_checksum){
                            entryerr[filename] = Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                            continue;
                        }
                    }
                }

                // Decryption and unprocessed string grab
                std::string raw_unprocessed_str = "";
                {
                    std::string ecrypt_str = b64::from_base64(processed_b64_str);

                    std::string XOR_k;
                    std::vector<int> ROT_k;
                    std::string Camellia_ks;
                    ByteArray Camellia_k;
                    ByteArray::size_type Camellia_kbl;

                    archive.get_xor_encryption(XOR_k);
                    archive.get_rot_encryption(ROT_k);
                    archive.get_camellia_encryption(Camellia_ks);

                    Camellia_k = Utils::StringToByteArray(Camellia_ks);

                    Camellia_kbl = Camellia_k.size();

                    if(archive.loaded_version >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty() && !ecrypt_str.empty()){
                        int ckbl = Camellia_kbl;
                        int ret = camellia_keysize(&ckbl);
                        if(ret == CRYPT_INVALID_KEYSIZE) {
                            entryerr[filename] = static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);
                        }

                        int num_rounds_camellia  = ((ckbl == 16) ? 18 : 24);

                        // Make key
                        symmetric_key camellia_key;
                        memset(&camellia_key, '\0', sizeof(camellia_key));
                        int setup_result = camellia_setup(&Camellia_k[0], ckbl, num_rounds_camellia, &camellia_key);
                        if (setup_result != CRYPT_OK) {
                            entryerr[filename] = static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_FAIL);
                            continue;
                        }

                        {
                            ByteArray ciphertext = Utils::StringToByteArray(ecrypt_str);
                            std::string plaintext = "";
                            camellia_decrypt(ciphertext, plaintext, &camellia_key, ckbl);
                            ecrypt_str = plaintext;
                        }

                        camellia_done(&camellia_key);
                    }

                    if(!ROT_k.empty()){
                        for(auto& key : ROT_k){
                            int tmp_key = (-1 * key);
                            key = tmp_key;
                        }
                        ecrypt_str = encrypt_rot(ecrypt_str, ROT_k);
                    }
                    if(!XOR_k.empty()){
                        ecrypt_str = encrypt_xor(ecrypt_str, XOR_k);
                    }

                    raw_unprocessed_str = ecrypt_str;
                }
                Entry entreh;
                entreh.content = Utils::StringToByteArray(raw_unprocessed_str);

                { // Calculate the checksum (CRC32) of the unprocessed (no Base64 and encryption) string
                    crc_t unprocessed_checksum = 0;
                    crc_t calculated_checksum = crc_init();

                    // Calculate
                    calculated_checksum = crc_update(calculated_checksum, raw_unprocessed_str.c_str(), raw_unprocessed_str.length());

                    calculated_checksum = crc_finalize(calculated_checksum);

                    {
                        // Grab the checksum from the entry
                        std::string upstrsum = j.at(MPARC::checksum_field);

                        // Parse and scan it
                        if(!scan_crc_t_from_string(upstrsum, unprocessed_checksum)){
                            entryerr[filename] = Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                            continue;
                        }
                    }

                    // Check
                    if(unprocessed_checksum != calculated_checksum){
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                        continue;
                    }
                }

                if(archive.loaded_version >= MPARC::FLETCHER32_INITIAL_UPDATE_VERSION){ // Calculate the Fletcher32 version of the unprocessed string
                    if(!j.contains(MPARC::fletcher_checksum_field)){
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );   
                        continue;
                    }

                    fletcher32_t sum = 0; // Scanned/Parsed sum
                    fletcher32_t lsum = 0; // Local sum

                    { // Calculate locally
                        std::vector<uint16_t> uvec16(raw_unprocessed_str.begin(), raw_unprocessed_str.end());
                        lsum = fletcher_t_update(&uvec16[0], uvec16.size());
                    }

                    {
                        // Grab the fletcher
                        std::string fletch = j.at(MPARC::fletcher_checksum_field);

                        if(!scan_fletcher_t_from_string(fletch, sum)){
                            entryerr[filename] = Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                            continue;
                        }
                    }

                    // Check
                    if(lsum != sum){
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                        continue;
                    }
                }

                if(archive.loaded_version >= MPARC::SECURE_UPDATE_VERSION){
                    if(!(
                        j.contains(MPARC::sha256_checksum_field) &&
                        j.contains(MPARC::md5_checksum_field)
                    )){
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
                        continue;
                    }

                    { // MD5 check
                        std::string md5sum; // scanned md5
                        std::string md5lsum; // local md5

                        md5sum = j.at(MPARC::md5_checksum_field);

                        MD5 md5summer;
                        md5summer.add(&raw_unprocessed_str[0], raw_unprocessed_str.size());
                        md5lsum = md5summer.getHash();

                        if(md5lsum != md5sum){
                            entryerr[filename] = Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                            continue;
                        }
                    }

                    { // SHA256 check
                        std::string sha256sum; // scanned sha256
                        std::string sha256lsum; // local 256

                        sha256sum = j.at(MPARC::sha256_checksum_field);

                        SHA256 sha256summer;
                        sha256summer.add(&raw_unprocessed_str[0], raw_unprocessed_str.size());
                        sha256lsum = sha256summer.getHash();

                        if(sha256lsum != sha256sum){
                            entryerr[filename] = Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                            continue;
                        }
                    }
                }

                if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // check if archive is version 2 or more
                    if(!j.contains(MPARC::meta_field)){ // check if the field is available
                        entryerr[filename] = Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
                    }
                    // Set the metadata
                    for(auto meta : j[MPARC::meta_field].items()){
                        entreh.metadata[b64::from_base64(meta.key())] = b64::from_base64(meta.value());
                    }
                }

                // Push it in
                Status stat;
                if(!(
                    stat = archive.push(b64::from_base64(
                        filename
                    ), entreh, true)
                )){
                    entryerr[filename] = stat;
                    continue;
                }
            }
        }
    }
    
    return make_parsereturn(std::move(noentryerr), std::move(entryerr));
}

static ParseReturn parse_footer(MPARC& archive, std::string footer_input){
    UNUSED((archive));
    size_t footer_pos = footer_input.find(MPARC::end_of_archive_marker);
    if(footer_pos == std::string::npos || footer_input[footer_pos] != MPARC::end_of_archive_marker) {
        return make_parsereturn({Status(
            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
        )}, {});
    }
    return make_parsereturn({Status(Status::Code::OK)}, {});
}



static std::string encrypt_xor(std::string input, std::string key){
    std::string outstr = input;
    std::string::size_type input_len = input.size();
    std::string::size_type key_len = key.size();

    if(!key.empty()){
        for(std::string::size_type i = 0; i < input_len; i++){
            outstr[i] = (input[i] ^ key[i % key_len]);
        }
    }

    return outstr;
}

static std::string encrypt_rot(std::string input, std::vector<int> key){
    std::string outstr = input;
    std::string::size_type input_len = input.size();
    std::vector<int>::size_type key_len = key.size();

    if(!key.empty()){
        for(std::string::size_type i = 0; i < input_len; i++){
            outstr[i] = (input[i] + key.at(i % key_len));
        }
    }

    return outstr;
}
