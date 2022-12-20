/**
  * @file mparc.hpp
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. C++ Wrapper.
  * @version 0.1
  * @date 2022-12-04
  * 
  * @copyright
  * 
  * Licensed To You Under Teh MIT License and the LGPL-2.1-Or-Later License
  * 
  * MIT License
  * 
  * Copyright (c) 2022 MXPSQL
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
  * Copyright (C) 2022 MXPSQL
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
 */

#ifndef _MXPSQL_MPARC_CXX
/// Include guard
#define _MXPSQL_MPARC_CXX

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
// NOOP
#else
#error "Compiler Not C++11 Compliant"
#endif

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstddef>

#include <cstdlib>

#include "./mparc.h"

namespace MXPSQL{
    namespace MPARC{
        class MPARC;
        class MPARC_Iter;
        class MPARC_Error;

        /**
         * @brief The C++ Object Oriented wrapper for MXPSQL_MPARC_err
         * 
         */
        class MPARC_Error{
            private:
            MXPSQL_MPARC_err err = MPARC_OK;

            public:
            /**
             * @brief Construct a new mparc error object with default values
             * 
             */
            MPARC_Error() : MPARC_Error(MPARC_OK) {}
            /**
             * @brief Construct a new mparc error object with OUR Values
             * 
             * @param in OUR Values
             */
            MPARC_Error(MXPSQL_MPARC_err in) : err(in) {}

            /**
             * @brief Get the Err enumeration
             * 
             * @return MXPSQL_MPARC_err Err enumeration
             */
            MXPSQL_MPARC_err getErr() {return err;}
            /**
             * @brief Set the Err enumeration
             * 
             * @param err Err enumeration
             */
            void setErr(MXPSQL_MPARC_err err) {this->err = err;}
            /**
             * @brief Die by abortion if I am not fine
             * 
             */
            void OrDie() {
                if(this->getErr() != MPARC_OK){
                    try{
                        this->OrThrow();
                    }
                    catch(std::runtime_error& err){
                        std::cerr << err.what() << std::endl << "Die operation has been requested, so death by abortion with std::abort!" << std::endl;
                        std::abort();
                    }
                }
            }

            /**
             * @brief Throw a std::runtime_error if I am not fine
             * 
             */
            void OrThrow(){
                if(this->getErr() != MPARC_OK){
                    std::string str = std::string("MPARC Runtime Error with code of ") + std::to_string(((int)this->getErr())) + ": " + "|";
                    throw std::runtime_error(str.c_str());
                }
            }

            /**
             * @brief Am I OK?
             * 
             * @return true I am OK
             * @return false I am not OK
             */
            bool isOk(){return (this->getErr() == MPARC_OK);}
        };

        /**
         * @brief The C++ Object Oriented wrapper for MXPSQL_MPARC_iter_t*
         * 
         * @note You can only go forward, not backwards. This is a limitation of the underlying iterator, not being lazy.
         */
        class MPARC_Iter{
            private:
            /**
             * @brief The handle
             * 
             */
            MXPSQL_MPARC_iter_t* handle = NULL;
            /// @brief Internal storage for the dereference operator
            std::string internal_nam = "";

            public:
            /**
             * @brief Construct a new mparc iterator object
             * 
             * @param Ptr 
             */
            MPARC_Iter(MXPSQL_MPARC_t* Ptr){
                MXPSQL_MPARC_err err = MPARC_list_iterator_init(&Ptr, &handle);
                MPARC_Error(err).OrThrow();
            }

            /**
             * @brief Destroy the mparc iterator object
             * 
             */
            ~MPARC_Iter(){
                MPARC_list_iterator_destroy(&handle);
            }

            /**
             * @brief Advance the iterator state
             * 
             * @param out Filename during advancing
             * @return MPARC_Error status
             */
            MPARC_Error next(std::string& out){
                MXPSQL_MPARC_err err = MPARC_OK;
                const char* nam = NULL;

                err = MPARC_list_iterator_next(&handle, &nam);

                if(err == MPARC_OK) {
                    internal_nam = std::string(nam);
                    out = internal_nam;
                }

                return MPARC_Error(err);
            }

            /**
             * @brief Prefix increment operator to advance state
             * 
             * @return std::string Current file, empty if reached the end
             */
            std::string operator++(){
                std::string str = "";
                next(str);
                return str;
            }

            /**
             * @brief Postfix increment operator to advance state
             * 
             * @return std::string Current file, empty if reached the end
             */
            std::string operator++(int){
                return operator++();
            }

            /**
             * @brief Dereference operator to get current state
             * 
             */
            std::string& operator*() {
                return internal_nam;
            }
        };

        /**
         * @brief The C++ Object Oriented wrapper for MXPSQL_MPARC_t*
         * 
         */
        class MPARC{
            private:
            /**
             * @brief The handle
             * 
             */
            MXPSQL_MPARC_t* archive = NULL;

            public:
            /**
             * @brief Construct a new MPARC object in a boring manner
             * 
             */
            MPARC() {
                MXPSQL_MPARC_err err = MPARC_init(&archive);
                MPARC_Error(err).OrThrow(); // throw
            }

            /**
             * @brief Construct a new MPARC object by reading a file
             * 
             * @param path archive file to read
             */
            MPARC(const char* path) : MPARC() {
                MXPSQL_MPARC_err err = MPARC_parse_filename(this->getInstance(), path);
                MPARC_Error(err).OrThrow();
            }

            /**
             * @brief Construct a new MPARC object by reading a file, but with C++ Strings instead of boring old C Strings
             * 
             * @param path archive file to read
             */
            MPARC(std::string path) : MPARC(path.c_str()) {}

            /**
             * @brief Construct a new MPARC object by copying and plagiarising other instances.
             * 
             * @param other 
             */
            MPARC(MPARC& other) : MPARC() {
                MXPSQL_MPARC_t* Ptr = other.getInstance();
                MXPSQL_MPARC_err err = MPARC_copy(&Ptr, &archive);
                MPARC_Error(err).OrThrow();
            }

            /**
             * @brief Destroy the MPARC object and its handle
             * 
             */
            ~MPARC(){
                MPARC_destroy(&archive);
            }



            /**
             * @brief Get the Last Error condition
             * 
             * @return MPARC_Error Last error condition
             */
            MPARC_Error getLastError(){
                MXPSQL_MPARC_err err = MPARC_OK;
                MXPSQL_MPARC_t* Ptr = this->getInstance();
                MPARC_get_last_error(&Ptr, &err);
                return MPARC_Error(err);
            }



            /**
             * @brief Get the internal MXPSQL_MPARC_t instance. Use this if you use with the C Functions or you need to leverage the power of pointers.
             * 
             * @return MXPSQL_MPARC_t* pointer to the internal instance
             * 
             * @note Whatever you do, resist the temptation to free this manually
             */
            MXPSQL_MPARC_t* getInstance(){
                return archive;
            }



            /**
            * @brief Plagiarised from mparc.h: Control cipher encryption;One function to control two ciphers (horrible design)
            * 
            * @param structure the target structure
            * @param SetXOR Indicate if you want to set XOR Encryption
            * @param XORKeyIn XOR Key Input
            * @param XORKeyLengthIn XOR Key Input Length
            * @param XORKeyOut XOR Key Output
            * @param XORKeyLengthOut XOR Key Output Length
            * @param SetROT Indicate if you want to set ROT Encryption
            * @param ROTKeyIn ROT Key Input
            * @param ROTKeyLengthIn ROT Key Input Length
            * @param ROTKeyOut ROT Key Output Length
            * @param ROTKeyLengthOut ROT Key Output Length
            * @return MPARC_Error Success?
            * 
            * @details
            * 
            * To disable encryption set XORKeyIn or ROTKeyIn to NULL
            * 
            * Output is first put before setting the new key, so you can get the old key.
            * 
            * @note Having the wrong encryption key will cause garbage data.
            */
            MPARC_Error cipher(
                int SetXORCipher, unsigned char* XORKeyIn, MXPSQL_MPARC_uint_repr_t XORKeyLengthIn, unsigned char** XORKeyOut, MXPSQL_MPARC_uint_repr_t* XORKeyLengthOut,
                int SetROTCipher, int* ROTKeyIn, MXPSQL_MPARC_uint_repr_t ROTKeyLengthIn, int** ROTKeyOut, MXPSQL_MPARC_uint_repr_t* ROTKeyLengthOut
            ){
                return MPARC_Error(
                    MPARC_cipher(this->getInstance(),
                        (SetXORCipher ? 1 : 0), XORKeyIn, XORKeyLengthIn, XORKeyOut, XORKeyLengthOut,
                        (SetROTCipher ? 1 : 0), ROTKeyIn, ROTKeyLengthIn, ROTKeyOut, ROTKeyLengthOut
                    )
                );
            }



            /**
             * @brief Grab an iterator instance
             * 
             * @return MPARC_Iter Yo iterator
             */
            MPARC_Iter grabIterator(){
                return MPARC_Iter(this->getInstance());
            }

            /**
             * @brief List itself to a vector
             * 
             * @param out output vector
             * @return MPARC_Error Success?
             */
            MPARC_Error list(std::vector<std::string>& out){
                MPARC_Error err(MPARC_OK);

                {
                    MPARC_Iter iter = this->grabIterator();

                    std::string name = "";

                    for(err.setErr(MPARC_OK); err.isOk() == true; err = iter.next(name)){
                        out.push_back(name);
                    }
                }

                return err;
            }

            

            /**
             * @brief Push a file
             * 
             * @param filename file to push
             * @param binary content of file
             * @param size size of binary
             * @return MPARC_Error Success?
             */
            MPARC_Error push(std::string filename, unsigned char* binary, MXPSQL_MPARC_uint_repr_t size){
                MXPSQL_MPARC_err err = MPARC_push_ufilestr(this->getInstance(), filename.c_str(), binary, size);
                return MPARC_Error(err);
            }

            /**
             * @brief Push a file stream
             * 
             * @param filename file to push
             * @param strem stream to read from
             * @return MPARC_Error Success?
             */
            MPARC_Error push(std::string filename, std::ifstream& strem){
                MPARC_Error err(MPARC_OK);
                if(!strem.is_open() || !strem.good()){
                    err.setErr(MPARC_FERROR);
                    return err;
                }

                {
                    std::vector<unsigned char> bin(std::istreambuf_iterator<char>(strem), {});

                    err = push(filename, &bin[0], bin.size());
                }

                return err;
            }

            /**
             * @brief Push a file name
             * 
             * @param filename file to push
             * @return MPARC_Error Success?
             */
            MPARC_Error push(std::string filename){
                std::ifstream strem(filename, std::ios::binary);
                return push(filename, strem);
            }



            /**
             * @brief Peek the contents
             * 
             * @param filename file to see
             * @param bout binary content
             * @param sout size of binary
             * @return MPARC_Error Success?
             */
            MPARC_Error peek(std::string filename, unsigned char** bout, MXPSQL_MPARC_uint_repr_t* sout){
                MXPSQL_MPARC_err err = MPARC_peek_file(this->getInstance(), filename.c_str(), bout, sout);
                return MPARC_Error(err);
            }



            /**
             * @brief Rename files
             * 
             * @param filename1 Old filename
             * @param filename2 New filename
             * @param overwrite Clobber/Overwrite filename2 if it exists?
             * @return MPARC_Error Success?
             */
            MPARC_Error rename(std::string filename1, std::string filename2, bool overwrite){
                MXPSQL_MPARC_err err = MPARC_rename_file(this->getInstance(), (overwrite ? 1 : 0), filename1.c_str(), filename2.c_str());
                return MPARC_Error(err);
            }

            /**
             * @brief Duplicate files
             * 
             * @param filename1 Source filename
             * @param filename2 Destination filename 
             * @param overwrite Clobber/Overwrite filename2 if it exists
             * @return MPARC_Error Success?
             */
            MPARC_Error duplicate(std::string filename1, std::string filename2, bool overwrite){
                MXPSQL_MPARC_err err = MPARC_duplicate_file(this->getInstance(), (overwrite ? 1 : 0), filename1.c_str(), filename2.c_str());
                return MPARC_Error(err);
            }

            /**
             * @brief Swap files
             * 
             * @param filename1 swap victim
             * @param filename2 another swap victim
             * @return MPARC_Error Success?
             */
            MPARC_Error swap(std::string filename1, std::string filename2){
                MXPSQL_MPARC_err err = MPARC_swap_file(this->getInstance(), filename1.c_str(), filename2.c_str());
                return MPARC_Error(err);
            }



            /**
             * @brief Pop a file off
             * 
             * @param filename file to pop off
             * @return MPARC_Error Success?
             */
            MPARC_Error pop(std::string filename){
                MXPSQL_MPARC_err err = MPARC_pop_file(this->getInstance(), filename.c_str());
                return MPARC_Error(err);
            }

            /**
             * @brief Clear the whole archive
             * 
             * @return MPARC_Error Success?
             */
            MPARC_Error clear(){
                MXPSQL_MPARC_err err = MPARC_clear(this->getInstance());
                return MPARC_Error(err);
            }




            /**
             * @brief Construct archive into a string or from a file
             * 
             * @param out output/filename
             * @param interpretation_mode How to interpret out, true to put output there, false to use it as a destination filename
             * @return MPARC_Error Success?
             */
            MPARC_Error construct(std::string& out, bool interpretation_mode){
                MXPSQL_MPARC_err err = MPARC_OK;
                if(interpretation_mode){
                    char* chout = NULL;
                    err = MPARC_construct_str(this->getInstance(), &chout);
                    out = std::string(chout);
                    MPARC_free(chout);
                }
                else{
                    std::ofstream strem(out, std::ios::binary);
                    return this->construct(strem);
                }
                return MPARC_Error(err);
            }

            /**
             * @brief Construct an archive and write it to a stream
             * 
             * @param out output stream to write into
             * @return MPARC_Error Success?
             */
            MPARC_Error construct(std::ostream& out){
                std::string i = "";
                MPARC_Error err = this->construct(i, true);
                out << i;
                return err;
            }



            /**
             * @brief Extract the archive, pro edition
             * 
             * @param dest_dir destination directory
             * @param dir2make What directory should I make?
             * @param on_item Called everytime a file is to be extracted
             * @param mk_dir Make me a directory function
             * @return MPARC_Error Success?
             */
            MPARC_Error extract(std::string dest_dir, char** dir2make, void (*on_item)(const char*), int (*mk_dir)(char*)){
                MXPSQL_MPARC_err err = MPARC_extract_advance(this->getInstance(), dest_dir.c_str(), dir2make, on_item, mk_dir);
                return MPARC_Error(err);
            }

            /**
             * @brief Extract the archive
             * 
             * @param dest_dir destination directory
             * @param dir2make What directory should I make?
             * @return MPARC_Error Success?
             */
            MPARC_Error extract(std::string dest_dir, char** dir2make){
                return this->extract(dest_dir, dir2make, NULL, NULL);
            }



            /**
             * @brief Read a directory
             * 
             * @param srcdir directory to read
             * @param recursive recursive read?
             * @param listdir function to deal with directory reading
             * @return MPARC_Error Success?
             * 
             * @details
             * 
             * > listdir Prototyping
             * 
             * the first parameter of the listdir function is the current directory that should be read from
             * 
             * the second parameter indicates if it should be recursive, its set to 0 if not, set to a non zero value (this implementation sets it to 1) if not
             * 
             * the third parameter is what files it has found, should be an array of string, terminated with NULL and Calloc'ed or Malloc'ed (pls Calloc it) (Also please use the MPARC allocation functions instead of the standard libc ones) as it relies on finding NULL and the array getting freed
             * 
             * the return value should always be 0 for success, other values indicate failure
             */
            MPARC_Error readdir(std::string srcdir, bool recursive, int (*listdir)(const char*, int, char**)){
                MXPSQL_MPARC_err err = MPARC_readdir(this->getInstance(), srcdir.c_str(), (recursive ? 1 : 0), listdir);
                return MPARC_Error(err);
            }



            /**
             * @brief Parse an archive
             * 
             * @param str Content of the archive or filename
             * @param interpretation_mode How to interpret the archive, true to interpretet as the archive content, false to interpret as a filename
             * @param erroronduplicate Error out if a duplicate is found.
             * @return MPARC_Error Success?
             */
            MPARC_Error parse(std::string str, bool interpretation_mode, bool erroronduplicate, bool sensitive){
                MXPSQL_MPARC_err err = MPARC_OK;
                if(interpretation_mode){
                    err = MPARC_parse_str_advance(this->getInstance(), str.c_str(), (erroronduplicate ? 1 : 0), sensitive);
                }
                else{
                    std::ifstream strem(str, std::ios::binary);
                    return this->parse(strem);
                }
                return MPARC_Error(err);
            }

            /**
             * @brief Parse from a stream
             * 
             * @param strem the stream to read from
             * @return MPARC_Error Success?
             */
            MPARC_Error parse(std::ifstream& strem){
                if(!strem.is_open() || !strem.good()) return MPARC_Error(MPARC_FERROR);
                std::string str = "";
                std::stringstream ss;
                ss << strem.rdbuf();
                str = ss.str();
                return this->parse(str, false, false, false);
            }
        };
    }
}



#endif
