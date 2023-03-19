/**
 * @brief The new C++11 (with a bit of C++17 if available) rewrite of MPARC from the spaghetti C99 code. Source file.
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

// NAMESPACE ALIASING
namespace MPARC11 = MXPSQL::MPARC11;
namespace Utils = MPARC11::Utils;
using ByteArray = MPARC11::ByteArray;
using Status = MPARC11::Status;
using MPARC = MPARC11::MPARC;
using Entry = MPARC11::Entry;

using json = nlohmann::json;
namespace b64 = base64;

#ifdef MXPSQL_MPARC_FIX11_CPP17
namespace stdfs = std::filesystem;
#endif

// Variables
static const MPARC::version_type EXTENSIBILITY_UPDATE_VERSION = 2;

// prototyping
static Status construct_entries(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_header(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_footer(MPARC& archive, std::string& output, MPARC::version_type ver);

static Status parse_header(MPARC& archive, std::string header_input);
static Status parse_entries(MPARC& archive, std::string entry_input);
static Status parse_footer(MPARC& archive, std::string footer_input);

[[noreturn]] static inline void my_unreachable(...);


// CRC32
typedef std::uint_fast32_t crc_t;
/**
 * Calculate the initial crc value.
 *
 * \return     The initial crc value.
 */
static inline crc_t crc_init(void)
{
	return 0xffffffff;
}


/**
 * Update the crc value with new data.
 *
 * \param[in] crc      The current crc value.
 * \param[in] data     Pointer to a buffer of \a data_len bytes.
 * \param[in] data_len Number of bytes in the \a data buffer.
 * \return             The updated crc value.
 */
static crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
	static const crc_t crc_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	const unsigned char *d = (const unsigned char *)data;
	unsigned int tbl_idx;

	while (data_len--) {
		tbl_idx = (crc ^ *d) & 0xff;
		crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;
		d++;
	}
	return crc & 0xffffffff;
}


/**
 * Calculate the final crc value.
 *
 * \param[in] crc  The current crc value.
 * \return     The
 */
static inline crc_t crc_finalize(crc_t crc)
{
	return crc ^ 0xffffffff;
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
    #ifdef MXPSQL_MPARC_FIX11_CPP17
    return (
        stdfs::is_directory(stdfs::path(path)) ?
        Status::Code::OK :
        Status::Code::ISDIR
    );
    #else
    (static_cast<void>(path));
    return Status::Code::NOT_IMPLEMENTED;
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

std::string Status::str(Status::Code* code = nullptr){
    Status::Code cod = Status::Code::OK;

    if(code){
        cod = *code;
    }
    else{
        cod = getCode();
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
    else if(cod & FALSE) {
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
        return "Archive failed during construction.";
    }
    else if (cod & PARSE_FAIL) {
        if(cod & NOT_MPAR_ARCHIVE) return "What was parsed is not an MPAR archive.";
        if(cod & CHECKSUM_ERROR) return "The checksum did not match or it was not obtained successfully.";
        if(cod & VERSION_ERROR) return "The version either was too new or it was not obtained successfully.";
        return "A generic parsing error has been detected.";
    }
    else{
        my_unreachable();
        return "Unknown code";
    }
}

std::string Status::str(){
    return str(nullptr);
}

Status::Code Status::getCode(){
    return stat_code;
}

Status::operator bool(){
    return isOK();
}


// MAIN ARCHIVE STRUCTURE
const std::string MPARC::filename_field = "filename"; // Compatibility with the C99 library
const std::string MPARC::content_field = "blob"; // Compatibility with the C99 library
const std::string MPARC::checksum_field = "crcsum"; // Compatibility with the C99 library
const std::string MPARC::processed_checksum_field = "crcsum.processed";
const std::string MPARC::meta_field = "metadata";

const std::string MPARC::encrypt_meta_field = "encrypt"; // Compatibility with the C99 library
const std::string MPARC::extra_meta_field = "extra";

const std::string MPARC::magic_number = "MXPSQL's Portable Archive"; // Compatibility with the C99 library

std::map<std::string, std::string> MPARC::dummy_extra_metadata;

MPARC::MPARC(){};
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


Status MPARC::extra_metadata_setter_getter(std::map<std::string, std::string>& output, std::map<std::string, std::string>& input){
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
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
        }
        archive << header;
    }
    { // Construct the tnreis
        std::string entries = "";
        if(!(stat = construct_entries(*this, entries, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
        }
        archive << entries;
    }
    { // Construct the footer
        std::string footer = "";
        if(!(stat = construct_footer(*this, footer, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
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


Status MPARC::parse(std::string input){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);

    Status stat;

    std::string header = "";
    std::string entries = "";
    std::string footer = "";

    // This section would be handled by a function that would perform steganography by searching for the archive if embedded in files, but it is not yet implemented.
    // Maybe never will, maybe will be implemented, I don't know, lets see later.
    // I realized that the function would require to be extremly smart at finding the archive, it cannpt be some "find a character" thing.
    // "Scan a block to see if it is an archive, then move one character if not an archive, put it in if its an archive" would be nice, but inefficient
    std::string searchInput = input;

    // find the header and footer marks
    size_t header_marksep_pos = searchInput.find(MPARC::post_header_separator);
    size_t footer_marksep_pos = searchInput.find_last_of(MPARC::end_of_entries_separator);

    // Check if either both marker exist and that the header marker position is before the footer marker position
    if(header_marksep_pos == std::string::npos || footer_marksep_pos == std::string::npos || header_marksep_pos >= footer_marksep_pos){
        return Status(
            (this->my_code = static_cast<Status::Code>(
                Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE
            ))
        );
    }

    // Slice the strings into their parts
    header = searchInput.substr(0, header_marksep_pos);
    entries = searchInput.substr(header_marksep_pos+1, (footer_marksep_pos-header_marksep_pos-1));
    footer = searchInput.substr(footer_marksep_pos+1, (input.size()-footer_marksep_pos));

    // Parse the header
    if(!(stat = parse_header(*this, header))){
        return (this->my_code = stat.getCode());
    }

    // Parse the entries
    if(!(stat = parse_entries(*this, entries))){
        return (this->my_code = stat.getCode());
    }

    // Parse the footer
    if(!(stat = parse_footer(*this, footer))){
        return (this->my_code = stat.getCode());
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
        Status::Code::FALSE
    );
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




// UTILITIES
[[noreturn]] static inline void my_unreachable(...){
    #if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
        std::unreachable();
    #else

        #ifdef __GNUC__ // GCC, Clang, ICC
            __builtin_unreachable();
        #elif defined(_MSC_VER) // MSVC
            __assume(false);
        #else // NULL / ABORT
            int* f = NULL;
            *f = 21;
            std::abort();
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
        j[b64::to_base64(MPARC::encrypt_meta_field)] = json::array();

        if(ver >= EXTENSIBILITY_UPDATE_VERSION){// Extra user defined metadata
            j[b64::to_base64(MPARC::extra_meta_field)] = json::object();
            std::map<std::string, std::string> extras;
            archive.extra_metadata_setter_getter(extras, MPARC::dummy_extra_metadata);
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
    using jty = std::pair<json, crc_t>;

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

        { // Unprocessed checksum calculate
            std::string csum = "";

            crc_t crc = crc_init();

            crc = crc_update(crc, strcontent.c_str(), strcontent.length());

            crc = crc_finalize(crc);

            csum = std::to_string(crc);

            jentry[MPARC::checksum_field] = csum;
        }

        // Put in the obvious ones (filename, contents)
        jentry[MPARC::filename_field] = b64::to_base64(entry);
        jentry[MPARC::content_field] = b64::to_base64(strcontent);


        if(ver >= EXTENSIBILITY_UPDATE_VERSION){ // Put in the checksum after it is processed
            std::string csum = "";
            std::string b64content = jentry.at(MPARC::content_field);

            crc_t crc = crc_init();

            crc = crc_update(crc, b64content.c_str(), b64content.length());

            crc = crc_finalize(crc);

            csum = std::to_string(crc);

            jentry[MPARC::processed_checksum_field] = csum;
        }

        if(ver >= EXTENSIBILITY_UPDATE_VERSION){ // Add the user defined metadata thing
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
        return (stat = Status(Status::Code::CONSTRUCT_FAIL));
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
    (static_cast<void>(archive));
    (static_cast<void>(ver));

    std::stringstream ssb;

    // Just one character
    ssb << MPARC::end_of_archive_marker;

    output = ssb.str();

    return Status(Status::Code::OK);
}



// ARCHIVE PARSER
static Status parse_header(MPARC& archive, std::string header_input){
    ((void)archive);

    std::string magic = "";
    std::string vf_other = "";

    { // Separate the magic number and the usable content and then test it
        size_t magic_separator_pos = header_input.find(MPARC::magic_number_separator); // Grab the magic separator
        if(magic_separator_pos == std::string::npos){ // Check if it exists
            return Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            );
        }
        // Extract the magic number and the other useful ones
        magic = header_input.substr(0, magic_separator_pos);
        vf_other = header_input.substr(magic_separator_pos+1, std::string::npos);

        if(magic != MPARC::magic_number){
            return Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            );
        }
    }

    {
        std::string version = "";
        std::string meta = "";
        { // Separate the version and the metadata and then grab it
            size_t vm_header_meta_separator_pos = vf_other.find(MPARC::header_meta_magic_separator); // Grab the metadata and version separator
            if(vm_header_meta_separator_pos == std::string::npos){ // Check if it exists
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                );
            }

            // Extract the version and the metadata
            version = vf_other.substr(0, vm_header_meta_separator_pos);
            meta = vf_other.substr(vm_header_meta_separator_pos+1, std::string::npos);
        }

        { // test the version
            MPARC::version_type ck_ver = 0;
            if(!Utils::StringToVersionType(version, ck_ver) || (ck_ver > MPARC::mpar_version)){
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::VERSION_ERROR)
                );
            }

            archive.loaded_version = ck_ver;
        }

        { // Parse the metadata
            json j = json::parse(meta, nullptr, false);
            if(j.is_discarded()){
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                );
            }

            if(j.contains(b64::to_base64(MPARC::encrypt_meta_field))){ // encryption thingy
                std::vector<std::string> encrypt_algos = j.at(b64::to_base64(MPARC::encrypt_meta_field));
            }
            else{
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                );
            }

            if(archive.loaded_version >= EXTENSIBILITY_UPDATE_VERSION){ // Extra global metadata, only if version 2 is reached
                if(!j.contains(b64::to_base64(MPARC::extra_meta_field))){ // check if the field is available
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                std::map<std::string, std::string> extras;

                for(auto meta : j[b64::to_base64(MPARC::extra_meta_field)].items()){
                    extras[b64::from_base64(meta.key())] = b64::from_base64(meta.value());
                }
                archive.extra_metadata_setter_getter(MPARC::dummy_extra_metadata, extras);
            }
        }
    }
    return Status::Code::OK;
}

static Status parse_entries(MPARC& archive, std::string entry_input){
    std::vector<std::string> lines;
    { // Read line by line
        std::stringstream ss(entry_input);
        std::string str;
        while(std::getline(ss, str, '\n')){
            lines.push_back(str);
        }
    }

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
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                // Grab the checksum and the actual entry
                std::string checksum = line.substr(0, checksum_entry_marksep_pos);
                entry = line.substr(checksum_entry_marksep_pos+1, (line.size()-checksum_entry_marksep_pos));

                // Grab the actual checksum value
                if(sscanf(checksum.c_str(), "%" SCNuFAST32, &crc) < 1){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    );
                }

                { // Calculate and check the checksum
                    crc_t crc_check_now = crc_init();
                    crc_check_now = crc_update(crc_check_now, entry.c_str(), strlen(entry.c_str()));
                    crc_check_now = crc_finalize(crc_check_now);

                    if(crc != crc_check_now){
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                    }
                }
            }

            { // Parse the JSON entry

                // Parse it and check for errors
                json j = json::parse(entry, nullptr, false);
                if(j.is_discarded()){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    );
                }

                if(
                    !(
                        j.contains(MPARC::content_field) && 
                        j.contains(MPARC::checksum_field) &&
                        j.contains(MPARC::filename_field)
                    )
                ){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                // Grab the processed base64 string
                std::string processed_b64_str = j.at(MPARC::content_field);

                if(archive.loaded_version >= EXTENSIBILITY_UPDATE_VERSION){ // Check if archive is version 2 or more
                    if(!j.contains(MPARC::processed_checksum_field)){ // check if the field is available
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
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

                            if(sscanf(strsum.c_str(), "%" SCNuFAST32, &processed_checksum) < 1){
                                return Status(
                                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                                );
                            }
                        }

                        // Check
                        if(processed_checksum != calculated_checksum){
                            return Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                        }
                    }
                }

                // Unprocessed string grab
                std::string raw_unprocessed_str = b64::from_base64(processed_b64_str);
                Entry entreh;
                entreh.content = Utils::StringToByteArray(raw_unprocessed_str);

                { // Calculate the checksum of the unprocessed (no base64) string
                    crc_t unprocessed_checksum = 0;
                    crc_t calculated_checksum = crc_init();

                    // Calculate
                    calculated_checksum = crc_update(calculated_checksum, raw_unprocessed_str.c_str(), raw_unprocessed_str.length());

                    calculated_checksum = crc_finalize(calculated_checksum);

                    {
                        // Grab the checksum from the entry
                        std::string upstrsum = j.at(MPARC::checksum_field);

                        // Parse and scan it
                        if(sscanf(upstrsum.c_str(), "%" SCNuFAST32, &unprocessed_checksum) < 1){
                            return Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                        }
                    }

                    // Check
                    if(unprocessed_checksum != calculated_checksum){
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                    }
                }

                if(archive.loaded_version >= EXTENSIBILITY_UPDATE_VERSION){ // check if archive is version 2 or more
                    if(!j.contains(MPARC::meta_field)){ // check if the field is available
                        return Status(
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
                        j.at(MPARC::filename_field)
                    ), entreh, true)
                )){
                    return stat;
                }
            }
        }
    }
    return Status(Status::Code::OK);
}

static Status parse_footer(MPARC& archive, std::string footer_input){
    (static_cast<void>(archive));
    size_t footer_pos = footer_input.find(MPARC::end_of_archive_marker);
    if(footer_pos == std::string::npos || footer_input[footer_pos] != MPARC::end_of_archive_marker) {
        return Status(
            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
        );
    }
    return Status(Status::Code::OK);
}
