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

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstddef>

#include <cstdlib>

#include "./mparc.h"

namespace MXPSQL{
    namespace MPARC{
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
                    std::abort();
                }
            }

            /**
             * @brief Throw a std::runtime_error if I am not fine
             * 
             */
            void OrThrow(){
                if(this->getErr() != MPARC_OK){
                    std::string str = std::string("MPARC Runtime Error with code of ") + std::to_string(((int)this->getErr()));
                    throw std::runtime_error(str.c_str());
                }
            }

            bool isOk(){return (this->getErr() == MPARC_OK);}
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
                MXPSQL_MPARC_err err = MPARC_parse_filename(archive, path);
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
             * @brief Get the internal MXPSQL_MPARC_t instance
             * 
             * @return MXPSQL_MPARC_t* pointer to the internal instance
             * 
             * @note Whatever you do, resist the temptation to free this manually
             */
            MXPSQL_MPARC_t* getInstance(){
                return archive;
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
                    MXPSQL_MPARC_iter_t* iter = NULL;
                    MXPSQL_MPARC_err ierr = MPARC_list_iterator_init(&archive, &iter);
                    if(ierr != MPARC_OK){
                        err.setErr(ierr);
                        return err;
                    }

                    const char* outnam = NULL;

                    while((ierr = MPARC_list_iterator_next(&iter, &outnam)) == MPARC_OK){
                        out.push_back(std::string(outnam));
                    }

                    MPARC_list_iterator_destroy(&iter);

                    if(ierr != MPARC_KNOEXIST){
                        err.setErr(ierr);
                        return err;
                    }
                }

                return err;
            }            
        };
    }
}



#endif
