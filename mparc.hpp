/**
  * @file mparc.hpp
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. C++ Wrapper
  * @version 0.1
  * @date 2022-09-26
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

#include <string>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include "./mparc.h"


namespace MXPSQL{
    namespace MPARC{
        class MPARC_Exception : public std::exception {
            protected:
            const char* msg = NULL;
            public:
            MPARC_Exception(const char* message){
                msg = message;
            }
            MPARC_Exception(MXPSQL_MPARC_err err){
                char* message = NULL;
                MPARC_strerror(err, &message);
                msg = message;
                free(message);
            }
            MPARC_Exception() : MPARC_Exception(MPARC_OK){}
            virtual const char* what() const noexcept {
                return (msg ? msg : "");
            }
        };

        class MPARC{
            protected:
            MXPSQL_MPARC_t* archive_handlet;

            public:
            MPARC(){
                MXPSQL_MPARC_err err = MPARC_init(&archive_handlet);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }


            std::vector<std::string> list(){
                char** listy_listout;
                size_t listy_sizey;
                MXPSQL_MPARC_err err = MPARC_list(archive_handlet, &listy_listout, &listy_sizey);
                if(err != MPARC_OK){
                    free(listy_listout);
                    throw MPARC_Exception(err);
                }
                std::vector<std::string> lists;
                for(size_t i = 0; i < listy_sizey; i++){
                    lists.push_back(std::string(listy_listout[i]));
                }
                free(listy_listout);
                return lists;
            }

            bool exists(char* filename){
                return MPARC_exists(archive_handlet, filename) == MPARC_OK;
            }

            bool exists(std::string filename){
                return exists(const_cast<char*>(filename.c_str()));
            }


            void push_file(char* filename, unsigned char* ufilestr, size_t size){
                MXPSQL_MPARC_err err = MPARC_push_ufilestr(archive_handlet, filename, ufilestr, size);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void push_file(char* filename, void* voidfile, size_t size){
                MXPSQL_MPARC_err err = MPARC_push_voidfile(archive_handlet, filename, voidfile, size);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void push_file(char* filename, char* stringc, size_t size){
                MXPSQL_MPARC_err err = MPARC_push_filestr(archive_handlet, filename, stringc, size);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void push_file(char* filename, char* path){
                MXPSQL_MPARC_err err = MPARC_push_filename(archive_handlet, path);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void push_file(char* filename, FILE* filepointerstream){
                MXPSQL_MPARC_err err = MPARC_push_filestream(archive_handlet, filepointerstream, filename);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void push_file(std::string filename, std::ifstream& std_inputfilestream){
                std::vector<char> bytes;
                std::streampos size = std_inputfilestream.tellg();
                if(std_inputfilestream.good()){
                    {
                        std_inputfilestream.seekg(0, std::ios_base::end);
                        bytes.resize(size);
                    }
                    std_inputfilestream.seekg(0, std::ios_base::beg);
                    std_inputfilestream.read(&bytes[0], size);
                
                    {
                        MXPSQL_MPARC_err err = MPARC_push_filestr(archive_handlet, const_cast<char*>(filename.c_str()), &bytes[0], size);
                    }
                }
                else{
                    throw MPARC_Exception(MPARC_FERROR);
                }
            }

            void push_file(std::string filename){
                std::ifstream fstrm(filename, std::ios::binary);
                push_file(filename, fstrm);
            }

            void push_file(std::string filename, std::string bytes){
                std::vector<char> rbytes(bytes.begin(), bytes.end());
                push_file(const_cast<char*>(filename.c_str()), &rbytes[0], rbytes.size());
            }


            void pop_file(char* filename){
                MXPSQL_MPARC_err err = MPARC_pop_file(archive_handlet, filename);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void pop_file(std::string filename){
                pop_file(const_cast<char*>(filename.c_str()));
            }


            void clear_file(){
                MXPSQL_MPARC_err err = MPARC_clear_file(archive_handlet);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }


            void peek_file(char* filename, unsigned char** bout, size_t* sout){
                MXPSQL_MPARC_err err = MPARC_peek_file(archive_handlet, filename, bout, sout);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void peek_file(std::string filename, std::vector<unsigned char>& bout, size_t* sout){
                unsigned char* rttbout = &bout[0];
                unsigned char** rtbout = &rttbout;
                peek_file(const_cast<char*>(filename.c_str()), rtbout, sout);
            }


            void construct(char** out){
                MXPSQL_MPARC_err err = MPARC_construct_str(archive_handlet, out);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void construct(std::string* sout){
                std::vector<char> bout;
                {
                    char* rttbout = &bout[0];
                    char** rtbout = &rttbout;
                    construct(rtbout);
                }
                if(sout != NULL) *sout = std::string(bout.begin(), bout.end());
            }

            void construct(FILE* fpstream){
                MXPSQL_MPARC_err err = MPARC_construct_filestream(archive_handlet, fpstream);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void construct(std::ofstream& os){
                std::string sout;
                construct(sout);
                if(os.good()){
                    os.write(sout.c_str(), sizeof(char)*sout.size());
                }
                else{
                    throw MPARC_Exception(MPARC_FERROR);
                }
            }

            void construct(char* filename){
                MXPSQL_MPARC_err err = MPARC_construct_filename(archive_handlet, filename);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void construct(std::string filename){
                construct(const_cast<char*>(filename.c_str()));
            }


            void extract(char* destdir, char** dir2make, void (*on_item)(const char*), int (*mk_dir)(char*)){
                MXPSQL_MPARC_err err = MPARC_extract_advance(archive_handlet, destdir, dir2make, on_item, mk_dir);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void extract(char* destdir, char** dir2make){
                MXPSQL_MPARC_err err = MPARC_extract(archive_handlet, destdir, dir2make);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }

            void extract(std::string destdir, std::string& dir2make, void (*on_item)(const char*), int (*mk_dir)(char*)){
                std::vector<char> d2;
                {
                    char* rttd2 = &d2[0];
                    char** rtd2 = &rttd2;
                    extract(const_cast<char*>(destdir.c_str()), rtd2, on_item, mk_dir);
                    dir2make = std::string(d2.begin(), d2.end());
                }
            }


            void parse(char* str){
                MXPSQL_MPARC_err err = MPARC_parse_str(archive_handlet, str);
                if(err != MPARC_OK){
                    throw MPARC_Exception(err);
                }
            }


            ~MPARC(){
                MPARC_destroy(archive_handlet);
            }
        };
    }
}
