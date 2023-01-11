#pragma once
#ifndef _MXPSQL_MPARC_H
#define _MXPSQL_MPARC_H

/**
  * @file mparc.h
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. C Header. Reentrant functions. Not thread and async safe, probably.
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
#include <cstdint>
#include <cstddef>
#include <cstdbool>
extern "C"{
#else
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h> 
#include <stdbool.h>
#endif


    // Types
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
     * 
     * Not atomic or thread safe (never aim to be that for C99 suport and portability, you do it yourself with platform threads (pthreads, winapi threads or C11 threads if you can)).
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
     * @brief Typedef our uint representation to make it easy to refactor
     * 
     * @details
     * 
     * This is currently typedef'd to uint_fast64_t, but we can switch to unsigned long long or size_t
     */
    typedef uint_fast64_t MXPSQL_MPARC_uint_repr_t;
    /**
     * @brief A function that performs preprocessing for the archive on its file
     * 
     * @note unused
     * 
     * @details
     * 
     * The first parameter is the input
     * 
     * The second parameter is the output
     */
    typedef int (*MXPSQL_MPARC_preprocessor_func_t)(char*, char**);


    // Error reporting
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
         * @brief Internal problem, should never see this. If you see this, it is a serious error and you should abort.
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
        MPARC_KNOEXIST,
        /**
         * @brief Key exists
         * 
         */
        MPARC_KEXISTS,

        /**
         * @brief Out Of Memory!
         * 
         */
        MPARC_OOM = 4,

        /**
         * @brief The archive you provided is actually not an archive determined by the 25 character long MAGIC NUMBER or some other factor, but most likely the magic number.
         * 
         */
        MPARC_NOTARCHIVE = 5,
        /**
         * @brief The format version of the archive is too new to be digested
         * 
         */
        MPARC_ARCHIVETOOSHINY,
        /**
         * @brief CRC checksum check and comparison failed.
         * 
         */
        MPARC_CHKSUM = 7,
        /**
         * @brief No encryption is set
         * 
         */
        MPARC_NOCRYPT  =8,

        /**
         * @brief Failure to construct archive
        */
        MPARC_CONSTRUCT_FAIL,

        /**
         * @brief Operation not complete
         * 
         */
        MPARC_OPPART = 10,

        /**
         * @brief FILE* seems to be having problems as ferror reports true or just that something is wrong with the archive itself
         * 
         */
        MPARC_FERROR = 11
    } MXPSQL_MPARC_err;
    /**
     * @brief Get last error from internally maintained state (If one of your functions does not return MXPSQL_MPARC_err, how cruel of you to do that by forcing users to resort to this function).
     * 
     * @param structure the target structure
     * @param out Get your error state here
     * @return MXPSQL_MPARC_err Level of error or status during lookup (not int though)
     */
    MXPSQL_MPARC_err MPARC_get_last_error(MXPSQL_MPARC_t** structure, MXPSQL_MPARC_err* out);
    /**
     * @brief Get error string from MXPSQL_MPARC_err
     * 
     * @param err error code
     * @param out output error string, can be NULL if an error occured
     * @return int Level of error (-1 for invalid, 0 for normal, 1 for special, 2 for serious conditions)
     */
    int MPARC_strerror(MXPSQL_MPARC_err err, char** out);
    /**
     * @brief Print the error message to your stream of choice with a message of your choice
     * 
     * @param err error code
     * @param filepstream stream of your choice
     * @param emsg message of your choice
     * @return int Level of error
     * 
     * @see MPARC_strerror
     */
    int MPARC_sfperror(MXPSQL_MPARC_err err, FILE* filepstream, const char* emsg);
    /**
     * @brief Print the error message of your stream of choice
     * 
     * @param err error code
     * @param fileptrstream stream of your choice
     * @return int Level of error
     * 
     * @see MPARC_sfperror
     */
    int MPARC_fperror(MXPSQL_MPARC_err err, FILE* fileptrstream);
    /**
     * @brief Print the error message to stderr
     * 
     * @param err error code
     * @return int Level of error
     * 
     * @see MPARC_fperror
     */
    int MPARC_perror(MXPSQL_MPARC_err err);


    // Main functions
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
     * @brief Control cipher encryption;One function to control two ciphers (horrible design)
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
     * @return MXPSQL_MPARC_err the status code: MPARC_OK if any encryption is set, MPARC_NOCRYPT if no encryption is set
     * 
     * @details
     * 
     * To disable encryption set XORKeyIn or ROTKeyIn to NULL
     * 
     * Output is first put before setting the new key, so you can get the old key.
     * 
     * @note Having the wrong encryption key will cause garbage data or a checksum failure.
     */
    MXPSQL_MPARC_err MPARC_cipher(MXPSQL_MPARC_t* structure, 
    bool SetXOR, unsigned char* XORKeyIn, MXPSQL_MPARC_uint_repr_t XORKeyLengthIn, unsigned char** XORKeyOut, MXPSQL_MPARC_uint_repr_t* XORKeyLengthOut,
    bool SetROT, int* ROTKeyIn, MXPSQL_MPARC_uint_repr_t ROTKeyLengthIn, int** ROTKeyOut, MXPSQL_MPARC_uint_repr_t* ROTKeyLengthOut);

    /**
     * @brief List out the current files included as an array
     * 
     * @param structure the target structure
     * @param listout the output list
     * @param length the length of listout
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note Free listout manually with MPARC_list_array_free, not 'free' or 'delete' for forward compatibility. Using 'MPARC_free' would get you memory leaks instead.
     * 
     * @see MPARC_list_array
     */
    MXPSQL_MPARC_err MPARC_list_array(MXPSQL_MPARC_t* structure, char*** listout, MXPSQL_MPARC_uint_repr_t* length);
    /**
     * @brief Utility function that free the list that you created with MPARC_list_array
     * 
     * @param list the list you got
     * @return MXPSQL_MPARC_err successful?
     * 
     * @see MPARC_list_array
     */
    MXPSQL_MPARC_err MPARC_list_array_free(char*** list);
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
     * @brief Foreach, with call backs
     * 
     * @param structure the target structure
     * @param cb_aborted a flag that indicated if an error code resulted from abortion request (true) or an internal error (false). Can be NULL.
     * @param callback callback function that gets called on every iteration, a void parameter is provided for context
     * @param cb_ctx context value that is passed to the void* parameter of callback
     * @return MXPSQL_MPARC_err the status code if successfully done. MPARC_OK if successfull, other error codes if aborted (dependent on callback).
     * 
     * @details
     * 
     * For the callback function:
     * 
     * To continue an iteration, return MPARC_OK.
     * 
     * To abort the loop, return any other values in MXPSQL_MPARC_err other than MPARC_OK. cb_aborted is set to 1 and the value is returned from callback.
     * 
     * Other details:
     * 
     * If an internal error occurs, the error code corresponding to the error is returned, but cb_aborted is set to 0.
     */
    MXPSQL_MPARC_err MPARC_list_foreach(MXPSQL_MPARC_t* structure, bool* cb_aborted, MXPSQL_MPARC_err (*callback)(MXPSQL_MPARC_t*, const char*, void*), void* cb_ctx);
    /**
     * @brief Check if file entry exists
     * 
     * @param structure the target structure
     * @param filename the filename to check
     * @return MXPSQL_MPARC_err the status code if successfully done or errors out
     */
    MXPSQL_MPARC_err MPARC_exists(MXPSQL_MPARC_t* structure, const char* filename);
    
    /**
     * @brief Push an unsigned string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param stripdir strip the directory from the filename
     * @param overwrite overwrites the entry if it exists
     * @param ustringc the bytes of string
     * @param sizy the size of ustringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note Filename only works on forward slash if stripdir is set due to basename only supporting that operation.
     */
    MXPSQL_MPARC_err MPARC_push_ufilestr_advance(MXPSQL_MPARC_t* structure, const char* filename, bool stripdir, bool overwrite, unsigned char* ustringc, MXPSQL_MPARC_uint_repr_t sizy);
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
    MXPSQL_MPARC_err MPARC_push_ufilestr(MXPSQL_MPARC_t* structure, const char* filename, unsigned char* ustringc, MXPSQL_MPARC_uint_repr_t sizy);
    /**
     * @brief Push a void pointer as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param buffer_guffer the void pointer
     * @param sizy the size of buffer_guffer
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_voidfile(MXPSQL_MPARC_t* structure, const char* filename, void* buffer_guffer, MXPSQL_MPARC_uint_repr_t sizy);
    /**
     * @brief Push a string as a file
     * 
     * @param structure the target structure
     * @param filename the filename to assign
     * @param stringc the bytes of string
     * @param sizey the size of stringc
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_filestr(MXPSQL_MPARC_t* structure, const char* filename, char* stringc, MXPSQL_MPARC_uint_repr_t sizey);
    /**
     * @brief Push a file read from the filesystem into the archive
     * 
     * @param structure the target structure
     * @param filename the filename to read from
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_push_filename(MXPSQL_MPARC_t* structure, const char* filename);
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
    MXPSQL_MPARC_err MPARC_push_filestream(MXPSQL_MPARC_t* structure, FILE* filestream, const char* filename);

    /**
     * @brief Rename an entry
     * 
     * @param structure the target structure
     * @param overwrite Overwrite an entry?
     * @param oldname the file you want to change name
     * @param newname the new name
     * @return MXPSQL_MPARC_err the sttaus code if successfully done. MPARC_KNOEXIST if oldname is not there and more...
     * 
     * @details
     * 
     * Internally implemented with MPARC_push_ufilestr, MPARC_pop_file and MPARC_peek_file
     */
    MXPSQL_MPARC_err MPARC_rename_file(MXPSQL_MPARC_t* structure, bool overwrite, const char* oldname, const char* newname);
    /**
     * @brief Duplicate an entry
     * 
     * @param structure the target structure
     * @param overwrite Overwrite an entry?
     * @param srcfile the source file to duplicate from.
     * @param destfile the destination file
     * @return MXPSQL_MPARC_err Yes.
     * 
     * @details
     * 
     * Internally implemented with MPARC_push_ufilestr and MPARC_peek_file
     */
    MXPSQL_MPARC_err MPARC_duplicate_file(MXPSQL_MPARC_t* structure, bool overwrite, const char* srcfile, const char* destfile);
    /**
     * @brief Swap 2 entries
     * 
     * @param structure the target structure.
     * @param file1 the filename to swap with file2. Also a swap victim
     * @param file2 the filename to swap with file1. Also a swap victim
     * @return MXPSQL_MPARC_err Lazy.
     * 
     * @details
     * 
     * Internally implemnted with MPARC_push_ufilestr and MPARC_peek_file
     */
    MXPSQL_MPARC_err MPARC_swap_file(MXPSQL_MPARC_t* structure, const char* file1, const char* file2);

    /**
     * @brief Pop a file off the archive
     * 
     * @param structure the target structure
     * @param filename the filename to pop off
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_pop_file(MXPSQL_MPARC_t* structure, const char* filename);
    /**
     * @brief Wipe everything (I mean every single file) off the archive
     * 
     * @param structure the target structure
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_clear(MXPSQL_MPARC_t* structure);

    /**
     * @brief Peek the contents of a file of the archive
     * 
     * @param structure the target structure
     * @param filename the filename to peek the contents at
     * @param bout the output pointer to a variable that represent the binary content of the file
     * @param sout the output pointer to a variable that represent the size of bout
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_peek_file(MXPSQL_MPARC_t* structure, const char* filename, unsigned char** bout, MXPSQL_MPARC_uint_repr_t* sout);

    /**
     * @brief Construct the archive into a string
     * 
     * @param structure the target structure
     * @param output the storage string
     * @return MXPSQL_MPARC_err the status code if successfully done
     * 
     * @note manually free output string with 'MPARC_free', not 'free' or 'delete' for forward compatibility.
     */
    MXPSQL_MPARC_err MPARC_construct_str(MXPSQL_MPARC_t* structure, char** output);
    /**
     * @brief Construct the archive into a file
     * 
     * @param structure the target structure
     * @param filename the filename target
     * @return MXPSQL_MPARC_err the status code if successfully done
     */
    MXPSQL_MPARC_err MPARC_construct_filename(MXPSQL_MPARC_t* structure, const char* filename);
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
     * @param on_item_ctx context value that is passed to on_item
     * @param mk_dir_ctx context value that is passed to mk_dir
     * @return MXPSQL_MPARC_err error status, some code are special, see details
     *
     * @details
     *
     * if the error code returns MPARC_OPPART, check dir2make to see if it needs you to make a new directory.
     *
     *
     * Example of the mk_dir function:  
     *      int mkdirer(char* dir, void* ctx){
     *          ((void)ctx);
     *          #if (defined(_WIN32) || defined(_WIN64)) && !(defined(__CYGWIN__))
     *          return !CreateDirectoryA(dir, NULL);
     *          #else
     *          return mkdir(dir, 0777);
     *          #endif
     *      }
     */
    MXPSQL_MPARC_err MPARC_extract_advance(MXPSQL_MPARC_t* structure, const char* destdir, char** dir2make, void (*on_item)(const char*, void*), int (*mk_dir)(char*, void*), void* on_item_ctx, void* mk_dir_ctx);
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
    MXPSQL_MPARC_err MPARC_extract(MXPSQL_MPARC_t* structure, const char* destdir, char** dir2make);

    /**
     * @brief Read a directory into the structure
     * 
     * @param structure the target structure
     * @param srcdir the source directory to read from
     * @param recursive read from subdirectories if given true
     * @param listdir function to list a directory, shall not be NULL or else it returns an error. This is the function that handles listing the file and recursion.
     * @param listdir_ctx context value that is passed to the void* parameter of listdir
     * @return MXPSQL_MPARC_err error status of reading
     * 
     * @details
     * 
     * > listdir Prototyping
     * 
     * the first parameter of the listdir function is the current directory that should be read from
     * 
     * the second parameter indicates if it should be recursive, its a boolean
     * 
     * the third parameter is what files it has found, should be an array of string, terminated with NULL and Calloc'ed or Malloc'ed (pls Calloc it) (Also please use the MPARC allocation functions instead of the standard libc ones) as it relies on finding NULL and the array getting freed
     * 
     * the fourth parameter is context, it's user defined, is set by putting a value in the listdir_ctx parameter
     * 
     * the return value should always be 0 for success, other values indicate failure
     * 
     * 
     * Example for listdir (POSIX only for now, also to be continued):
     *      int list_me_dir_recur(const char* path, bool recursive, char** output, void* ctx, size_t block_size){
     *          ((void)ctx);
     *          DIR* dir;
     *          struct dirent* entry;
     *          
     *          if(!(dir = opendir(path))) return 1;
     *          
     *          while((entry = readdir(dir))){
     *              if(recursive){
     *                  struct stat stet;
     *                  if(stat(entry->d_name, &statbuf) != 0) return 1;
     *                  if(S_ISDIR(stet.st_mode)){
     *                      // TBA
     *                  }
     *              }
     *          }
     *          
     *          closedir(dir);
     * 
     *          return 0;
     *      }
     *      
     *      int list_me_dir(const char* path, bool recursive, char*** output, void* ctx){
     *          static size_t bloc_size = 5;
     *          char** buff = MPARC_calloc(5, sizeof(char*));
     *          *output
     *          return list_me_dir_recur(path, recursive, buff, ctx);
     *      }
     */
    MXPSQL_MPARC_err MPARC_readdir(MXPSQL_MPARC_t* structure, const char* srcdir, bool recursive, int (*listdir)(const char*, bool, char***, void*), void* listdir_ctx);

    /**
     * @brief Parse the archive into the structure with extra flags
     * 
     * @param structure the target structure
     * @param stringy the string to be parsed to
     * @param erronduplicate error with returning MPARC_KEXISTS if the key exists
     * @param sensitive should it be standalone or can it be a polyglot? Allows steganography and Unicode with BOM parsing. Disclaimer: May possibly fail with a primitive lookup system (strrchr and strstr).
     * @return MXPSQL_MPARC_err Did it parse well or did not
     */
    MXPSQL_MPARC_err MPARC_parse_str_advance(MXPSQL_MPARC_t* structure, const char* stringy, bool erronduplicate, bool sensitive);
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
    MXPSQL_MPARC_err MPARC_parse_str(MXPSQL_MPARC_t* structure, const char* stringy);
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
    MXPSQL_MPARC_err MPARC_parse_filename(MXPSQL_MPARC_t* structure, const char* filename);


    // Memory management utilities for easy refactoring
    /**
     * @brief My malloc, for easy plug and switch. Extensions should use this instead of malloc.
     * 
     * @param size size of bytes to allocate
     * @return void* allocated memory pointer
     * 
     * @note
     * MPARC_malloc has its pitfalls. Don't fall for it.  
     * Right now is using C's malloc, so it is at the mercy of that function as malloc has flaws.  
     * Allocating a zero sized object with MPARC_Malloc, danger is from that it is implementation defined.
     */
    void* MPARC_malloc(MXPSQL_MPARC_uint_repr_t size);
    /**
     * @brief My calloc, for easy plug and switch. Extensions should use this instead of calloc.
     * 
     * @param arr_size size of array (if applicable, else put 1)
     * @param el_size size of element
     * @return void* allocated memory pointer
     * 
     * @note
     * MPARC_calloc has its pitfalls. Don't fall for it.  
     * Right now is using C's calloc, so it is at the mercy of that function as calloc has flaws.  
     * Allocating a zero sized object with MPARC_calloc, danger is from that it is implementation defined.
     */
    void* MPARC_calloc(MXPSQL_MPARC_uint_repr_t arr_size, size_t el_size);
    /**
     * @brief My realloc, for easy plug and switch. Extensions should use this instead of realloc.
     * 
     * @param oldmem old memory to change size
     * @param newsize new size
     * @return void* reallocated memory pointer
     * 
     * @note
     * 
     * MPARC_realloc has its pitfalls. Don't fall for it.  
     * Right now is using C's realloc, so it is at the mercy of that function as realloc has flaws.  
     * Common one:  
     * In place realloc, danger is from when it errors out and now you created a memory leak.  
     * Reusing old realloc pointers, danger is that it may have been invalidated.  
     * Allocating a zero sized object with MPARC_realloc, danger is that it used to be implementation defined before C23, after C23 is now undefined behaviour.
     * 
     */
    void* MPARC_realloc(void* oldmem, MXPSQL_MPARC_uint_repr_t newsize);
    /**
     * @brief My free, for easy plug and switch. Extensions should use this instead of free.
     * 
     * @param mem memory to free
     * 
     * @note
     * 
     * MPARC_free has its pitfalls. Don't fall for it.  
     * Right now is using C's free, so it is at the mercy of that function as free has flaws.  
     * Common one:  
     * Double free, danger is that it is undefined behaviour to do so.  
     * Free not allocated memory, danger is that it is also undefined behaviour to do so.  
     * Not freeing memory if you are done with it, danger is that you played yourself with memory leaks.
     */
    void MPARC_free(void* mem);


    // Type internal utilities
    /**
     * @brief Get sizeof MXPSQL_MPARC_t
     * 
     * @return size_t sizeof(MXPSQL_MPARC_t)
     * 
     * @see MXPSQL_MPARC_t
     */
    size_t MPARC_MXPSQL_MPARC_t_sizeof(void);
    /**
     * @brief Get sizeof MXPSQL_MPARC_iter_t
     * 
     * @return size_t sizeof(MXPSQL_MPARC_iter_t)
     * 
     * @see MXPSQL_MPARC_iter_t
     */
    size_t MPARC_MXPSQL_MPARC_iter_t_sizeof(void);

    // Auxiliary function
    #ifdef MPARC_WANT_EXTERN_AUX_UTIL_FUNCTIONS
    /**
     * @brief My strlen for portability
     * 
     * @param str string to check
     * @param maxlen max length
     * @return size_t actual str length or maxlen
     */
    size_t MPARC_strnlen(const char* str, size_t maxlen);
    /**
     * @brief Strtok Safe Edition
     * 
     * @param s string to be tokenized, must be mutable
     * @param delim delimiter to be used while tokenizing
     * @param save_ptr storage space for the rest of the 'she' string
     * @return char* The token
     */
    char* MPARC_strtok_r (char *s, const char *delim, char **save_ptr);
    /**
     * @brief Copy memory
     * 
     * @param src source memory bytes, interpreted as unsigned char*
     * @param len length of src
     * @return void* duplicated value, interpreted as unsigned char*
     * 
     * @note Use MPARC_free to deallocate instead of 'free' and 'delete' for forward compatibility.
     * 
     * @see MPARC_memdup
     */
    void* MPARC_memdup(const void* src, size_t len);
    /**
     * @brief Copy string with length limits
     * 
     * @param src source string
     * @param ilen max length to copy
     * @return char* duplicated string
     * 
     * @note Use 'MPARC_free' to deallocate instead of 'free' and 'delete' for forward compatibility.
     * 
     * @see MPARC_strndup
     */
    char* MPARC_strndup(const char* src, size_t ilen);
    /**
     * @brief Copy string with no length limits
     * 
     * @param src source string
     * @return char* duplicated string
     * 
     * @note Use 'MPARC_free' to deallocate instead of 'free' and 'delete' for forward compatibility.
     * 
     * @see MPARC_strdup
     */
    char* MPARC_strdup(const char* src);
    /**
     * @brief Get the basename of the file paht
     * 
     * @param filename file path
     * @return char* Base name of filename
     * 
     * @details
     * 
     * On unix, it is strrchr'd on '/' only.
     * 
     * On windows, it is not only strrchr'd on '/', it is also strchrr'd on '\\' (How you say the \ character on C)
     */
    char* MPARC_basename (const char *filename);
    /**
     * @brief Get directory
     * 
     * @param path file path
     * @return char* the directory path without the filename
     */
    char* MPARC_dirname (char *path);
    /**
     * @brief Get the extension. Dodgy implementation
     * 
     * @param fnp file path with extension
     * @param full_or_not if set to 0, strip from the last '.', else strip from the first '.'
     * @return char* the extension
     */
    char* MPARC_get_extension(const char* fnp, int full_or_not);
    #endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // end of header
