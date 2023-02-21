#ifndef _MXPSQL_MPARC_FIX11_HPP
#define _MXPSQL_MPARC_FIX11_HPP
/**
 * @brief The new C++11 (with a bit of C++17 if available) rewrite of MPARC from the spaghetti C99 code.
 * 
 */

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define MXPSQL_MPARC_FIX11_CPP17 true
#endif

#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <vector>
#include <streambuf>

#ifdef MXPSQL_MPARC_FIX11_CPP17
#include <filesystem>
#endif

namespace MXPSQL::MPARC11{
    /// @brief A typedef for a vector of unsigned char, also a byte aray
    using ByteArray = std::vector<unsigned char>;

    /// @brief A representation of an entry
    struct Entry{
        /// @brief Content of the entry, if applicable
        ByteArray content;
    };

    class Status{
        public:
        /// @brief Code enumeration for error values
        enum Code : std::uint64_t {
            /// @brief Ok, nothing is wrong
            OK=0,

            /// @brief  The most generic error code
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
        Code stat_code = Code::OK;

        public:
        Status() : Status(Code::OK) {}
        Status(Code code) : stat_code(code) {}

        std::string str(Code* code);
        bool isOK();
        Code getCode();
        void assert(bool throw_err);

        operator bool();
    };

    /**
     * @brief The class, which is also the archive
     * 
     */
    class MPARC{
        public:
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
        /// @brief  Construct an empty archive
        MPARC();
        /// @brief Construct an archive from a list of strings
        /// @param entries the list of strings that mention the names of the file you want to copy
        MPARC(std::vector<std::string> entries);
        /// @brief Construct an archive by copying another one
        /// @param other that other one you want to copy from
        MPARC(MPARC& other);

        /// @brief Does [name] exists?
        /// @param name the entry to check for existence
        /// @return Status::Code::OK = Exists. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist.
        Status exists(std::string name);

        /// @brief Push an entry struct as a file into the archive.
        /// @param name Name of the entry.
        /// @param entry The entry of file. It also can be a directory.
        /// @param overwrite Overwrite an existing entry?
        /// @return Status::Code::OK = Exists. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist.
        Status push(std::string name, Entry entry, bool overwrite);
        /// @brief Push arguments as an entry struct as a file into the archive.
        /// @param name Name of the entry
        /// @param directory Is entry a directory?
        /// @param content Content of the entry, ignored if directory is true
        /// @param overwrite Overwrite an existing entry?
        /// @return Status::Code::OK = Exists. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist.
        Status push(std::string name, ByteArray content, bool overwrite);
        /// @brief Push arguments (string edition) as an entry struct as a file into the archive.
        /// @param name Name of the entry
        /// @param directory Is entry a directory
        /// @param content Content of the entry, ignored if directory is true
        /// @param overwrite Overwrite an existing entry?
        /// @return Status::Code::OK = Exists. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist.
        Status push(std::string name, std::string content, bool overwrite);
        /// @brief Push an entry by reading it into the archive.
        /// @param name Name of the entry
        /// @param overwrite Overwrite an existing entry?
        /// @return Status::Code::OK = Exists. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist.
        Status push(std::string name, bool overwrite);

        /// @brief Pop an entry off the archive
        /// @param name Name of the entry to pop off
        /// @return Status::Code::OK = Success. Status::Code::KEY | Status::Code::KEY_NOEXISTS = [name] does not exist.
        Status pop(std::string name);

        /// @brief Basically an alias of exists
        /// @param name Name of the entry to check
        /// @see exists
        /// @return Status::Code::OK = Success, you got a file. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, it doesn't exist.
        Status peek(std::string name);
        /// @return Status::Code::OK = Success. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, it doesn't exist.
        Status peek(std::string name, std::string* output_str, ByteArray* output_ba);

        /// @brief Swap [name] and [name2]
        /// @param name Name of the first entry to swap
        /// @param name2 Name of the second entry to swap
        /// @return Status::Code::OK = Success. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, one of the names doesn't exist.
        Status swap(std::string name, std::string name2);
        /// @brief copy from [name] to [name2]
        /// @param name Name of the first entry as a source
        /// @param name2 Name of the second entry as a destination
        /// @param overwrite Overwrite [name2] if it exists?
        /// @return Status::Code::OK = Success. Status::Code::KEY | Status::Code::KEY_EXISTS = Fail, name2 exists, no overwrite. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, one of the names doesn't exist.
        Status copy(std::string name, std::string name2, bool overwrite);
        /// @brief Rename [name] to [name2]
        /// @param name Old name of entry you wish to rename
        /// @param name2 New name of [name]
        /// @param overwrite Overwrite [name2] if it exists?
        /// @return Status::Code::OK = Success. Status::Code::KEY | Status::Code::KEY_EXISTS = Fail, name2 exists, no overwrite. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Fail, one of the names doesn't exist.
        Status rename(std::string name, std::string name2, bool overwrite);

        /// @brief List all the files
        /// @param output Output vector.
        /// @return Status::Code::OK = Success.
        /// @warning This function will clear your vector.
        Status list(std::vector<std::string>& output);
    };


    namespace Utils{
        /// @brief Convert a string to a bytearray
        /// @param content Your string
        /// @return Your bytearray
        ByteArray StringToByteArray(std::string content);
        /// @brief Convert a bytearray to a string
        /// @param bytearr Your bytearray
        /// @return Your string
        std::string ByteArrayToString(ByteArray bytearr);

        /// @brief The default implementation for checking if [path] is a directory. Used in the push function
        /// @param path The path to check
        /// @return Status::Code::OK = Exists, is a file. Status::Code::ISDIR = Exists, is a directory. Status::Code::KEY | Status::Code::KEY_NOEXISTS = Does not exist. Status::Code::NOT_IMPLEMENTED = the directory check function is not implemented in your platform.
        Status::Code isDirectoryDefaultImplementation(std::string path);
    };
};



#endif
