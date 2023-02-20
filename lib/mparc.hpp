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
    using ByteArray = std::vector<unsigned char>;

    struct Entry{
        ByteArray content;
        bool directory;
    };

    class Status{
        public:
        enum Code : std::int64_t {
            OK=0,

            GENERIC = 1 << 1,
            INTERNAL = 1 << 2,
            NOT_IMPLEMENTED = 1 << 3,

            INVALID_VALUE = 1 << 4,
            NULL_VALUE = 1 << 5,

            KEY = 1 << 6,
            KEY_EXISTS = 1 << 7,
            KEY_NOEXISTS = 1 << 8,

            FERROR = 1 << 9,
            ISDIR = 1 << 10
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

    class MPARC{
        public:
        /**
         * @brief An alias for a function that check if something is a directory
         * 
         * @details
         * 
         * It shall return these values:
         * 
         * - Status::Code::OK -> Is a irectory
         * - Status::Code::KEY | Status::Code::KEY_NOEXISTS -> Not a directory
         * - Status::Code::NOT_IMPLEMENTED -> Not implemnted
         * - Others -> Eroneous to return
         * 
         */
        using isDirFuncType = std::function<Status::Code(std::string)>;

        private:
        std::map<std::string, Entry> entries;
        std::recursive_mutex sync_mutex;
        Status::Code my_code = Status::Code::OK;

        void init();

        public:
        MPARC();
        MPARC(std::vector<std::string> entries);
        MPARC(MPARC& other);

        Status exists(std::string name);

        Status push(std::string name, Entry entry, bool overwrite);
        Status push(std::string name, bool directory, ByteArray content, bool overwrite);
        Status push(std::string name, bool directory, std::string content, bool overwrite);
        Status push(std::string name, isDirFuncType isDirFunc, bool overwrite);
        Status push(std::string name, bool overwrite);

        Status pop(std::string name);

        Status peek(std::string name);
        Status peek(std::string name, std::string* output_str, ByteArray* output_ba);
    };


    namespace Utils{
        ByteArray StringToByteArray(std::string content);
        std::string ByteArrayToString(ByteArray bytearr);

        Status::Code isDirectoryDefaultImplementation(std::string path);
    };
};



#endif
