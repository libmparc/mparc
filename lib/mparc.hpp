#ifndef _MXPSQL_MPARC_FIX11_HPP
#define _MXPSQL_MPARC_FIX11_HPP
/**
 * @brief The new C++11 (with a bit of C++17 if available) rewrite of MPARC from
 * the spaghetti C99 code.
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
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

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define MXPSQL_MPARC_FIX11_CPP17 true
#endif

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

#ifdef MXPSQL_MPARC_FIX11_CPP17
#include <filesystem>
#endif

namespace MXPSQL {
namespace MPARC11 {
/// @brief A typedef for a vector of unsigned char, also a byte aray
using ByteArray = std::vector<unsigned char>;

/// @brief A representation of an entry
struct Entry {
    /// @brief Content of the entry, if applicable
    ByteArray content;
    /// @brief Extra metadata
    std::map<std::string, std::string> metadata;
};

class Status {
public:
    /// @brief Code enumeration for error values
    enum Code : std::uint16_t {
        /// @brief Ok, nothing is wrong
        OK = 0,

        /// @brief    The most generic error code
        GENERIC = 1 << 1,
        /// @brief Internal error
        INTERNAL = 1 << 2,
        /// @brief Not implemented at all
        NOT_IMPLEMENTED = 1 << 3,
        /// @brief False return value
        FALSE = 1 << 4,

        /// @brief Invalid value provided
        INVALID_VALUE = 1 << 5,
        /// @brief Null value provided
        NULL_VALUE = 1 << 6,

        /// @brief Key related error
        KEY = 1 << 7,
        /// @brief Key exists
        KEY_EXISTS = 1 << 8,
        /// @brief Key does not exists
        KEY_NOEXISTS = 1 << 9,

        /// @brief File I/0 related error
        FERROR = 1 << 10,
        /// @brief Is a directory
        ISDIR = 1 << 11
    };

private:
    /// @brief Internal code
    Code stat_code = Code::OK;

public:
    /// @brief    Default constructor
    Status() : Status(Code::OK) {}
    /// @brief Constructor with code
    /// @param code The code
    Status(Code code) : stat_code(code) {}

    /// @brief Get a string representation of the code
    /// @param code The code that you want to use. If nullptr, use the internal
    /// code, else use the value of the code pointer.
    /// @return The string representation.
    std::string str(Code *code);
    /// @brief Is code OK?
    /// @return Is code OK?
    bool isOK();
    /// @brief Get the internal code
    /// @return the internal code
    Code getCode();
    /// @brief Assert whether the status is OK. If not, it either throws a
    /// std::runtime_error or calls abort();
    /// @param throw_err If true, instruct to throw std::runtime_error. If false,
    /// abort();
    void assertion(bool throw_err);

    /// @brief An alias for isOK
    /// @see isOK
    operator bool();
};

/**
 * @brief The class, which is also the archive
 *
 */
class MPARC {
public:
    /// @brief Separator between each entry in the entry list
    static const char entries_entry_separator = '\n';
    /// @brief Separator for the checksum and the content in each entry
    static const char entry_checksum_content_separator = '%';

private:
    /// @brief Internal storage
    std::map<std::string, Entry> entries;
    /// @brief Safety mutex
    std::recursive_mutex sync_mutex;
    /// @brief Internal error reporting
    Status::Code my_code = Status::Code::OK;

    /// @brief Initialization function
    void init();

public:
    /// @brief Construct an empty archive
    MPARC();
    /// @brief Construct an archive from a list of strings
    /// @param entries the list of strings that mention the names of the file you
    /// want to copy
    MPARC(std::vector<std::string> entries);
    /// @brief Construct an archive by copying another one
    /// @param other that other one you want to copy from
    MPARC(MPARC &other);

    /// @brief Does [name] exists?
    /// @param name the entry to check for existence
    /// @return Status::Code::OK = Exists. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Does not exist.
    Status exists(std::string name);

    /// @brief Push an entry struct as a file into the archive.
    /// @param name Name of the entry.
    /// @param entry The entry of file. It also can be a directory.
    /// @param overwrite Overwrite an existing entry?
    /// @return Status::Code::OK = Exists. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Does not exist.
    Status push(std::string name, Entry entry, bool overwrite);
    /// @brief Push arguments as an entry struct as a file into the archive.
    /// @param name Name of the entry
    /// @param directory Is entry a directory?
    /// @param content Content of the entry, ignored if directory is true
    /// @param overwrite Overwrite an existing entry?
    /// @return Status::Code::OK = Exists. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Does not exist.
    Status push(std::string name, ByteArray content, bool overwrite);
    /// @brief Push arguments (string edition) as an entry struct as a file into
    /// the archive.
    /// @param name Name of the entry
    /// @param directory Is entry a directory
    /// @param content Content of the entry, ignored if directory is true
    /// @param overwrite Overwrite an existing entry?
    /// @return Status::Code::OK = Exists. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Does not exist.
    Status push(std::string name, std::string content, bool overwrite);
    /// @brief Push an entry by reading it into the archive.
    /// @param name Name of the entry
    /// @param overwrite Overwrite an existing entry?
    /// @return Status::Code::OK = Exists. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Does not exist.
    Status push(std::string name, bool overwrite);

    /// @brief Pop an entry off the archive
    /// @param name Name of the entry to pop off
    /// @return Status::Code::OK = Success. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = [name] does not exist.
    Status pop(std::string name);

    /// @brief Basically an alias of exists
    /// @param name Name of the entry to check
    /// @see exists
    /// @return Status::Code::OK = Success, you got a file. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Fail, it doesn't exist.
    Status peek(std::string name);
    /// @return Status::Code::OK = Success. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Fail, it doesn't exist.
    Status peek(std::string name, std::string *output_str, ByteArray *output_ba);

    /// @brief Swap [name] and [name2]
    /// @param name Name of the first entry to swap
    /// @param name2 Name of the second entry to swap
    /// @return Status::Code::OK = Success. Status::Code::KEY |
    /// Status::Code::KEY_NOEXISTS = Fail, one of the names doesn't exist.
    Status swap(std::string name, std::string name2);
    /// @brief copy from [name] to [name2]
    /// @param name Name of the first entry as a source
    /// @param name2 Name of the second entry as a destination
    /// @param overwrite Overwrite [name2] if it exists?
    /// @return Status::Code::OK = Success. Status::Code::KEY |
    /// Status::Code::KEY_EXISTS = Fail, name2 exists, no overwrite.
    /// Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, one of the names
    /// doesn't exist.
    Status copy(std::string name, std::string name2, bool overwrite);
    /// @brief Rename [name] to [name2]
    /// @param name Old name of entry you wish to rename
    /// @param name2 New name of [name]
    /// @param overwrite Overwrite [name2] if it exists?
    /// @return Status::Code::OK = Success. Status::Code::KEY |
    /// Status::Code::KEY_EXISTS = Fail, name2 exists, no overwrite.
    /// Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, one of the names
    /// doesn't exist.
    Status rename(std::string name, std::string name2, bool overwrite);

    /// @brief List all the files
    /// @param output Output vector.
    /// @return Status::Code::OK = Success.
    /// @warning This function will clear your vector.
    Status list(std::vector<std::string> &output);

    /// @brief Construct the archive
    /// @param output Output string to store the archive. Output is untouched if
    /// an error occurs.
    /// @return Status::Code::OK = Success.
    Status construct(std::string &output);

    operator bool();
    operator==(MPARC &other);
};

namespace Utils {

/// @brief Convert a string to a bytearray
/// @param content Your string
/// @return Your bytearray
ByteArray StringToByteArray(std::string content);
/// @brief Convert a bytearray to a string
/// @param bytearr Your bytearray
/// @return Your string
std::string ByteArrayToString(ByteArray bytearr);

/// @brief The default implementation for checking if [path] is a directory.
/// Used in the push function
/// @param path The path to check
/// @return Status::Code::OK = Exists, is a file. Status::Code::ISDIR = Exists,
/// is a directory. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not
/// exist. Status::Code::NOT_IMPLEMENTED = the directory check function is not
/// implemented in your platform.
Status::Code isDirectoryDefaultImplementation(std::string path);

}; // namespace Utils
}; // namespace MPARC11
}; // namespace MXPSQL

#endif
