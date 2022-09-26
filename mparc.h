#ifndef _MXPSQL_MPARC_H
#define _MXPSQL_MPARC_H

/**
  * MPARC, A Dumb Archiver Format C Rewrite Of MPAR
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

#ifndef __STDC_VERSION__
#error "Minimum C Version is C99"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
#include <cstdio>
extern "C"{
#else
#include <stdio.h>
#endif

    /**
     * @brief Error states, these are self explanatory. But I give them brief anyways
     * 
     */
    typedef enum MXPSQL_MPARC_err {
        /**
         * @brief Everything's gine
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
         * @brief Invalid value or generic but more specific errors than MPARC_IDK
         * 
         */
        MPARC_IVAL = 1,
        /**
         * @brief Key does not exist
         * 
         */
        MPARC_NOEXIST=2,

        /**
         * @brief Out Of Memory!
         * 
         */
        MPARC_OOM = 3,

        /**
         * @brief The archive you provided is actually not an archive determined by the 25 character long MAGIC NUMBER.
         * 
         */
        MPARC_NOTARCHIVE=4,
        /**
         * @brief The format version of the archive is too new to be digested
         * 
         */
        MPARC_ARCHIVETOOSHINY=5,
        /**
         * @brief CRC checksum failed, unused as of now
         * 
         */
        MPARC_CHKSUM=6,

        /**
         * @brief Operation not complete
         * 
         */
        MPARC_OPPART=7,

        /**
         * @brief FILE* seems to be having problems as ferror reports true or just that something is wrong with the archive itself
         * 
         */
        MPARC_FERROR=8
    } MXPSQL_MPARC_err;
    int MPARC_strerror(MXPSQL_MPARC_err err, char** out);
    int MPARC_sfperror(MXPSQL_MPARC_err err, FILE* filepstream, char* emsg);
    int MPARC_fperror(MXPSQL_MPARC_err err, FILE* fileptrstream);
    int MPARC_perror(MXPSQL_MPARC_err err);


    /**
     * @brief Opaque Implementation Struct
     * 
     */
    typedef struct MXPSQL_MPARC_impleq_t MXPSQL_MPARC_impleq_t;
    /**
     * @brief Ptr type, should be initialized to null on first use
     * 
     * @details
     * 
     * Pointer type to MXPSQL_MPARC_impleq_t.
     * 
     * This should be initialized to NULL to prevent problems with uninitialized value.
     * 
     * Don't ever try to dereference this thing.
     * 
     * This can never be declared as a non pointer object.
     * 
     * @see MXPSQL_MPARC_impleq_t
     */
    typedef struct MXPSQL_MPARC_impleq_t MXPSQL_MPARC_t;

    /**
     * @brief Initialize sturcture
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code
     */
    MXPSQL_MPARC_err MPARC_init(MXPSQL_MPARC_t** structure);
    /**
     * @brief Tear down structure
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code
     */
    MXPSQL_MPARC_err MPARC_destroy(MXPSQL_MPARC_t* structure);

    /**
     * @brief List out the current files included
     * 
     * @param structure the target structure
     * @param listout the output list
     * @param length the length of listout
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note Free listout manually with 'free'
     */
    MXPSQL_MPARC_err MPARC_list(MXPSQL_MPARC_t* structure, char*** listout, size_t* length);
    /**
     * @brief Check if file entry exists
     * 
     * @param structure the target structure
     * @param filename the filename to check
     * @return MXPSQL_MPARC_err the status code if successfully done or errors out
     */
    MXPSQL_MPARC_err MPARC_exists(MXPSQL_MPARC_t* structure, char* filename);

    /**
     * @brief Push an unsigned string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param ustringc the bytes of string
     * @param sizy the size of ustringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_ufilestr(MXPSQL_MPARC_t* structure, char* filename, unsigned char* ustringc, size_t sizy);
    /**
     * @brief Push a string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param stringc the bytes of string
     * @param sizey the size of stringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_filestr(MXPSQL_MPARC_t* structure, char* filename, char* stringc, size_t sizey);
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
     * @param filestream the active filestream to read from
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
    MXPSQL_MPARC_err MPARC_peek_file(MXPSQL_MPARC_t* structure, char* filename, unsigned char** bout, size_t* sout);

    /**
     * @brief Construct the archive into a string
     * 
     * @param structure the target structure
     * @param output the storage string
     * @return MXPSQL_MPARC_err the status code if successfully done
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
     * @param fpstream the file stream
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note fpstream should be closed and opened manually
     */
    MXPSQL_MPARC_err MPARC_construct_filestream(MXPSQL_MPARC_t* structure, FILE* fpstream);


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
     * @brief Extract the archive into the directory
     * 
     * @param structure the target structure
     * @param destdir the destination directory
     * @param dir2make NULL if there is no directory to make, not NULL if it needs you to make a directory
     * @param on_item called every time a new entry is read, NULL can be placed in
     * @param mk_dir function to be called when making a directory, NULL can be placed in. Overrides dir2make on certain platform, may not be called on other platform
     * @return MXPSQL_MPARC_err err status of extraction, some codes are special.
     * 
     * @details
     * 
     * Special Error Codes: MPARC_OPPART, it has interrupted it's operation and asking the user for assistance.
     * 
     * - dir2make is not NULL: use the variable dir2make and make a directory. Will not happen if platform supports ENOENT and mk_dir is not NULL
     */
    MXPSQL_MPARC_err MPARC_extract_advance(MXPSQL_MPARC_t* structure, char* destdir, char** dir2make, void (*on_item)(const char*), int (*mk_dir)(char*));

    /**
     * @brief Parse the archive into the structure
     * 
     * @param structure the target structure
     * @param stringy the string to be parsed to
     * @return MXPSQL_MPARC_err Did it parse well or did not
     */
    MXPSQL_MPARC_err MPARC_parse_str(MXPSQL_MPARC_t* structure, char* stringy);
    /**
     * @brief Parse the opened file stream archive into the structure
     * 
     * @param structure the target structure
     * @param fpstream the stream to read from
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

#endif