#ifndef _MXPSQL_MPARC_H
#define _MXPSQL_MPARC_H

/**
  * @file mparc.h
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. C Header With Implementation. Never reeterant, thread and async safe.
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

#if (!(defined(__STDC__) && __STDC__))
#error "C must be ANSI C, not K&R C or non ANSI C compliant C"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
#include <cstdio>
#include <cstdarg>
#include <cinttypes>
extern "C"{
#else
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#endif

    /**
     * @brief Error states, these are self explanatory. But I give them brief anyways
     * 
     */
    typedef enum MXPSQL_MPARC_err {
        /**
         * @brief Everything's fine
         * 
         */
        MPARC_OK = 0,

        /**
         * @brief Generic error
         * 
         */
        MPARC_IDK = -1,
        /**
         * @brief Internal problem, not used.
         * 
         */
        MPARC_INTERNAL = -2,
        /**
         * @brief NULL input
         * 
         */
        MPARC_NULL = -3,

        /**
         * @brief Invalid value or generic but more specific errors than MPARC_IDK
         * 
         */
        MPARC_IVAL = 1,

        /**
         * @brief Key does not exist
         * 
         */
        MPARC_KNOEXIST=2,
        /**
         * @brief Key exists
         * 
         */
        MPARC_KEXISTS=3,

        /**
         * @brief Out Of Memory!
         * 
         */
        MPARC_OOM = 4,

        /**
         * @brief The archive you provided is actually not an archive determined by the 25 character long MAGIC NUMBER or some other factor, but most likely the magic number.
         * 
         */
        MPARC_NOTARCHIVE=5,
        /**
         * @brief The format version of the archive is too new to be digested
         * 
         */
        MPARC_ARCHIVETOOSHINY=6,
        /**
         * @brief CRC checksum check and comparison failed.
         * 
         */
        MPARC_CHKSUM=7,

        /**
         * @brief Failure to construct archive
        */
        MPARC_CONSTRUCT_FAIL=8,

        /**
         * @brief Operation not complete
         * 
         */
        MPARC_OPPART=9,

        /**
         * @brief FILE* seems to be having problems as ferror reports true or just that something is wrong with the archive itself
         * 
         */
        MPARC_FERROR=10
    } MXPSQL_MPARC_err;
    int MPARC_strerror(MXPSQL_MPARC_err err, char** out);
    int MPARC_sfperror(MXPSQL_MPARC_err err, FILE* filepstream, char* emsg);
    int MPARC_fperror(MXPSQL_MPARC_err err, FILE* fileptrstream);
    int MPARC_perror(MXPSQL_MPARC_err err);

    /**
     * @brief Ptr type of the archive, should be initialized to null on first use
     * 
     * @details
     * 
     * This should be initialized to NULL to prevent problems with uninitialized value.
     * 
     * Don't ever try to dereference this thing.
     * 
     * This can never be declared as a non pointer object.
     */
    typedef struct MXPSQL_MPARC_t MXPSQL_MPARC_t;
    /**
     * @brief Ptr type of the iterator, should be initialized to null on first use
     * 
     * @details
     * 
     * This should be initialized to NULL to prevent problems with uninitialized value.
     * 
     * Don't ever try to dereference this thing.
     * 
     * This can never be declared as a non pointer object.
     */
    typedef struct MXPSQL_MPARC_iter_t MXPSQL_MPARC_iter_t;

    /**
     * @brief Initialize sturcture
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code
     */
    MXPSQL_MPARC_err MPARC_init(MXPSQL_MPARC_t** structure);
    /**
     * @brief Initialize structure by copying
     * 
     * @param structure the target structure for copying
     * @param targetdest the destination target structure to be overwritten
     * @return MXPSQL_MPARC_err the status code
     */
    MXPSQL_MPARC_err MPARC_copy(MXPSQL_MPARC_t** structure, MXPSQL_MPARC_t** targetdest);
    /**
     * @brief Tear down structure
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code
     */
    MXPSQL_MPARC_err MPARC_destroy(MXPSQL_MPARC_t** structure);

    /**
     * @brief List out the current files included as an array
     * 
     * @param structure the target structure
     * @param listout the output list
     * @param length the length of listout
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note Free listout manually with 'free', not 'delete'
     */
    MXPSQL_MPARC_err MPARC_list_array(MXPSQL_MPARC_t* structure, char*** listout, uint_fast64_t* length);
    /**
     * @brief Initialize the iterator that list the current files included
     * 
     * @param structure the target structure
     * @param iterator the target iterator
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_list_iterator_init(MXPSQL_MPARC_t** structure, MXPSQL_MPARC_iter_t** iterator);
    /**
     * @brief Update the state of the iterator to point to the next one
     * 
     * @param iterator the target iterator
     * @param outnam output string
     * @return MXPSQL_MPARC_err 
     * 
     * @details
     * 
     * If it returns KMPARC_NOEXIST, the iterator has reached the end
     */
    MXPSQL_MPARC_err MPARC_list_iterator_next(MXPSQL_MPARC_iter_t** iterator, const char** outnam);
    /**
     * @brief Destroy the iterator
     * 
     * @param iterator the target iterator
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_list_iterator_destroy(MXPSQL_MPARC_iter_t** iterator);
    /**
     * @brief Check if file entry exists
     * 
     * @param structure the target structure
     * @param filename the filename to check
     * @return MXPSQL_MPARC_err the status code if successfully done or errors out
     */
    MXPSQL_MPARC_err MPARC_exists(MXPSQL_MPARC_t* structure, char* filename);
    /**
     * @brief Query the structre for files that match a specific criteria
     * 
     * @param structure the target structure
     * @param output The files that match a specific criteria.
     * @param command The criteria to look for
     * @param ... Vaargs that depend on the command arg
     * @return MXPSQL_MPARC_err Success or a failure (MPARC_SUCCESS if yes, MPARC_KNOEXIST if the command is invalid, MPARC_IVAL if a problem occured)
     * 
     * @details
     * 
     * There are multiple commands available in this query function.
     * This list is formated as the following: [command] - [description] - [required vaargs]
     * Available commands (case sensitive):
     * size_bigger - List all files that is bigger than the specified size - [number, preferably uint_fast64_t, a 1 is equal to a single byte (2 means two bytes, 3 means three bytes, etc...)]
     * size_equal - List all files that is equal to the specified size - [number, preferably uint_fast64_t, a 1 is equal to a single byte (2 means two bytes, 3 means three bytes, etc...)]
     * size_smaller - List all files that is small than the specified size - [number, preferably uint_fast64_t, a 1 is equal to a single byte (2 means two bytes, 3 means three bytes, etc...)]
     * extension - Get the file extension based from the first dot - [a string of the file extension, do not include the first dot in the parameter]
     * rextension - Get the file extension based from the last dot - [a string of the file extension, do not include the first dot in the parameter]
     * 
     * The output parameter is always terminated with NULL. It itself and its content is also dynamically allocated, so you must deallocate it manually using 'free', not 'delete[]'.
     * 
     * Passing the wrong vaargs will lead to undefined behavior, I cannot defend you from against that unless you cooperate with me. C Standard Library says so to do undefined behaviour in vaargs implementation.
     */
    MXPSQL_MPARC_err MPARC_query(MXPSQL_MPARC_t* structure, char*** output, char* command, ...);
    /**
     * @brief A version of MPARC_query that accepts a va_list instead
     * 
     * @param structure the target structure
     * @param output The files that match a specific criteria
     * @param command The criteria to look for
     * @param vlist A list that is suppoused to be a vaargs that depend on the command arg
     * @return MXPSQL_MPARC_err Success or a failure (MPARC_SUCCESS if yes, MPARC_KNOEXIST if the command is invalid, MPARC_IVAL if a problem occured)
     * 
     * @see MPARC_query
     * 
     * @note You are responsible for starting and ending vlist. Also look at MPARC_query for more bad things that could happen.
     */
    MXPSQL_MPARC_err MPARC_query_vlist(MXPSQL_MPARC_t* structure, char*** output, char* command, va_list vlist);
    
    /**
     * @brief Push an unsigned string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param stripdir strip the directory from the filename, set to 0 to disable
     * @param overwrite if set to anything other than a 0, overwrites the entry if it exists. if set to 0, prevents you from overwriting it if it exists. set to 0 to disable ability to overwrite (already explained)
     * @param ustringc the bytes of string
     * @param sizy the size of ustringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note Filename only works on forward slash if stripdir is set due to basename only supporting that operation.
     */
    MXPSQL_MPARC_err MPARC_push_ufilestr_advance(MXPSQL_MPARC_t* structure, char* filename, int stripdir, int overwrite, unsigned char* ustringc, uint_fast64_t sizy);
    /**
     * @brief Simple version of MPARC_push_ufilestr_advance that does not strip the directory name
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param ustringc the bytes of string
     * @param sizy the size of ustringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @see MPARC_push_ufilestr_advance
     */
    MXPSQL_MPARC_err MPARC_push_ufilestr(MXPSQL_MPARC_t* structure, char* filename, unsigned char* ustringc, uint_fast64_t sizy);
    /**
     * @brief Push a void pointer as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param buffer_guffer the void pointer
     * @param sizy the size of buffer_guffer
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_voidfile(MXPSQL_MPARC_t* structure, char* filename, void* buffer_guffer, uint_fast64_t sizy);
    /**
     * @brief Push a string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param stringc the bytes of string
     * @param sizey the size of stringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_filestr(MXPSQL_MPARC_t* structure, char* filename, char* stringc, uint_fast64_t sizey);
    /**
     * @brief Push a file read from the filesystem into the archive
     * 
     * @param structure the target structure
     * @param filename the filename to read from
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_filename(MXPSQL_MPARC_t* structure, char* filename);
    /**
     * @brief Push an active filestream into the archive as a file
     * 
     * @param structure the target structure
     * @param filestream the active filestream to read from, should be opened in "rb" mode
     * @param filename the filename to assign
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note filestream should be opened and closed manually
     */
    MXPSQL_MPARC_err MPARC_push_filestream(MXPSQL_MPARC_t* structure, FILE* filestream, char* filename);

    /**
     * @brief Pop a file off the archive
     * 
     * @param structure the target structure
     * @param filename the filename to pop off
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_pop_file(MXPSQL_MPARC_t* structure, char* filename);
    /**
     * @brief Wipe everything (I mean every single file) off the archive
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_clear_file(MXPSQL_MPARC_t* structure);

    /**
     * @brief Peek the contents of a file of the archive
     * 
     * @param structure the target structure
     * @param filename the filename to peek the contents at
     * @param bout the output pointer to a variable that represent the binary content of the file
     * @param sout the output pointer to a variable that represent the size of bout
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_peek_file(MXPSQL_MPARC_t* structure, char* filename, unsigned char** bout, uint_fast64_t* sout);

    /**
     * @brief Construct the archive into a string
     * 
     * @param structure the target structure
     * @param output the storage string
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note manually free output string
     */
    MXPSQL_MPARC_err MPARC_construct_str(MXPSQL_MPARC_t* structure, char** output);
    /**
     * @brief Construct the archive into a file
     * 
     * @param structure the target structure
     * @param filename the filename target
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_construct_filename(MXPSQL_MPARC_t* structure, char* filename);
    /**
     * @brief Construct the archive into a file stream
     * 
     * @param structure the target structure
     * @param fpstream the file stream, should be opened in "rb" mode
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note fpstream should be closed and opened manually
     */
    MXPSQL_MPARC_err MPARC_construct_filestream(MXPSQL_MPARC_t* structure, FILE* fpstream);


    /**
     * @brief Advanced extraction function
     * 
     * @param structure the target structure
     * @param destdir the destination directory
     * @param dir2make NULL if there is no directory to make, not NULL if it needs you to make a directory
     * @param on_item invoked everytime a new item is iterated over
     * @param mk_dir invoked when directory is needed to be created, return 0 on success, non-zero on error. Overrides dir2make.
     * @return MXPSQL_MPARC_err error status, some code are special, see details
     * 
     * @details
     * 
     * if the error code returns MPARC_OPPART, check dir2make to see if it needs you to make a new directory
     */
    MXPSQL_MPARC_err MPARC_extract_advance(MXPSQL_MPARC_t* structure, char* destdir, char** dir2make, void (*on_item)(const char*), int (*mk_dir)(char*));
    /**
     * @brief Simple version of MPARC_extract_advance
     * 
     * @param structure the target structure
     * @param destdir the destination directory
     * @param dir2make NULL if there is no directory to make, not NULL if it needs you to make a directory
     * @return MXPSQL_MPARC_err error status of extraction, some codes are special, see MPARC_extract_advance for more info
     * 
     * @see MPARC_extract_advance
     */
    MXPSQL_MPARC_err MPARC_extract(MXPSQL_MPARC_t* structure, char* destdir, char** dir2make);

    /**
     * @brief Read a directory into the structure
     * 
     * @param structure the target structure
     * @param srcdir the source directory to read from
     * @param recursive read from subdirectories if not set to 0
     * @param listdir function to list a directory, shall not be NULL or else it returns an error. This is the function that handles listing the file and recursion.
     * @return MXPSQL_MPARC_err error status of reading
     * 
     * @details
     * 
     * > listdir Prototyping
     * 
     * the first parameter of the listdir function is the current directory that should be read from
     * 
     * the second parameter indicates if it should be recursive, set to 0 if not, don't if not
     * 
     * the third parameter is what files it has found, should be an array of string, terminated with NULL and Calloc'ed or Malloc'ed (pls Calloc it) as it relies on finding NULL and the array getting freed
     * 
     * the return value should always be 0 for success, other values indicate failure
     */
    MXPSQL_MPARC_err MPARC_readdir(MXPSQL_MPARC_t* structure, char* srcdir, int recursive, int (*listdir)(char*, int, char**));

    /**
     * @brief Parse the archive into the structure with extra flags
     * 
     * @param structure the target structure
     * @param stringy the string to be parsed to
     * @param erronduplicate error with returning MPARC_KEXISTS if the key exists
     * @return MXPSQL_MPARC_err Did it parse well or did not
     */
    MXPSQL_MPARC_err MPARC_parse_str_advance(MXPSQL_MPARC_t* structure, char* stringy, int erronduplicate);
    /**
     * @brief Parse the archive into the structure, a simpler version of MPARC_parse_str_advance
     * 
     * @param structure the target structure
     * @param stringy the string to be parsed to
     * @return MXPSQL_MPARC_err Did it parse well or did not
     * 
     * @details
     * 
     * It is MPARC_parse_str_advance with the following options:
     * 
     * - Overwrite entries if duplicate found
     * 
     * @see MPARC_parse_str_advance
     */
    MXPSQL_MPARC_err MPARC_parse_str(MXPSQL_MPARC_t* structure, char* stringy);
    /**
     * @brief Parse the opened file stream archive into the structure
     * 
     * @param structure the target structure
     * @param fpstream the stream to read from, should be opened in "rb" mode
     * @return MXPSQL_MPARC_err Did it parse well or did not
     */
    MXPSQL_MPARC_err MPARC_parse_filestream(MXPSQL_MPARC_t* structure, FILE* fpstream);
    /**
     * @brief Parse the archive file from the filename into the structure
     * 
     * @param structure the target structure
     * @param filename the filename to read from
     * @return MXPSQL_MPARC_err Did it parse well or did not
     */
    MXPSQL_MPARC_err MPARC_parse_filename(MXPSQL_MPARC_t* structure, char* filename);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // end of header
