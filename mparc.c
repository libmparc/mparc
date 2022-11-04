/**
  * @file mparc.c
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. C Source File.
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

#ifndef _MXPSQL_MPARC_C
#define _MXPSQL_MPARC_C

#include "mparc.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#ifdef MPARC_DEBUG
#define MPARC_MEM_DEBUG 1
#endif

#ifdef MPARC_MEM_DEBUG
#define GC_DEBUG
#include <gc/gc.h>
#define malloc(n) GC_MALLOC(n)
#define calloc(m,n) GC_MALLOC((m)*(n))
#define free(p) GC_FREE(p)
#define realloc(p,n) GC_REALLOC((p),(n))
#define CHECK_LEAKS() GC_gcollect()
#else
#define CHECK_LEAKS()
#endif

/* defines */

#define STANKY_MPAR_FILE_FORMAT_MAGIC_NUMBER_25 "MXPSQL's Portable Archive"

// define for format version number and representation
#define STANKY_MPAR_FILE_FORMAT_VERSION_NUMBER 1
#define STANKY_MPAR_FILE_FORMAT_VERSION_HASH_ADDED 1
// #define STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION uint_fast64_t
#define STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION unsigned long long

// special separators, only added here if necessary
#define MPARC_MAGIC_CHKSM_SEP '%'

// sorting mode
// if set to 0, then the entries will be sorted by their checksum values
// else if set to 1 sorted by their filename
// else randomly sorted
// default is 1
// that description for mode 0 was actually false (sort of), normally the checksum will sort itself, but if it fails, then the json will sort it
#ifndef MPARC_QSORT_MODE
#define MPARC_QSORT_MODE 1
#endif

// Fwrite and Fread usage threshold
// how much bytes is needed to warant using fwrite or fread, else use fputc or fgetc
// default is 8 kiloytes
// think of as in bytes, not bits
#define MPARC_DIRECTF_MINIMUM (8000)

/* not defines */


#ifndef MXPSQL_MPARC_NO_B64

/*
 * B64 Section and Copyright notice
 *
 * Unlicensed :)
 * C++ implementation was ripped from tomykaira's gist 
 * https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
 *
 * written by skullchap 
 * https://github.com/skullchap/b64
*/

static char *b64Encode(unsigned char *data, uint_fast64_t inlen)
{
		static const char b64e[] = {
				'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
				'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
				'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
				'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
				'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
				'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
				'w', 'x', 'y', 'z', '0', '1', '2', '3',
				'4', '5', '6', '7', '8', '9', '+', '/'};

		uint_fast64_t outlen = ((((inlen) + 2) / 3) * 4);

		char *out = malloc(outlen + 1);
		if (out == NULL) return NULL;
		out[outlen] = '\0';
		char *p = out;

		uint_fast64_t i;
		for (i = 0; i < inlen - 2; i += 3)
		{
				*p++ = b64e[(data[i] >> 2) & 0x3F];
				*p++ = b64e[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
				*p++ = b64e[((data[i + 1] & 0xF) << 2) | ((data[i + 2] & 0xC0) >> 6)];
				*p++ = b64e[data[i + 2] & 0x3F];
		}

		if (i < inlen)
		{
				*p++ = b64e[(data[i] >> 2) & 0x3F];
				if (i == (inlen - 1))
				{
						*p++ = b64e[((data[i] & 0x3) << 4)];
						*p++ = '=';
				}
				else
				{
						*p++ = b64e[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
						*p++ = b64e[((data[i + 1] & 0xF) << 2)];
				}
				*p++ = '=';
		}

		return out;
}

static unsigned char *b64Decode(char *data, uint_fast64_t inlen, uint_fast64_t* outplen)
{
		static const char b64d[] = {
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
				64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
				15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
				64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
				41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

		if (inlen == 0 || inlen % 4) return NULL;
		uint_fast64_t outlen = (((inlen) / 4) * 3);

		if (data[inlen - 1] == '=') outlen--;
		if (data[inlen - 2] == '=') outlen--;

		unsigned char *out = (unsigned char*) malloc(outlen);
		if (out == NULL) return NULL;
		*outplen = outlen;

		typedef size_t u32;
		for (uint_fast64_t i = 0, j = 0; i < inlen;)
		{
				u32 a = data[i] == '=' ? 0 & i++ : (uint_fast64_t) b64d[((uint_fast64_t) data[(uint_fast64_t) i++])];
				u32 b = data[i] == '=' ? 0 & i++ : (uint_fast64_t) b64d[((uint_fast64_t) data[(uint_fast64_t) i++])];
				u32 c = data[i] == '=' ? 0 & i++ : (uint_fast64_t) b64d[((uint_fast64_t) data[(uint_fast64_t) i++])];
				u32 d = data[i] == '=' ? 0 & i++ : (uint_fast64_t) b64d[((uint_fast64_t) data[(uint_fast64_t) i++])];

				u32 triple = (a << 3 * 6) + (b << 2 * 6) +
													(c << 1 * 6) + (d << 0 * 6);

				if (j < outlen) out[j++] = (triple >> 2 * 8) & 0xFF;
				if (j < outlen) out[j++] = (triple >> 1 * 8) & 0xFF;
				if (j < outlen) out[j++] = (triple >> 0 * 8) & 0xFF;
		}

		return out;
}



static const struct {
		char* (*btoa) (unsigned char*, uint_fast64_t);
		unsigned char* (*atob) (char*, uint_fast64_t, uint_fast64_t*);
} b64 = {b64Encode, b64Decode};




/* END OF B64 SECTION */

#endif


#ifndef MXPSQL_MPARC_NO_PYCRC32

/* crc.h and crc.c by pycrc section */

/**
 * The type of the CRC values.
 *
 * This type must be big enough to contain at least 32 bits.
 */
typedef uint_fast32_t crc_t;


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
static crc_t crc_update(crc_t crc, const void *data, uint_fast64_t data_len)
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
 * \return     The final crc value.
 */
static inline crc_t crc_finalize(crc_t crc)
{
	return crc ^ 0xffffffff;
}

/* end of CRC32 section */

#endif


#ifndef MXPSQL_MPARC_NO_CCAN_JSON

/* jsmn.h and json.c section and copyright notice */

/* json.c section */

/*
  Copyright (C) 2011 Joseph A. Adams (joeyadams3.14159@gmail.com)
  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

typedef enum {
	JSON_NULL,
	JSON_BOOL,
	JSON_STRING,
	JSON_NUMBER,
	JSON_ARRAY,
	JSON_OBJECT,
} JsonTag;

typedef struct JsonNode JsonNode;

struct JsonNode
{
	/* only if parent is an object or array (NULL otherwise) */
	JsonNode *parent;
	JsonNode *prev, *next;
	
	/* only if parent is an object (NULL otherwise) */
	char *key; /* Must be valid UTF-8. */
	
	JsonTag tag;
	union {
		/* JSON_BOOL */
		bool boole;
		
		/* JSON_STRING */
		char *string; /* Must be valid UTF-8. */
		
		/* JSON_NUMBER */
		double number;
		
		/* JSON_ARRAY */
		/* JSON_OBJECT */
		struct {
			JsonNode *head, *tail;
		} children;
	} store;
};
	

#define out_of_memory() do {                    \
		/* dumb */ \
		/* fprintf(stderr, "Out of memory.\n"); */   \
		/* exit(EXIT_FAILURE);    */                 \
		; /* Null statement */ \
	} while (0)

/* Sadly, strdup is not portable. */
static char *json_strdup(const char *str)
{
	char *ret = (char*) malloc(strlen(str) + 1);
	if (ret == NULL){
		out_of_memory();
		return NULL;
	}
	strcpy(ret, str);
	return ret;
}

/* String buffer */

typedef struct
{
	char *cur;
	char *end;
	char *start;
} SB;

static void sb_init(SB *sb)
{
	sb->start = (char*) malloc(17);
	if (sb->start == NULL){
		out_of_memory();
		return;
	}
	sb->cur = sb->start;
	sb->end = sb->start + 16;
}

/* sb and need may be evaluated multiple times. */
#define sb_need(sb, need) do {                  \
		if ((sb)->end - (sb)->cur < (need))     \
			sb_grow(sb, need);                  \
		if(sb == NULL) break;   \
	} while (0)

static void sb_grow(SB *sb, int need)
{
	size_t length = sb->cur - sb->start;
	size_t alloc = sb->end - sb->start;
	
	do {
		alloc *= 2;
	} while (alloc < length + need);
	
	void* newsb = realloc(sb->start, alloc + 1);
	if (newsb == NULL){
		out_of_memory();
		return;
	}
	sb->start = (char*) newsb;
	sb->cur = sb->start + length;
	sb->end = sb->start + alloc;
}

static void sb_put(SB *sb, const char *bytes, int count)
{
	sb_need(sb, count);
	memcpy(sb->cur, bytes, count);
	sb->cur += count;
}

#define sb_putc(sb, c) do {         \
		if ((sb)->cur >= (sb)->end) \
			sb_grow(sb, 1);         \
		if(sb == NULL) break;   \
		*(sb)->cur++ = (c);         \
	} while (0)

static void sb_puts(SB *sb, const char *str)
{
	sb_put(sb, str, strlen(str));
}

static char *sb_finish(SB *sb)
{
	*sb->cur = 0;
	assert(sb->start <= sb->cur && strlen(sb->start) == (size_t)(sb->cur - sb->start));
	return sb->start;
}

static void sb_free(SB *sb)
{
	if(sb && sb->start) free(sb->start);
}

/*
 * Unicode helper functions
 *
 * These are taken from the ccan/charset module and customized a bit.
 * Putting them here means the compiler can (choose to) inline them,
 * and it keeps ccan/json from having a dependency.
 */

/*
 * Type for Unicode codepoints.
 * We need our own because wchar_t might be 16 bits.
 */
typedef uint32_t uchar_t;

/*
 * Validate a single UTF-8 character starting at @s.
 * The string must be null-terminated.
 *
 * If it's valid, return its length (1 thru 4).
 * If it's invalid or clipped, return 0.
 *
 * This function implements the syntax given in RFC3629, which is
 * the same as that given in The Unicode Standard, Version 6.0.
 *
 * It has the following properties:
 *
 *  * All codepoints U+0000..U+10FFFF may be encoded,
 *    except for U+D800..U+DFFF, which are reserved
 *    for UTF-16 surrogate pair encoding.
 *  * UTF-8 byte sequences longer than 4 bytes are not permitted,
 *    as they exceed the range of Unicode.
 *  * The sixty-six Unicode "non-characters" are permitted
 *    (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF).
 */
static int utf8_validate_cz(const char *s)
{
	unsigned char c = *s++;
	
	if (c <= 0x7F) {        /* 00..7F */
		return 1;
	} else if (c <= 0xC1) { /* 80..C1 */
		/* Disallow overlong 2-byte sequence. */
		return 0;
	} else if (c <= 0xDF) { /* C2..DF */
		/* Make sure subsequent byte is in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 2;
	} else if (c <= 0xEF) { /* E0..EF */
		/* Disallow overlong 3-byte sequence. */
		if (c == 0xE0 && (unsigned char)*s < 0xA0)
			return 0;
		
		/* Disallow U+D800..U+DFFF. */
		if (c == 0xED && (unsigned char)*s > 0x9F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 3;
	} else if (c <= 0xF4) { /* F0..F4 */
		/* Disallow overlong 4-byte sequence. */
		if (c == 0xF0 && (unsigned char)*s < 0x90)
			return 0;
		
		/* Disallow codepoints beyond U+10FFFF. */
		if (c == 0xF4 && (unsigned char)*s > 0x8F)
			return 0;
		
		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		
		return 4;
	} else {                /* F5..FF */
		return 0;
	}
}

/* Validate a null-terminated UTF-8 string. */
static bool utf8_validate(const char *s)
{
	int len;
	
	for (; *s != 0; s += len) {
		len = utf8_validate_cz(s);
		if (len == 0)
			return false;
	}
	
	return true;
}

/*
 * Read a single UTF-8 character starting at @s,
 * returning the length, in bytes, of the character read.
 *
 * This function assumes input is valid UTF-8,
 * and that there are enough characters in front of @s.
 */
static int utf8_read_char(const char *s, uchar_t *out)
{
	const unsigned char *c = (const unsigned char*) s;
	
	assert(utf8_validate_cz(s));

	if (c[0] <= 0x7F) {
		/* 00..7F */
		*out = c[0];
		return 1;
	} else if (c[0] <= 0xDF) {
		/* C2..DF (unless input is invalid) */
		*out = ((uchar_t)c[0] & 0x1F) << 6 |
			   ((uchar_t)c[1] & 0x3F);
		return 2;
	} else if (c[0] <= 0xEF) {
		/* E0..EF */
		*out = ((uchar_t)c[0] &  0xF) << 12 |
			   ((uchar_t)c[1] & 0x3F) << 6  |
			   ((uchar_t)c[2] & 0x3F);
		return 3;
	} else {
		/* F0..F4 (unless input is invalid) */
		*out = ((uchar_t)c[0] &  0x7) << 18 |
			   ((uchar_t)c[1] & 0x3F) << 12 |
			   ((uchar_t)c[2] & 0x3F) << 6  |
			   ((uchar_t)c[3] & 0x3F);
		return 4;
	}
}

/*
 * Write a single UTF-8 character to @s,
 * returning the length, in bytes, of the character written.
 *
 * @unicode must be U+0000..U+10FFFF, but not U+D800..U+DFFF.
 *
 * This function will write up to 4 bytes to @out.
 */
static int utf8_write_char(uchar_t unicode, char *out)
{
	unsigned char *o = (unsigned char*) out;
	
	assert(unicode <= 0x10FFFF && !(unicode >= 0xD800 && unicode <= 0xDFFF));

	if (unicode <= 0x7F) {
		/* U+0000..U+007F */
		*o++ = unicode;
		return 1;
	} else if (unicode <= 0x7FF) {
		/* U+0080..U+07FF */
		*o++ = 0xC0 | unicode >> 6;
		*o++ = 0x80 | (unicode & 0x3F);
		return 2;
	} else if (unicode <= 0xFFFF) {
		/* U+0800..U+FFFF */
		*o++ = 0xE0 | unicode >> 12;
		*o++ = 0x80 | (unicode >> 6 & 0x3F);
		*o++ = 0x80 | (unicode & 0x3F);
		return 3;
	} else {
		/* U+10000..U+10FFFF */
		*o++ = 0xF0 | unicode >> 18;
		*o++ = 0x80 | (unicode >> 12 & 0x3F);
		*o++ = 0x80 | (unicode >> 6 & 0x3F);
		*o++ = 0x80 | (unicode & 0x3F);
		return 4;
	}
}

/*
 * Compute the Unicode codepoint of a UTF-16 surrogate pair.
 *
 * @uc should be 0xD800..0xDBFF, and @lc should be 0xDC00..0xDFFF.
 * If they aren't, this function returns false.
 */
static bool from_surrogate_pair(uint16_t uc, uint16_t lc, uchar_t *unicode)
{
	if (uc >= 0xD800 && uc <= 0xDBFF && lc >= 0xDC00 && lc <= 0xDFFF) {
		*unicode = 0x10000 + ((((uchar_t)uc & 0x3FF) << 10) | (lc & 0x3FF));
		return true;
	} else {
		return false;
	}
}

/*
 * Construct a UTF-16 surrogate pair given a Unicode codepoint.
 *
 * @unicode must be U+10000..U+10FFFF.
 */
static void to_surrogate_pair(uchar_t unicode, uint16_t *uc, uint16_t *lc)
{
	uchar_t n;
	
	assert(unicode >= 0x10000 && unicode <= 0x10FFFF);
	
	n = unicode - 0x10000;
	*uc = ((n >> 10) & 0x3FF) | 0xD800;
	*lc = (n & 0x3FF) | 0xDC00;
}

#define is_space(c) ((c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == ' ')
#define is_digit(c) ((c) >= '0' && (c) <= '9')


static JsonNode   *json_decode         (const char *json);
static char       *json_encode         (const JsonNode *node);
static char       *json_encode_string  (const char *str);
static char       *json_stringify      (const JsonNode *node, const char *space);
static void        json_delete         (JsonNode *node);
static bool        json_validate       (const char *json);

static bool parse_value     (const char **sp, JsonNode        **out);
static bool parse_string    (const char **sp, char            **out);
static bool parse_number    (const char **sp, double           *out);
static bool parse_array     (const char **sp, JsonNode        **out);
static bool parse_object    (const char **sp, JsonNode        **out);
static bool parse_hex16     (const char **sp, uint16_t         *out);

static bool expect_literal  (const char **sp, const char *str);
static void skip_space      (const char **sp);

static void emit_value              (SB *out, const JsonNode *node);
static void emit_value_indented     (SB *out, const JsonNode *node, const char *space, int indent_level);
static void emit_string             (SB *out, const char *str);
static void emit_number             (SB *out, double num);
static void emit_array              (SB *out, const JsonNode *array);
static void emit_array_indented     (SB *out, const JsonNode *array, const char *space, int indent_level);
static void emit_object             (SB *out, const JsonNode *object);
static void emit_object_indented    (SB *out, const JsonNode *object, const char *space, int indent_level);

static int write_hex16(char *out, uint16_t val);

/*** Lookup and traversal ***/

static JsonNode   *json_find_element   (JsonNode *array, int index);
static JsonNode   *json_find_member    (JsonNode *object, const char *name);

static JsonNode   *json_first_child    (const JsonNode *node);

/*** Construction and manipulation ***/

static JsonNode *json_mknull(void);
static JsonNode *json_mkbool(bool b);
static JsonNode *json_mkstring(const char *s);
static JsonNode *json_mknumber(double n);
static JsonNode *json_mkarray(void);
static JsonNode *json_mkobject(void);
static void json_append_element(JsonNode *array, JsonNode *element);
static void json_prepend_element(JsonNode *array, JsonNode *element);
static void json_append_member(JsonNode *object, const char *key, JsonNode *value);
static void json_prepend_member(JsonNode *object, const char *key, JsonNode *value);
static void json_remove_from_parent(JsonNode *node);

static JsonNode *mknode(JsonTag tag);
static void append_node(JsonNode *parent, JsonNode *child);
static void prepend_node(JsonNode *parent, JsonNode *child);
static void append_member(JsonNode *object, char *key, JsonNode *value);

/* Assertion-friendly validity checks */
static bool tag_is_valid(unsigned int tag);
static bool number_is_valid(const char *num);

/*** Debugging ***/

/*
 * Look for structure and encoding problems in a JsonNode or its descendents.
 *
 * If a problem is detected, return false, writing a description of the problem
 * to errmsg (unless errmsg is NULL).
 */
static bool json_check(const JsonNode *node, char errmsg[256]);


#define json_foreach(i, object_or_array)            \
	for ((i) = json_first_child(object_or_array);   \
		 (i) != NULL;                               \
		 (i) = (i)->next)


static JsonNode *json_decode(const char *json)
{
	const char *s = json;
	JsonNode *ret;
	
	skip_space(&s);
	if (!parse_value(&s, &ret))
		return NULL;
	
	skip_space(&s);
	if (*s != 0) {
		json_delete(ret);
		return NULL;
	}
	
	return ret;
}

static char *json_encode(const JsonNode *node)
{
	return json_stringify(node, NULL);
}

static char *json_encode_string(const char *str)
{
	SB sb;
	sb_init(&sb);
	if(sb.start == NULL) return NULL;
	
	emit_string(&sb, str);
	
	return sb_finish(&sb);
}

static char *json_stringify(const JsonNode *node, const char *space)
{
	SB sb;
	sb_init(&sb);
	if(sb.start == NULL) return NULL;
	
	if (space != NULL)
		emit_value_indented(&sb, node, space, 0);
	else
		emit_value(&sb, node);
	
	return sb_finish(&sb);
}

static void json_delete(JsonNode *node)
{
	if (node != NULL) {
		json_remove_from_parent(node);
		
		switch (node->tag) {
			case JSON_STRING:
				if(node->store.string) free(node->store.string);
				break;
			case JSON_ARRAY:
			case JSON_OBJECT:
			{
				JsonNode *child, *next;
				for (child = node->store.children.head; child != NULL; child = next) {
					next = child->next;
					json_delete(child);
				}
				break;
			}
			default:;
		}
		
		if(node) free(node);
		node=NULL;
	}
}

static bool json_validate(const char *json)
{
	const char *s = json;
	
	skip_space(&s);
	if (!parse_value(&s, NULL))
		return false;
	
	skip_space(&s);
	if (*s != 0)
		return false;
	
	return true;
}

static JsonNode *json_find_element(JsonNode *array, int index)
{
	JsonNode *element;
	int i = 0;
	
	if (array == NULL || array->tag != JSON_ARRAY)
		return NULL;
	
	json_foreach(element, array) {
		if (i == index)
			return element;
		i++;
	}
	
	return NULL;
}

static JsonNode *json_find_member(JsonNode *object, const char *name)
{
	JsonNode *member;
	
	if (object == NULL || object->tag != JSON_OBJECT)
		return NULL;
	
	json_foreach(member, object)
		if (strcmp(member->key, name) == 0)
			return member;
	
	return NULL;
}

static JsonNode *json_first_child(const JsonNode *node)
{
	if (node != NULL && (node->tag == JSON_ARRAY || node->tag == JSON_OBJECT))
		return node->store.children.head;
	return NULL;
}

static JsonNode *mknode(JsonTag tag)
{
	JsonNode *ret = (JsonNode*) calloc(1, sizeof(JsonNode));
	if (ret == NULL){
		out_of_memory();
		return NULL;
	}
	ret->tag = tag;
	return ret;
}

static JsonNode *json_mknull(void)
{
	return mknode(JSON_NULL);
}

static JsonNode *json_mkbool(bool b)
{
	JsonNode *ret = mknode(JSON_BOOL);
	if(ret == NULL) return NULL;
	ret->store.boole = b;
	return ret;
}

static JsonNode *mkstring(char *s)
{
	JsonNode *ret = mknode(JSON_STRING);
	if(ret == NULL) return NULL;
	ret->store.string = s;
	return ret;
}

static JsonNode *json_mkstring(const char *s)
{
	char* dup = json_strdup(s);
	if(dup == NULL) return NULL;
	else return mkstring(dup);
}

static JsonNode *json_mknumber(double n)
{
	JsonNode *node = mknode(JSON_NUMBER);
	if(node == NULL) return NULL;
	node->store.number = n;
	return node;
}

static JsonNode *json_mkarray(void)
{
	return mknode(JSON_ARRAY);
}

static JsonNode *json_mkobject(void)
{
	return mknode(JSON_OBJECT);
}

static void append_node(JsonNode *parent, JsonNode *child)
{
	child->parent = parent;
	child->prev = parent->store.children.tail;
	child->next = NULL;
	
	if (parent->store.children.tail != NULL)
		parent->store.children.tail->next = child;
	else
		parent->store.children.head = child;
	parent->store.children.tail = child;
}

static void prepend_node(JsonNode *parent, JsonNode *child)
{
	child->parent = parent;
	child->prev = NULL;
	child->next = parent->store.children.head;
	
	if (parent->store.children.head != NULL)
		parent->store.children.head->prev = child;
	else
		parent->store.children.tail = child;
	parent->store.children.head = child;
}

static void append_member(JsonNode *object, char *key, JsonNode *value)
{
	value->key = key;
	append_node(object, value);
}

static void json_append_element(JsonNode *array, JsonNode *element)
{
	assert(array->tag == JSON_ARRAY);
	assert(element->parent == NULL);
	
	append_node(array, element);
}

static void json_prepend_element(JsonNode *array, JsonNode *element)
{
	assert(array->tag == JSON_ARRAY);
	assert(element->parent == NULL);
	
	prepend_node(array, element);
}

static void json_append_member(JsonNode *object, const char *key, JsonNode *value)
{
	assert(object->tag == JSON_OBJECT);
	assert(value->parent == NULL);

	char* k = json_strdup(key);
	if(k == NULL) return;
	
	append_member(object, k, value);
}

static void json_prepend_member(JsonNode *object, const char *key, JsonNode *value)
{
	assert(object->tag == JSON_OBJECT);
	assert(value->parent == NULL);

	char* k = json_strdup(key);
	if(k == NULL) return;

	value->key = k;
	prepend_node(object, value);
}

static void json_remove_from_parent(JsonNode *node)
{
	JsonNode *parent = node->parent;
	
	if (parent != NULL) {
		if (node->prev != NULL)
			node->prev->next = node->next;
		else
			parent->store.children.head = node->next;
		if (node->next != NULL)
			node->next->prev = node->prev;
		else
			parent->store.children.tail = node->prev;
		
		if(node->key) free(node->key);
		
		node->parent = NULL;
		node->prev = node->next = NULL;
		node->key = NULL;
	}
}

static bool parse_value(const char **sp, JsonNode **out)
{
	const char *s = *sp;
	
	switch (*s) {
		case 'n':
			if (expect_literal(&s, "null")) {
				if (out)
					*out = json_mknull();
				*sp = s;
				return true;
			}
			return false;
		
		case 'f':
			if (expect_literal(&s, "false")) {
				if (out)
					*out = json_mkbool(false);
				*sp = s;
				return true;
			}
			return false;
		
		case 't':
			if (expect_literal(&s, "true")) {
				if (out)
					*out = json_mkbool(true);
				*sp = s;
				return true;
			}
			return false;
		
		case '"': {
			char *str;
			if (parse_string(&s, out ? &str : NULL)) {
				if (out)
					*out = mkstring(str);
				*sp = s;
				return true;
			}
			return false;
		}
		
		case '[':
			if (parse_array(&s, out)) {
				*sp = s;
				return true;
			}
			return false;
		
		case '{':
			if (parse_object(&s, out)) {
				*sp = s;
				return true;
			}
			return false;
		
		default: {
			double num;
			if (parse_number(&s, out ? &num : NULL)) {
				if (out)
					*out = json_mknumber(num);
				*sp = s;
				return true;
			}
			return false;
		}
	}
}

static bool parse_array(const char **sp, JsonNode **out)
{
	const char *s = *sp;
	JsonNode *ret = out ? json_mkarray() : NULL;
	JsonNode *element;
	
	if (*s++ != '[')
		goto failure;
	skip_space(&s);
	
	if (*s == ']') {
		s++;
		goto success;
	}
	
	for (;;) {
		if (!parse_value(&s, out ? &element : NULL))
			goto failure;
		skip_space(&s);
		
		if (out)
			json_append_element(ret, element);
		
		if (*s == ']') {
			s++;
			goto success;
		}
		
		if (*s++ != ',')
			goto failure;
		skip_space(&s);
	}
	
success:
	*sp = s;
	if (out)
		*out = ret;
	return true;

failure:
	json_delete(ret);
	return false;
}

static bool parse_object(const char **sp, JsonNode **out)
{
	const char *s = *sp;
	JsonNode *ret = out ? json_mkobject() : NULL;
	char *key;
	JsonNode *value;
	
	if (*s++ != '{')
		goto failure;
	skip_space(&s);
	
	if (*s == '}') {
		s++;
		goto success;
	}
	
	for (;;) {
		if (!parse_string(&s, out ? &key : NULL))
			goto failure;
		skip_space(&s);
		
		if (*s++ != ':')
			goto failure_free_key;
		skip_space(&s);
		
		if (!parse_value(&s, out ? &value : NULL))
			goto failure_free_key;
		skip_space(&s);
		
		if (out)
			append_member(ret, key, value);
		
		if (*s == '}') {
			s++;
			goto success;
		}
		
		if (*s++ != ',')
			goto failure;
		skip_space(&s);
	}
	
success:
	*sp = s;
	if (out)
		*out = ret;
	return true;

failure_free_key:
	if (out)
		if(key) free(key);
failure:
	json_delete(ret);
	return false;
}

static bool parse_string(const char **sp, char **out)
{
	const char *s = *sp;
	SB sb;
	char throwaway_buffer[4];
		/* enough space for a UTF-8 character */
	char *b;
	
	if (*s++ != '"')
		return false;
	
	if (out) {
		sb_init(&sb);
		if(sb.start == NULL) return false;
		sb_need(&sb, 4);
		b = sb.cur;
	} else {
		b = throwaway_buffer;
	}
	
	while (*s != '"') {
		unsigned char c = *s++;
		
		/* Parse next character, and write it to b. */
		if (c == '\\') {
			c = *s++;
			switch (c) {
				case '"':
				case '\\':
				case '/':
					*b++ = c;
					break;
				case 'b':
					*b++ = '\b';
					break;
				case 'f':
					*b++ = '\f';
					break;
				case 'n':
					*b++ = '\n';
					break;
				case 'r':
					*b++ = '\r';
					break;
				case 't':
					*b++ = '\t';
					break;
				case 'u':
				{
					uint16_t uc, lc;
					uchar_t unicode;
					
					if (!parse_hex16(&s, &uc))
						goto failed;
					
					if (uc >= 0xD800 && uc <= 0xDFFF) {
						/* Handle UTF-16 surrogate pair. */
						if (*s++ != '\\' || *s++ != 'u' || !parse_hex16(&s, &lc))
							goto failed; /* Incomplete surrogate pair. */
						if (!from_surrogate_pair(uc, lc, &unicode))
							goto failed; /* Invalid surrogate pair. */
					} else if (uc == 0) {
						/* Disallow "\u0000". */
						goto failed;
					} else {
						unicode = uc;
					}
					
					b += utf8_write_char(unicode, b);
					break;
				}
				default:
					/* Invalid escape */
					goto failed;
			}
		} else if (c <= 0x1F) {
			/* Control characters are not allowed in string literals. */
			goto failed;
		} else {
			/* Validate and echo a UTF-8 character. */
			int len;
			
			s--;
			len = utf8_validate_cz(s);
			if (len == 0)
				goto failed; /* Invalid UTF-8 character. */
			
			while (len--)
				*b++ = *s++;
		}
		
		/*
		 * Update sb to know about the new bytes,
		 * and set up b to write another character.
		 */
		if (out) {
			sb.cur = b;
			sb_need(&sb, 4);
			b = sb.cur;
		} else {
			b = throwaway_buffer;
		}
	}
	s++;
	
	if (out)
		*out = sb_finish(&sb);
	*sp = s;
	return true;

failed:
	if (out)
		sb_free(&sb);
	return false;
}

/*
 * The JSON spec says that a number shall follow this precise pattern
 * (spaces and quotes added for readability):
 *	 '-'? (0 | [1-9][0-9]*) ('.' [0-9]+)? ([Ee] [+-]? [0-9]+)?
 *
 * However, some JSON parsers are more liberal.  For instance, PHP accepts
 * '.5' and '1.'.  JSON.parse accepts '+3'.
 *
 * This function takes the strict approach.
 */
bool parse_number(const char **sp, double *out)
{
	const char *s = *sp;

	/* '-'? */
	if (*s == '-')
		s++;

	/* (0 | [1-9][0-9]*) */
	if (*s == '0') {
		s++;
	} else {
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	/* ('.' [0-9]+)? */
	if (*s == '.') {
		s++;
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	/* ([Ee] [+-]? [0-9]+)? */
	if (*s == 'E' || *s == 'e') {
		s++;
		if (*s == '+' || *s == '-')
			s++;
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	if (out)
		*out = strtod(*sp, NULL);

	*sp = s;
	return true;
}

static void skip_space(const char **sp)
{
	const char *s = *sp;
	while (is_space(*s))
		s++;
	*sp = s;
}

static void emit_value(SB *out, const JsonNode *node)
{
	assert(tag_is_valid(node->tag));
	switch (node->tag) {
		case JSON_NULL:
			sb_puts(out, "null");
			break;
		case JSON_BOOL:
			sb_puts(out, node->store.boole ? "true" : "false");
			break;
		case JSON_STRING:
			emit_string(out, node->store.string);
			break;
		case JSON_NUMBER:
			emit_number(out, node->store.number);
			break;
		case JSON_ARRAY:
			emit_array(out, node);
			break;
		case JSON_OBJECT:
			emit_object(out, node);
			break;
		default:
			assert(false);
	}
}

static void emit_value_indented(SB *out, const JsonNode *node, const char *space, int indent_level)
{
	assert(tag_is_valid(node->tag));
	switch (node->tag) {
		case JSON_NULL:
			sb_puts(out, "null");
			break;
		case JSON_BOOL:
			sb_puts(out, node->store.boole ? "true" : "false");
			break;
		case JSON_STRING:
			emit_string(out, node->store.string);
			break;
		case JSON_NUMBER:
			emit_number(out, node->store.number);
			break;
		case JSON_ARRAY:
			emit_array_indented(out, node, space, indent_level);
			break;
		case JSON_OBJECT:
			emit_object_indented(out, node, space, indent_level);
			break;
		default:
			assert(false);
	}
}

static void emit_array(SB *out, const JsonNode *array)
{
	const JsonNode *element;
	
	sb_putc(out, '[');
	json_foreach(element, array) {
		emit_value(out, element);
		if (element->next != NULL)
			sb_putc(out, ',');
	}
	sb_putc(out, ']');
}

static void emit_array_indented(SB *out, const JsonNode *array, const char *space, int indent_level)
{
	const JsonNode *element = array->store.children.head;
	int i;
	
	if (element == NULL) {
		sb_puts(out, "[]");
		return;
	}
	
	sb_puts(out, "[\n");
	while (element != NULL) {
		for (i = 0; i < indent_level + 1; i++)
			sb_puts(out, space);
		emit_value_indented(out, element, space, indent_level + 1);
		
		element = element->next;
		sb_puts(out, element != NULL ? ",\n" : "\n");
	}
	for (i = 0; i < indent_level; i++)
		sb_puts(out, space);
	sb_putc(out, ']');
}

static void emit_object(SB *out, const JsonNode *object)
{
	const JsonNode *member;
	
	sb_putc(out, '{');
	json_foreach(member, object) {
		emit_string(out, member->key);
		sb_putc(out, ':');
		emit_value(out, member);
		if (member->next != NULL)
			sb_putc(out, ',');
	}
	sb_putc(out, '}');
}

static void emit_object_indented(SB *out, const JsonNode *object, const char *space, int indent_level)
{
	const JsonNode *member = object->store.children.head;
	int i;
	
	if (member == NULL) {
		sb_puts(out, "{}");
		return;
	}
	
	sb_puts(out, "{\n");
	while (member != NULL) {
		for (i = 0; i < indent_level + 1; i++)
			sb_puts(out, space);
		emit_string(out, member->key);
		sb_puts(out, ": ");
		emit_value_indented(out, member, space, indent_level + 1);
		
		member = member->next;
		sb_puts(out, member != NULL ? ",\n" : "\n");
	}
	for (i = 0; i < indent_level; i++)
		sb_puts(out, space);
	sb_putc(out, '}');
}

static void emit_string(SB *out, const char *str)
{
	bool escape_unicode = false;
	const char *s = str;
	char *b;
	
	assert(utf8_validate(str));
	
	/*
	 * 14 bytes is enough space to write up to two
	 * \uXXXX escapes and two quotation marks.
	 */
	sb_need(out, 14);
	b = out->cur;
	
	*b++ = '"';
	while (*s != 0) {
		unsigned char c = *s++;
		
		/* Encode the next character, and write it to b. */
		switch (c) {
			case '"':
				*b++ = '\\';
				*b++ = '"';
				break;
			case '\\':
				*b++ = '\\';
				*b++ = '\\';
				break;
			case '\b':
				*b++ = '\\';
				*b++ = 'b';
				break;
			case '\f':
				*b++ = '\\';
				*b++ = 'f';
				break;
			case '\n':
				*b++ = '\\';
				*b++ = 'n';
				break;
			case '\r':
				*b++ = '\\';
				*b++ = 'r';
				break;
			case '\t':
				*b++ = '\\';
				*b++ = 't';
				break;
			default: {
				int len;
				
				s--;
				len = utf8_validate_cz(s);
				
				if (len == 0) {
					/*
					 * Handle invalid UTF-8 character gracefully in production
					 * by writing a replacement character (U+FFFD)
					 * and skipping a single byte.
					 *
					 * This should never happen when assertions are enabled
					 * due to the assertion at the beginning of this function.
					 */
					assert(false);
					if (escape_unicode) {
						strcpy(b, "\\uFFFD");
						b += 6;
					} else {
						*b++ = (unsigned char) 0xEF;
						*b++ = (unsigned char) 0xBF;
						*b++ = (unsigned char) 0xBD;
					}
					s++;
				} else if (c < 0x1F || (c >= 0x80 && escape_unicode)) {
					/* Encode using \u.... */
					uint32_t unicode;
					
					s += utf8_read_char(s, &unicode);
					
					if (unicode <= 0xFFFF) {
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, unicode);
					} else {
						/* Produce a surrogate pair. */
						uint16_t uc, lc;
						assert(unicode <= 0x10FFFF);
						to_surrogate_pair(unicode, &uc, &lc);
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, uc);
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, lc);
					}
				} else {
					/* Write the character directly. */
					while (len--)
						*b++ = *s++;
				}
				
				break;
			}
		}
	
		/*
		 * Update *out to know about the new bytes,
		 * and set up b to write another encoded character.
		 */
		out->cur = b;
		sb_need(out, 14);
		b = out->cur;
	}
	*b++ = '"';
	
	out->cur = b;
}

static void emit_number(SB *out, double num)
{
	/*
	 * This isn't exactly how JavaScript renders numbers,
	 * but it should produce valid JSON for reasonable numbers
	 * preserve precision well enough, and avoid some oddities
	 * like 0.3 -> 0.299999999999999988898 .
	 */
	char buf[64];
	sprintf(buf, "%.16g", num);
	
	if (number_is_valid(buf))
		sb_puts(out, buf);
	else
		sb_puts(out, "null");
}

static bool tag_is_valid(unsigned int tag)
{
	return (/* tag >= JSON_NULL && */ tag <= JSON_OBJECT);
}

static bool number_is_valid(const char *num)
{
	return (parse_number(&num, NULL) && *num == '\0');
}

static bool expect_literal(const char **sp, const char *str)
{
	const char *s = *sp;
	
	while (*str != '\0')
		if (*s++ != *str++)
			return false;
	
	*sp = s;
	return true;
}

/*
 * Parses exactly 4 hex characters (capital or lowercase).
 * Fails if any input chars are not [0-9A-Fa-f].
 */
static bool parse_hex16(const char **sp, uint16_t *out)
{
	const char *s = *sp;
	uint16_t ret = 0;
	uint16_t i;
	uint16_t tmp;
	char c;

	for (i = 0; i < 4; i++) {
		c = *s++;
		if (c >= '0' && c <= '9')
			tmp = c - '0';
		else if (c >= 'A' && c <= 'F')
			tmp = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			tmp = c - 'a' + 10;
		else
			return false;

		ret <<= 4;
		ret += tmp;
	}
	
	if (out)
		*out = ret;
	*sp = s;
	return true;
}

/*
 * Encodes a 16-bit number into hexadecimal,
 * writing exactly 4 hex chars.
 */
static int write_hex16(char *out, uint16_t val)
{
	const char *hex = "0123456789ABCDEF";
	
	*out++ = hex[(val >> 12) & 0xF];
	*out++ = hex[(val >> 8)  & 0xF];
	*out++ = hex[(val >> 4)  & 0xF];
	*out++ = hex[ val        & 0xF];
	
	return 4;
}

static bool json_check(const JsonNode *node, char errmsg[256])
{
	#define problem(...) do { \
			if (errmsg != NULL) \
				snprintf(errmsg, 256, __VA_ARGS__); \
			return false; \
		} while (0)
	
	if (node->key != NULL && !utf8_validate(node->key))
		problem("key contains invalid UTF-8");
	
	if (!tag_is_valid(node->tag))
		problem("tag is invalid (%u)", node->tag);
	
	if (node->tag == JSON_BOOL) {
		if (node->store.boole != false && node->store.boole != true)
			problem("bool_ is neither false (%d) nor true (%d)", (int)false, (int)true);
	} else if (node->tag == JSON_STRING) {
		if (node->store.string == NULL)
			problem("string_ is NULL");
		if (!utf8_validate(node->store.string))
			problem("string_ contains invalid UTF-8");
	} else if (node->tag == JSON_ARRAY || node->tag == JSON_OBJECT) {
		JsonNode *head = node->store.children.head;
		JsonNode *tail = node->store.children.tail;
		
		if (head == NULL || tail == NULL) {
			if (head != NULL)
				problem("tail is NULL, but head is not");
			if (tail != NULL)
				problem("head is NULL, but tail is not");
		} else {
			JsonNode *child;
			JsonNode *last = NULL;
			
			if (head->prev != NULL)
				problem("First child's prev pointer is not NULL");
			
			for (child = head; child != NULL; last = child, child = child->next) {
				if (child == node)
					problem("node is its own child");
				if (child->next == child)
					problem("child->next == child (cycle)");
				if (child->next == head)
					problem("child->next == head (cycle)");
				
				if (child->parent != node)
					problem("child does not point back to parent");
				if (child->next != NULL && child->next->prev != child)
					problem("child->next does not point back to child");
				
				if (node->tag == JSON_ARRAY && child->key != NULL)
					problem("Array element's key is not NULL");
				if (node->tag == JSON_OBJECT && child->key == NULL)
					problem("Object member's key is NULL");
				
				if (!json_check(child, errmsg))
					return false;
			}
			
			if (last != tail)
				problem("tail does not match pointer found by starting at head and following next links");
		}
	}
	
	return true;
	
	#undef problem
}

#endif


#ifndef MXPSQL_MPARC_NO_JSMN

/* jsmn.h part from https://github.com/zserge/jsmn */

/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
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
 */

#define JSMN_STATIC static



/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
	JSMN_UNDEFINED = 0,
	JSMN_OBJECT = 1 << 0,
	JSMN_ARRAY = 1 << 1,
	JSMN_STRING = 1 << 2,
	JSMN_PRIMITIVE = 1 << 3
} jsmntype_t;

enum jsmnerr {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct jsmntok {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct jsmn_parser {
	unsigned int pos;     /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper;         /* superior token node, e.g. parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
JSMN_STATIC void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing
 * a single JSON object.
 */
JSMN_STATIC int jsmn_parse(jsmn_parser *parser, const char *js, const size_t len,
												jsmntok_t *tokens, const unsigned int num_tokens);

/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens,
																	 const size_t num_tokens) {
	jsmntok_t *tok;
	if (parser->toknext >= num_tokens) {
		return NULL;
	}
	tok = &tokens[parser->toknext++];
	tok->start = tok->end = -1;
	tok->size = 0;
#ifdef JSMN_PARENT_LINKS
	tok->parent = -1;
#endif
	return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, const jsmntype_t type,
														const int start, const int end) {
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
																const size_t len, jsmntok_t *tokens,
																const size_t num_tokens) {
	jsmntok_t *token;
	int start;

	start = parser->pos;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		switch (js[parser->pos]) {
#ifndef JSMN_STRICT
		/* In strict mode primitive must be followed by "," or "}" or "]" */
		case ':':
#endif
		case '\t':
		case '\r':
		case '\n':
		case ' ':
		case ',':
		case ']':
		case '}':
			goto found;
		default:
									 /* to quiet a warning from gcc*/
			break;
		}
		if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
			parser->pos = start;
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	parser->pos = start;
	return JSMN_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->pos--;
		return 0;
	}
	token = jsmn_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->pos = start;
		return JSMN_ERROR_NOMEM;
	}
	jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
														 const size_t len, jsmntok_t *tokens,
														 const size_t num_tokens) {
	jsmntok_t *token;

	int start = parser->pos;
	
	/* Skip starting quote */
	parser->pos++;
	
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c = js[parser->pos];

		/* Quote: end of string */
		if (c == '\"') {
			if (tokens == NULL) {
				return 0;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				parser->pos = start;
				return JSMN_ERROR_NOMEM;
			}
			jsmn_fill_token(token, JSMN_STRING, start + 1, parser->pos);
#ifdef JSMN_PARENT_LINKS
			token->parent = parser->toksuper;
#endif
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && parser->pos + 1 < len) {
			int i;
			parser->pos++;
			switch (js[parser->pos]) {
			/* Allowed escaped symbols */
			case '\"':
			case '/':
			case '\\':
			case 'b':
			case 'f':
			case 'r':
			case 'n':
			case 't':
				break;
			/* Allows escaped symbol \uXXXX */
			case 'u':
				parser->pos++;
				for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0';
						 i++) {
					/* If it isn't a hex character we have an error */
					if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) ||   /* 0-9 */
								(js[parser->pos] >= 65 && js[parser->pos] <= 70) ||   /* A-F */
								(js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
						parser->pos = start;
						return JSMN_ERROR_INVAL;
					}
					parser->pos++;
				}
				parser->pos--;
				break;
			/* Unexpected symbol */
			default:
				parser->pos = start;
				return JSMN_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
JSMN_STATIC int jsmn_parse(jsmn_parser *parser, const char *js, const size_t len,
												jsmntok_t *tokens, const unsigned int num_tokens) {
	int r;
	int i;
	jsmntok_t *token;
	int count = parser->toknext;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		jsmntype_t type;

		c = js[parser->pos];
		switch (c) {
		case '{':
		case '[':
			count++;
			if (tokens == NULL) {
				break;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				return JSMN_ERROR_NOMEM;
			}
			if (parser->toksuper != -1) {
				jsmntok_t *t = &tokens[parser->toksuper];
#ifdef JSMN_STRICT
				/* In strict mode an object or array can't become a key */
				if (t->type == JSMN_OBJECT) {
					return JSMN_ERROR_INVAL;
				}
#endif
				t->size++;
#ifdef JSMN_PARENT_LINKS
				token->parent = parser->toksuper;
#endif
			}
			token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
			token->start = parser->pos;
			parser->toksuper = parser->toknext - 1;
			break;
		case '}':
		case ']':
			if (tokens == NULL) {
				break;
			}
			type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
			if (parser->toknext < 1) {
				return JSMN_ERROR_INVAL;
			}
			token = &tokens[parser->toknext - 1];
			for (;;) {
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JSMN_ERROR_INVAL;
					}
					token->end = parser->pos + 1;
					parser->toksuper = token->parent;
					break;
				}
				if (token->parent == -1) {
					if (token->type != type || parser->toksuper == -1) {
						return JSMN_ERROR_INVAL;
					}
					break;
				}
				token = &tokens[token->parent];
			}
#else
			for (i = parser->toknext - 1; i >= 0; i--) {
				token = &tokens[i];
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JSMN_ERROR_INVAL;
					}
					parser->toksuper = -1;
					token->end = parser->pos + 1;
					break;
				}
			}
			/* Error if unmatched closing bracket */
			if (i == -1) {
				return JSMN_ERROR_INVAL;
			}
			for (; i >= 0; i--) {
				token = &tokens[i];
				if (token->start != -1 && token->end == -1) {
					parser->toksuper = i;
					break;
				}
			}
#endif
			break;
		case '\"':
			r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
			if (r < 0) {
				return r;
			}
			count++;
			if (parser->toksuper != -1 && tokens != NULL) {
				tokens[parser->toksuper].size++;
			}
			break;
		case '\t':
		case '\r':
		case '\n':
		case ' ':
			break;
		case ':':
			parser->toksuper = parser->toknext - 1;
			break;
		case ',':
			if (tokens != NULL && parser->toksuper != -1 &&
					tokens[parser->toksuper].type != JSMN_ARRAY &&
					tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
				parser->toksuper = tokens[parser->toksuper].parent;
#else
				for (i = parser->toknext - 1; i >= 0; i--) {
					if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
						if (tokens[i].start != -1 && tokens[i].end == -1) {
							parser->toksuper = i;
							break;
						}
					}
				}
#endif
			}
			break;
#ifdef JSMN_STRICT
		/* In strict mode primitives are: numbers and booleans */
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 't':
		case 'f':
		case 'n':
			/* And they must not be keys of the object */
			if (tokens != NULL && parser->toksuper != -1) {
				const jsmntok_t *t = &tokens[parser->toksuper];
				if (t->type == JSMN_OBJECT ||
						(t->type == JSMN_STRING && t->size != 0)) {
					return JSMN_ERROR_INVAL;
				}
			}
#else
		/* In non-strict mode every unquoted value is a primitive */
		default:
#endif
			r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
			if (r < 0) {
				return r;
			}
			count++;
			if (parser->toksuper != -1 && tokens != NULL) {
				tokens[parser->toksuper].size++;
			}
			break;

#ifdef JSMN_STRICT
		/* Unexpected char in strict mode */
		default:
			return JSMN_ERROR_INVAL;
#endif
		}
	}

	if (tokens != NULL) {
		for (i = parser->toknext - 1; i >= 0; i--) {
			/* Unmatched opened object or array */
			if (tokens[i].start != -1 && tokens[i].end == -1) {
				return JSMN_ERROR_PART;
			}
		}
	}

	return count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
JSMN_STATIC void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}

#endif

/* END OF jsmn.h and json.h section */


#ifndef MXPSQL_MPARC_NO_RXI_MAP

/* map.h by RXI section and copyright notice 
 * 
 * Sourced from https://github.com/rxi/map
 * 
 * Copyright (c) 2014 rxi
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
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
*/

struct map_node_t;
typedef struct map_node_t map_node_t;

typedef struct {
	map_node_t **buckets;
	unsigned nbuckets, nnodes;
} map_base_t;

typedef struct {
	unsigned bucketidx;
	map_node_t *node;
} map_iter_t;


#define map_t(T)\
	struct { map_base_t base; T *ref; T tmp; }


#define map_init(m)\
	memset(m, 0, sizeof(*(m)))


#define map_deinit(m)\
	map_deinit_(&(m)->base)


#define map_get(m, key)\
	( (m)->ref = map_get_(&(m)->base, key) )


#define map_set(m, key, value)\
	( (m)->tmp = (value),\
		map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)) )


#define map_remove(m, key)\
	map_remove_(&(m)->base, key)


#define map_iter(m)\
	map_iter_()


#define map_next(m, iter)\
	map_next_(&(m)->base, iter)


static void map_deinit_(map_base_t *m);
static void *map_get_(map_base_t *m, const char *key);
static int map_set_(map_base_t *m, const char *key, void *value, int vsize);
static void map_remove_(map_base_t *m, const char *key);
static map_iter_t map_iter_(void);
static const char *map_next_(map_base_t *m, map_iter_t *iter);


typedef map_t(void*) map_void_t;
typedef map_t(char*) map_str_t;
typedef map_t(int) map_int_t;
typedef map_t(char) map_char_t;
typedef map_t(float) map_float_t;
typedef map_t(double) map_double_t;

struct map_node_t {
	unsigned hash;
	void *value;
	map_node_t *next;
	/* char key[]; */
	/* char value[]; */
};


static unsigned map_hash(const char *str) {
	unsigned hash = 5381;
	while (*str) {
		hash = ((hash << 5) + hash) ^ *str++;
	}
	return hash;
}


static map_node_t *map_newnode(const char *key, void *value, int vsize) {
	map_node_t *node;
	int ksize = strlen(key) + 1;
	int voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*));
	node = malloc(sizeof(*node) + voffset + vsize);
	if (!node) return NULL;
	memcpy(node + 1, key, ksize);
	node->hash = map_hash(key);
	node->value = ((char*) (node + 1)) + voffset;
	memcpy(node->value, value, vsize);
	return node;
}


static int map_bucketidx(map_base_t *m, unsigned hash) {
	/* If the implementation is changed to allow a non-power-of-2 bucket count,
	 * the line below should be changed to use mod instead of AND */
	return hash & (m->nbuckets - 1);
}


static void map_addnode(map_base_t *m, map_node_t *node) {
	int n = map_bucketidx(m, node->hash);
	node->next = m->buckets[n];
	m->buckets[n] = node;
}


static int map_resize(map_base_t *m, int nbuckets) {
	map_node_t *nodes, *node, *next;
	map_node_t **buckets;
	int i; 
	/* Chain all nodes together */
	nodes = NULL;
	i = m->nbuckets;
	while (i--) {
		node = (m->buckets)[i];
		while (node) {
			next = node->next;
			node->next = nodes;
			nodes = node;
			node = next;
		}
	}
	/* Reset buckets */
	buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
	if (buckets != NULL) {
		m->buckets = buckets;
		m->nbuckets = nbuckets;
	}
	if (m->buckets) {
		memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
		/* Re-add nodes to buckets */
		node = nodes;
		while (node) {
			next = node->next;
			map_addnode(m, node);
			node = next;
		}
	}
	/* Return error code if realloc() failed */
	return (buckets == NULL) ? -1 : 0;
}


static map_node_t **map_getref(map_base_t *m, const char *key) {
	unsigned hash = map_hash(key);
	map_node_t **next;
	if (m->nbuckets > 0) {
		next = &m->buckets[map_bucketidx(m, hash)];
		while (*next) {
			if ((*next)->hash == hash && !strcmp((char*) (*next + 1), key)) {
				return next;
			}
			next = &(*next)->next;
		}
	}
	return NULL;
}


static void map_deinit_(map_base_t *m) {
	map_node_t *next, *node;
	int i;
	i = m->nbuckets;
	while (i--) {
		node = m->buckets[i];
		while (node) {
			next = node->next;
			if(node) free(node);
			node = next;
		}
	}
	if(m && m->buckets) free(m->buckets);
}


static void *map_get_(map_base_t *m, const char *key) {
	map_node_t **next = map_getref(m, key);
	return next ? (*next)->value : NULL;
}


static int map_set_(map_base_t *m, const char *key, void *value, int vsize) {
	int n, err;
	map_node_t **next, *node;
	/* Find & replace existing node */
	next = map_getref(m, key);
	if (next) {
		memcpy((*next)->value, value, vsize);
		return 0;
	}
	/* Add new node */
	node = map_newnode(key, value, vsize);
	if (node == NULL) goto fail;
	if (m->nnodes >= m->nbuckets) {
		n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;
		err = map_resize(m, n);
		if (err) goto fail;
	}
	map_addnode(m, node);
	m->nnodes++;
	return 0;
	fail:
	if (node) free(node);
	return -1;
}


static void map_remove_(map_base_t *m, const char *key) {
	map_node_t *node;
	map_node_t **next = map_getref(m, key);
	if (next) {
		node = *next;
		*next = (*next)->next;
		if(node) free(node);
		m->nnodes--;
	}
}


static map_iter_t map_iter_(void) {
	map_iter_t iter;
	iter.bucketidx = -1;
	iter.node = NULL;
	return iter;
}


static const char *map_next_(map_base_t *m, map_iter_t *iter) {
	if (iter->node) {
		iter->node = iter->node->next;
		if (iter->node == NULL) goto nextBucket;
	} else {
		nextBucket:
		do {
			if (++iter->bucketidx >= m->nbuckets) {
				return NULL;
			}
			iter->node = m->buckets[iter->bucketidx];
		} while (iter->node == NULL);
	}
	return (char*) (iter->node + 1);
}

#endif

/* end of map.h section */

/* Linked List from https://gist.github.com/meylingtaing/11018042 and modified for memory safety, no output randomly and make it an actual list instead of a stack (it makes a great stack instead of a linked list :P) */

/* struct node {
		void *data;
		struct node *next;
};

typedef struct node * llist;

llist *llist_create(void *new_data)
{
		struct node *new_node;

		llist *new_list = (llist *)malloc(sizeof (llist));
		if(new_list == NULL){
				return NULL;
		}
		*new_list = (struct node *)malloc(sizeof (struct node));
		if(new_list == NULL){
				return NULL;
		}
		
		new_node = *new_list;
		new_node->data = new_data;
		new_node->next = NULL;
		return new_list;
}

void llist_free(llist *list)
{
		struct node *curr = *list;
		struct node *next;

		while (curr != NULL) {
				next = curr->next;
				free(curr);
				curr = next;
		}

		free(list);
}

// Returns 0 on failure
int llist_add_inorder(void *data, llist *list,
											 int (*comp)(void *, void *))
{
		struct node *new_node;
		struct node *curr;
		struct node *prev = NULL;
		
		if (list == NULL || *list == NULL) {
				// fprintf(stderr, "llist_add_inorder: list is null\n");
				return 0;
		}
		
		curr = *list;
		if (curr->data == NULL) {
				curr->data = data;
				return 1;
		}

		new_node = (struct node *)malloc(sizeof (struct node));
		new_node->data = data;

		// Find spot in linked list to insert new node
		while (curr != NULL && curr->data != NULL && comp(curr->data, data) < 0) {
				prev = curr;
				curr = curr->next;
		}
		new_node->next = curr;

		if (prev == NULL) 
				*list = new_node;
		else 
				prev->next = new_node;

		return 1;
}

// returns 0 on failure
int llist_push(llist *list, void *data)
{
		struct node *head;
		struct node *new_node;
		if (list == NULL || *list == NULL) {
				// fprintf(stderr, "llist_add_inorder: list is null\n");
				return 0;
		}

		head = *list;
		
		// Head is empty node
		if (head->data == NULL){
				head->data = data;
				return 1;
		}

		// Head is not empty, add new node to front
		else {
				new_node = malloc(sizeof (struct node));
				if(new_node == NULL){
						return 0;
				}
				new_node->data = data;
				new_node->next = head;
				*list = new_node;
				return 1;
		}
}

void *llist_peek(llist* list){
		struct node *head = *list;
		if(list == NULL || head->data == NULL)
				return NULL;

		return head->data;
}

void *llist_pop(llist *list)
{
		void *popped_data;
		struct node *head = *list;

		if (list == NULL || head->data == NULL)
				return NULL;
		
		popped_data = llist_peek(list);
		*list = head->next;

		free(head);

		return popped_data;
} */

/* end of llist.h and llist.c section */

/* start of LZ77 Section: FastL7 */
/* https://github.com/ariya/FastLZ source */
/* 
 * Have a bowl of this MIT License
 * 
 * FastLZ - Byte-aligned LZ77 compression library
 * Copyright (C) 2005-2020 Ariya Hidayat <ariya.hidayat@gmail.com>
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

// Nah, but I can still put it in here though

/* end of LZ77 Section: FastL7 */

/* TINY SNIPPETS THAT WILL BE VERY USEFUL LATER ON OK SECTION */


/* Glibc Strtok, Here's Your LGPL Notice and Why I Dual Licensed it under the LGPL and MIT

		Source: https://codebrowser.dev/glibc/glibc/string/strtok_r.c.html

		Reentrant string tokenizer.  Generic version.
		Copyright (C) 1991-2022 Free Software Foundation, Inc.
		This file is part of the GNU C Library.
		The GNU C Library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2.1 of the License, or (at your option) any later version.
		The GNU C Library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
		Lesser General Public License for more details.
		You should have received a copy of the GNU Lesser General Public
		License along with the GNU C Library; if not, see
		<https://www.gnu.org/licenses/>.  */
char * MPARC_strtok_r (char *s, const char *delim, char **save_ptr)
{
	char *end;
	if (s == NULL)
		s = *save_ptr;
	if (*s == '\0')
		{
			*save_ptr = s;
			return NULL;
		}
	/* Scan leading delimiters.  */
	s += strspn (s, delim);
	if (*s == '\0')
		{
			*save_ptr = s;
			return NULL;
		}
	/* Find the end of the token.  */
	end = s + strcspn (s, delim);
	if (*end == '\0')
		{
			*save_ptr = end;
			return s;
		}
	/* Terminate the token and make *SAVE_PTR point past it.  */
	*end = '\0';
	*save_ptr = end + 1;
	return s;
}

char* const_strdup(const char* src){
		char *str;
		char *p;
		int len = 0;

		while (src[len])
				len++;
		str = malloc(len + 1);
		p = str;
		while (*src)
				*p++ = *src++;
		*p = '\0';
		return str;
}

// from glibc from https://github.com/lattera/glibc/blob/master/string/basename.c
char* MPARC_basename (const char *filename)
{
  char *p = strrchr (filename, '/');
  return p ? p + 1 : (char *) filename;
}

// from glibc from https://github.com/lattera/glibc/blob/master/misc/dirname.c
char* MPARC_dirname (char *path)
{
  static const char dot[] = ".";
  char *last_slash;

  /* Find last '/'.  */
  last_slash = path != NULL ? strrchr (path, '/') : NULL;

  if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
	{
	  /* Determine whether all remaining characters are slashes.  */
	  char *runp;

	  for (runp = last_slash; runp != path; --runp){
		if (runp[-1] != '/'){
		  break;
		}
	  }

	  /* The '/' is the last character, we have to look further.  */
	  if (runp != path)
		last_slash = memchr (path, '/', runp - path);
	}

  if (last_slash != NULL)
	{
	  /* Determine whether all remaining characters are slashes.  */
	  char *runp;

	  for (runp = last_slash; runp != path; --runp){
		if (runp[-1] != '/'){
		  break;
		}
	  }

	  /* Terminate the path.  */
	  if (runp == path)
		{
		  /* The last slash is the first character in the string.  We have to
			 return "/".  As a special case we have to return "//" if there
			 are exactly two slashes at the beginning of the string.  See
			 XBD 4.10 Path Name Resolution for more information.  */
		  if (last_slash == path + 1){
			++last_slash;
		  }
		  else{
			last_slash = path + 1;
		  }
		}
		else{
			last_slash = runp;
		}

	  last_slash[0] = '\0';
	}
  else{
	/* This assignment is ill-designed but the XPG specs require to
	   return a string containing "." in any case no directory part is
	   found and so a static and constant string is required.  */
	path = (char *) dot;
  }

  return path;
}

// bsearch and strcoll
static int voidstrcmp(const void* str1, const void* str2){
	return strcoll((const char*) str1, (const char*) str2);
}

// future, may not be used
static int isLittleEndian(){
	volatile uint32_t i=0x01234567;
	// return 0 for big endian, 1 for little endian.
	return (*((uint8_t*)(&i))) == 0x67;
}

/* END OF SNIPPETS */

/* BEGINNING OF MY SECTION OK */



		/* MY LINKED LIST IMPL OK, UNUSED OK, NOW DONT SPEAK WE DONT SPEAKE ABOUT llist OOK */

		struct _llist_node {
				char* val;
				struct _llist_node* next;
		};

		typedef struct _llist {
				struct _llist_node* top;
		} _llist;
		

		/* MAIN CODE OK */
		typedef struct MPARC_blob_store{
				uint_fast64_t binary_size;
				unsigned char* binary_blob;
				crc_t binary_crc;
		} MPARC_blob_store;

		typedef map_t(MPARC_blob_store) map_blob_t;

		struct MXPSQL_MPARC_t {
				/* separator markers */
				char magic_byte_sep; // separate the 25 character long magic number from the rest of the archive
				char meta_sep; // separate the version number from implementation specific metadata
				char entry_sep_or_general_marker; // a new line, but usually used to separate entries
				char comment_marker; // unused, but can be implemented, other implementations can use this as extension metadata
				char begin_entry_marker; // indicate beginning of entries
				char entry_elem2_sep_marker_or_magic_sep_marker; // separate checksum from entry contents
				char end_entry_marker; // indicate end of entry
				char end_file_marker; // indicate end of file

				/* metadata */
				STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION writerVersion; // version that is used when creating archive
				STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION loadedVersion; // version that indicates what version of the archive is loaded

				/* storage actually */
				map_blob_t globby;

				
				/* hidden */
				va_list vlist;
		};

		struct MXPSQL_MPARC_iter_t {
			MXPSQL_MPARC_t* archive;
			map_iter_t itery;
		};

		int MPARC_strerror(MXPSQL_MPARC_err err, char** out){
			// ((void)out);
			((void)isLittleEndian);
			switch(err){
				case MPARC_OK:
				*out = const_strdup("it fine, no error");
				break;

				case MPARC_IDK:
				*out = const_strdup("it fine cause idk, unknown error");
				return 1;
				case MPARC_INTERNAL:
				*out = const_strdup("Internal error detected");
				return 1;


				case MPARC_IVAL:
				*out = const_strdup("Bad vals or just generic but less generic than MPARC_IDK");
				break;

				case MPARC_KNOEXIST:
				*out = const_strdup("It does not exist you dumb dumb, basically that key does not exist");
				break;
				case MPARC_KEXISTS:
				*out = const_strdup("Key exists");
				break;

				case MPARC_OOM:
				*out = const_strdup("Oh noes I run out of memory, you need to deal with your ram cause there is a problem with your memory due to encountering out of memory condition");
				break;

				case MPARC_NOTARCHIVE:
				*out = const_strdup("You dumb person what you put in is not an archive by the 25 character long magic number it has or maybe we find out it is not a valid archive by the json or anything else");
				break;
				case MPARC_ARCHIVETOOSHINY:
				*out = const_strdup("You dumb person the valid archive you put in me is too new for me to process, this archive processor may be version 1, but the archive is version 2");
				break;
				case MPARC_CHKSUM:
				*out = const_strdup("My content is gone or I can't write my content properly because it failed the CRC32 test :P");
				break;

				case MPARC_CONSTRUCT_FAIL:
				*out = const_strdup("Failed to construct archive.");
				break;

				case MPARC_OPPART:
				*out = const_strdup("Operation was not complete, continue the operation after giving it the remedy it needs.");
				break;

				case MPARC_FERROR:
				*out = const_strdup("FILE.exe has stopped responding as there is a problem with the FILE IO operation");
				break;

				default:
				*out = const_strdup("Man what happened here that was not a valid code");
				return 1;
			}
			return 0;
		}

		int MPARC_sfperror(MXPSQL_MPARC_err err, FILE* filepstream, char* emsg){
			char* s = "";
			// printf("%d", err);
			int r = MPARC_strerror(err, &s);
			fprintf(filepstream, "%s%s\n", emsg, s);
			// free(s);
			return r;
		}

		int MPARC_fperror(MXPSQL_MPARC_err err, FILE* fileptrstream){
			return MPARC_sfperror(err, fileptrstream, "");
		}

		int MPARC_perror(MXPSQL_MPARC_err err){
			return MPARC_fperror(err, stderr);
		}


		static MXPSQL_MPARC_err MPARC_i_push_ufilestr_advancea(MXPSQL_MPARC_t* structure, char* filename, int stripdir, int overwrite, unsigned char* ustringc, uint_fast64_t sizy, crc_t crc3){
			MPARC_blob_store blob = {
				sizy,
				ustringc,
				crc3
			};

			char* pfilename = NULL;
			if(stripdir != 0){
				pfilename = MPARC_basename(pfilename);
			}
			else{
				pfilename = filename;
			}

			if(overwrite == 0 && MPARC_exists(structure, pfilename) != MPARC_KNOEXIST){
				return MPARC_KEXISTS;
			}

			if(map_set(&structure->globby, pfilename, blob) != 0){
				return MPARC_IVAL;
			}

			return MPARC_OK;
		}

		static int MPARC_i_sortcmp(const void* p1, const void* p2){
			const char* str1 = *((const char**) p1);
			const char* str2 = *((const char**) p2);

			int spress = 0;

			switch(MPARC_QSORT_MODE){
				case 0: {
					// dumb sort
					// most likely checksum will do the work of sorting it
					spress = voidstrcmp(str1, str2);
					break;
				}

				case 1: {
					// smart sort
					char* str1d = NULL;
					char* str2d = NULL;

					{
						str1d = const_strdup(str1);
						if(str1d == NULL) goto me_my_errhandler;
						str2d = const_strdup(str2);
						if(str2d == NULL) goto me_my_errhandler;
					}

					char* sav = NULL;
					char sep[2] = {MPARC_MAGIC_CHKSM_SEP, '\0'};

					char* str1fnam = NULL;
					char* str2fnam = NULL;
					{
						char* tok = MPARC_strtok_r(str1d, sep, &sav);
						if(tok == NULL || strcmp(sav, "") == 0 || sav == NULL){
							goto me_my_errhandler;
						}

						{
							// ease, use json.c
							JsonNode* root = json_decode(sav);
							JsonNode* node = NULL;
							json_foreach(node, root) {
								// ignore non string ones
								if(node->tag == JSON_STRING){
									if(strcmp(node->key, "filename") == 0){
										str1fnam = node->store.string;
									}
								}
							};
						}
					}
					{
						char* tok = MPARC_strtok_r(str2d, sep, &sav);
						if(tok == NULL || strcmp(sav, "") == 0 || sav == NULL){
							goto me_my_errhandler;
						}

						{
							// ease, use json.c
							JsonNode* root = json_decode(sav);
							JsonNode* node = NULL;
							json_foreach(node, root) {
								// ignore non string ones
								if(node->tag == JSON_STRING){
									if(strcmp(node->key, "filename") == 0){
										str2fnam = node->store.string;
									}
								}
							};
						}
					}

					spress = voidstrcmp(str1fnam, str2fnam);

					goto me_my_errhandler;

					me_my_errhandler:
					{
						if(str1d) free(str1d);
						if(str2d) free(str2d);
					}

					break;
				}

				default: {
					double difftimet = difftime(time(NULL), 0);
					srand((unsigned int) difftimet);
					spress = (rand() % 30) - 15;
					break;
				}
			};

			if(spress > 0) return 1;
			else if(spress < 0) return -1;
			else if(spress == 0) return 0;
			else return 0;
		}
		
		static MXPSQL_MPARC_err MPARC_i_sort(char** sortedstr){
			if(sortedstr == NULL) return MPARC_IVAL;

			uint_fast64_t counter = 0;
			{
				uint_fast64_t i = 0;
				for(i = 0; 
					sortedstr[i] != NULL
					; i++);
				counter = i;
			}
			{
				qsort(sortedstr, counter, sizeof(*sortedstr), MPARC_i_sortcmp);
				qsort(sortedstr, counter, sizeof(*sortedstr), MPARC_i_sortcmp);
			}
			/* {
				uint_fast64_t i = 0, j = 0;
				char* temp = NULL;
				for (i=0;i<counter-1;i++)
				{
					for (j=0;j<counter-i-1;j++)
					{
						if (MPARC_i_sortcmp(sortedstr[j], sortedstr[j + 1] ) > 0)  // more readable with indexing syntax
						{
							temp = sortedstr[j];
							sortedstr[j] = sortedstr[j+1];
							sortedstr[j+1] = temp;
						}
					}
				}
			} */
			return MPARC_OK;
		}

		static char* MPARC_i_construct_header(MXPSQL_MPARC_t* structure){
			JsonNode* nod = json_mkobject();
			char* s = json_encode(nod);

			{
				static char* fmt = STANKY_MPAR_FILE_FORMAT_MAGIC_NUMBER_25"%c%llu%c%s%c";
				int sps = snprintf(NULL, 0, fmt, structure->magic_byte_sep, structure->writerVersion, structure->meta_sep, s, structure->begin_entry_marker);
				if(sps < 0){
						return NULL;
				}
				char* alloc = calloc(sps+1, sizeof(char));
				if(alloc == NULL) return NULL;
				if(snprintf(alloc, sps+1, fmt, structure->magic_byte_sep, structure->writerVersion, structure->meta_sep, s, structure->begin_entry_marker) < 0){
						if(alloc) free(alloc);
						return NULL;
				}
				else{
						return alloc;
				}
			}
		}

		static char* MPARC_i_construct_entries(MXPSQL_MPARC_t* structure, MXPSQL_MPARC_err* eout){
				char* estring = NULL;

				char** jsonry = NULL;
				uint_fast64_t jsonentries;

				if(MPARC_list_array(structure, NULL, &jsonentries) != MPARC_OK){
					if(eout) *eout = MPARC_KNOEXIST;
					return NULL;
				}

				jsonry = calloc(jsonentries+1, sizeof(char*));
				jsonry[jsonentries]=NULL;

				const char* nkey;
				MXPSQL_MPARC_iter_t* itery = NULL;
				// map_iter_t itery = map_iter(&structure->globby);
				uint_fast64_t indexy = 0;

				{
					MXPSQL_MPARC_err err = MPARC_list_iterator_init(&structure, &itery);
					if(err != MPARC_OK){
						if(eout) *eout = MPARC_KNOEXIST;
						MPARC_list_iterator_destroy(&itery);
						goto errhandler;
					}
				}

				while((MPARC_list_iterator_next(&itery, &nkey)) == MPARC_OK){
						MPARC_blob_store* bob_the_blob_raw = map_get(&structure->globby, nkey);
						if(!bob_the_blob_raw){
							continue;
						}
						crc_t crc3 = crc_init();
						JsonNode* objectweb = json_mkobject();
						MPARC_blob_store bob_the_blob = *bob_the_blob_raw;
						char* btob = b64.btoa(bob_the_blob.binary_blob, bob_the_blob.binary_size);
						crc3 = bob_the_blob.binary_crc;
						if(btob == NULL) {
							if(eout) *eout = MPARC_OOM;
							MPARC_list_iterator_destroy(&itery);
							goto errhandler;
						}
						JsonNode* glob64 = json_mkstring(btob);
						JsonNode* filename = json_mkstring(nkey);
						JsonNode* blob_chksum = NULL;
						if(glob64 == NULL || filename == NULL) {
							if(eout) *eout = MPARC_OOM;
							MPARC_list_iterator_destroy(&itery);
							goto errhandler;
						}

						{
							static char* fmter = "%"PRIuFAST32;
							char* globsum = NULL;
							int size = snprintf(NULL, 0, fmter, crc3);
							if(size < 0){
								if(eout) *eout = MPARC_CONSTRUCT_FAIL;
								if(jsonry) free(jsonry);
								goto errhandler;
							}
							globsum = calloc(size+1, sizeof(char));
							if(globsum == NULL){
								if(eout) *eout = MPARC_OOM;
								MPARC_list_iterator_destroy(&itery);
								goto errhandler;
							}
							if(snprintf(globsum, size, fmter, crc3) < 0){
								if(eout) *eout = MPARC_CONSTRUCT_FAIL;
								goto errhandler;
							}
							blob_chksum = json_mkstring(globsum);
							if(blob_chksum == NULL){
								if(eout) *eout = MPARC_OOM;
								MPARC_list_iterator_destroy(&itery);
								goto errhandler;
							}
							if(globsum) free(globsum);
						}
						json_append_member(objectweb, "filename", filename);
						json_append_member(objectweb, "blob", glob64);
						json_append_member(objectweb, "crcsum", blob_chksum);
						if(!json_check(objectweb, NULL)){
							if(eout) *eout = MPARC_KNOEXIST;
							MPARC_list_iterator_destroy(&itery);
							goto errhandler;
						}
						char* stringy = json_encode(objectweb);


						{
							char* crcStringy;
							crc_t crc = crc_init();
							crc = crc_update(crc, stringy, strlen(stringy));
							crc = crc_finalize(crc);
							{
								static char* fmt = "%"PRIuFAST32"%c%s";

								int sp = snprintf(NULL, 0, fmt, crc, structure->entry_elem2_sep_marker_or_magic_sep_marker, stringy)+10; // silly hack workaround, somehow snprintf is kind of broken in this part
								crcStringy = calloc((sp+1),sizeof(char));
								if(crcStringy == NULL){
									if(eout) *eout = MPARC_OOM;
									MPARC_list_iterator_destroy(&itery);
									goto errhandler;
								}
								if(snprintf(crcStringy, sp, fmt, crc, structure->entry_elem2_sep_marker_or_magic_sep_marker, stringy) < 0){
									if(eout) *eout = MPARC_CONSTRUCT_FAIL;
									if(crcStringy) free(crcStringy);
									MPARC_list_iterator_destroy(&itery);
									goto errhandler;
								}
								jsonry[indexy] = crcStringy;
							}
						}

						indexy++;
				}

				MPARC_list_iterator_destroy(&itery);

				// ((void)MPARC_i_sort);
				MPARC_i_sort(jsonry); // We qsort this, qsort can be quicksort, insertion sort or BOGOSORT. It is ordered by the checksum instead of the name lmao.

				{
						uint_fast64_t iacrurate_snprintf_len = 1;
						for(uint_fast64_t i = 0; i < jsonentries; i++){
								iacrurate_snprintf_len += strlen(jsonry[i])+10;
						}

						char* str = calloc(iacrurate_snprintf_len+1, sizeof(char));
						if(str == NULL) {
							if(eout) *eout = MPARC_OOM;
							goto errhandler;
						}
						memset(str, '\0', iacrurate_snprintf_len+1);
						for(uint_fast64_t i2 = 0; i2 < jsonentries; i2++){
								char* outstr = calloc(iacrurate_snprintf_len+1, sizeof(char));
								if(outstr == NULL){
									if(eout) *eout = MPARC_OOM;
									if(str) free(str);
									goto errhandler;
								}
								strcpy(outstr, str);
								int len = snprintf(outstr, iacrurate_snprintf_len, "%s%c%s", str, structure->entry_sep_or_general_marker, jsonry[i2]);
								if(len < 0 || ((uint_fast64_t)len) > iacrurate_snprintf_len){
									if(eout) *eout = MPARC_CONSTRUCT_FAIL;
									if(outstr) free(outstr);
									goto errhandler;
								}
								strcpy(str, outstr);
						}

						estring = str;
				}

				goto commonexit;


				errhandler:
				estring = NULL;

				goto commonexit;

				commonexit:
				{
					for(uint_fast64_t i = 0; i < jsonentries; i++){
							if(jsonry[i]) free(jsonry[i]);
					}
					if(jsonry) free(jsonry);
				}

				return estring;
		}

		static char* MPARC_i_construct_ender(MXPSQL_MPARC_t* structure){
				static char* format = "%c%c";
				char* charachorder = NULL;
				int charachorder_len = 0;
				charachorder_len = snprintf(NULL, 0, format, structure->end_entry_marker, structure->end_file_marker);
				if(charachorder_len < 0){
						return NULL;
				}
				charachorder = calloc(charachorder_len+1, sizeof(char));
				if(charachorder == NULL){
						return NULL;
				}
				if(snprintf(charachorder, charachorder_len+1, format, structure->end_entry_marker, structure->end_file_marker) < 0){
						if(charachorder) free(charachorder);
						return NULL;
				}
				return charachorder;
		}


		static STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION MPARC_i_parse_version(char* str, int* success){
			STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION lversion = 0;
			/*{
				if(sscanf(str, "%"SCNuFAST64, &lversion) != 1){
					if(success != NULL) *success = 0;
					return 0;
				}
				if(success != NULL) *success = 1;
			}*/
			{
				char* endptr = NULL;
				errno = 0;
				lversion = strtoull(str, &endptr, 2);
				if(!(errno == 0 && str && (!*endptr || *endptr != 0 || *endptr != '\0'))){
					if(success != NULL) success = 0;
					return 0;
				}
				if(success != NULL) *success = 1;
			}
			return lversion;
		}

		static MXPSQL_MPARC_err MPARC_i_parse_header(MXPSQL_MPARC_t* structure, char* Stringy){
			STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION version = structure->writerVersion;

			// not bad implementation
			// this is not broken
			// but not compliant with the note I written
			/* {
				char sep[2] = {structure->begin_entry_marker, '\0'};
				char* saveptr;
				char* btok = MPARC_strtok_r(Stringy, sep, &saveptr);
				if(btok == NULL || strcmp(saveptr, "") == 0){
					return MPARC_NOTARCHIVE;
				}

				{
					char* saveptr2;
					char sep2[2] = {structure->magic_byte_sep, '\0'};
					char* tok = MPARC_strtok_r(btok, sep2, &saveptr2);
					if(tok == NULL || strcmp(STANKY_MPAR_FILE_FORMAT_MAGIC_NUMBER_25, btok) != 0 || strcmp(saveptr2, "") == 0){
						return MPARC_NOTARCHIVE;
					}
					tok = MPARC_strtok_r(NULL, sep2, &saveptr2);
					if(tok == NULL){
						return MPARC_NOTARCHIVE;
					}
					{
						char* nstok;
						char* jstok;
						{
							char* saveptr3 = NULL;
							char sep3[2] = {structure->meta_sep, '\0'};
							nstok = MPARC_strtok_r(tok, sep3, &saveptr3);
							if(nstok == NULL || strcmp(saveptr3, "") == 0){
								return MPARC_NOTARCHIVE;
							}
							jstok = saveptr3;
						}
						{
							STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION lversion = 0;
							{
								int status = 0;
								lversion = MPARC_i_parse_version(nstok, &status);
								if(status != 1){
									return MPARC_NOTARCHIVE;
								}
							}
							if(lversion > version){
								errno = ERANGE;
								return MPARC_ARCHIVETOOSHINY;
							}
							structure->loadedVersion = lversion;
						}
						{
							((void)jstok);
						}
					}
				}
			} */

			// construction note tip compliant method
			// also cleaner
			{
				char magic_sep[] = {structure->magic_byte_sep, '\0'};
				char* magic_saveptr;
				char* magic_tok = MPARC_strtok_r(Stringy, magic_sep, &magic_saveptr);
				if(magic_tok == NULL || strcmp(magic_saveptr, "") == 0){
					return MPARC_NOTARCHIVE;
				}

				{
					char pre_entry_begin_sep[2] = {structure->begin_entry_marker, '\0'};
					char* pre_entry_begin_saveptr;
					char* peb_tok = MPARC_strtok_r(magic_saveptr, pre_entry_begin_sep, &pre_entry_begin_saveptr); // peb means pre entry begin
					if(peb_tok == NULL || strcmp(peb_tok, "") == 0){
						return MPARC_NOTARCHIVE;
					}


					{
						char meta_sep_sep[2] = {structure->meta_sep, '\0'};
						char* meta_sep_sep_saveptr;
						char* mss_tok = MPARC_strtok_r(peb_tok, meta_sep_sep, &meta_sep_sep_saveptr); // mss means meta sep sep
						if(mss_tok == NULL || strcmp(mss_tok, "") == 0 || strcmp(meta_sep_sep_saveptr, "") == 0){
							return MPARC_NOTARCHIVE;
						}

						{
							STANKY_MPAR_FILE_FORMAT_VERSION_REPRESENTATION lversion = 0;

							{
								int status = 0;
								lversion = MPARC_i_parse_version(mss_tok, &status);
								if(status != 1){
									return MPARC_NOTARCHIVE;
								}

								if(lversion > version){
									return MPARC_ARCHIVETOOSHINY;
								}

								structure->loadedVersion = lversion;
							}
						}
					}
				}
			}
			return MPARC_OK;
		}

		static MXPSQL_MPARC_err MPARC_i_parse_entries(MXPSQL_MPARC_t* structure, char* Stringy, int erronduplicate){
			char** entries = NULL;
			char** json_entries = NULL;
			MXPSQL_MPARC_err err = MPARC_OK;
			uint_fast64_t ecount = 0;
			{
				char* entry = NULL;
				
				char* saveptr = NULL;
				char sepsis[2] = {structure->begin_entry_marker, '\0'};
				entry = MPARC_strtok_r(Stringy, sepsis, &saveptr);
				if(entry == NULL) return MPARC_NOTARCHIVE;
				entry = MPARC_strtok_r(NULL, sepsis, &saveptr);
				if(entry == NULL) return MPARC_NOTARCHIVE;
				sepsis[0] = structure->end_entry_marker;
				char* entry2 = MPARC_strtok_r(entry, sepsis, &saveptr);
				{
					char* saveptr2 = NULL;
					char septic[2] = {structure->entry_sep_or_general_marker, '\0'};
					{
						char* edup = const_strdup(entry2);
						if(edup == NULL){
							err = MPARC_OOM;
							goto errhandler;
						}
						char* sp3 = NULL;
						char* e = MPARC_strtok_r(edup, septic, &sp3);
						while(e != NULL) {
							ecount += 1;
							e = MPARC_strtok_r(NULL, septic, &sp3);
						}
						if(edup) free(edup);
					}
					entries = calloc(ecount+1, sizeof(char*));
					json_entries = calloc(ecount+1, sizeof(char*));
					json_entries[ecount] = NULL;
					if(entries == NULL){
						err = MPARC_OOM;
						goto errhandler;
					}
					char* entry64 = MPARC_strtok_r(entry2, septic, &saveptr2);
					for(uint_fast64_t i = 0; entry64 != NULL; i++){
						entries[i] = const_strdup(entry64);
						if(entries[i] == NULL){
							err = MPARC_OOM;
							goto errhandler;
						}
						entry64 = MPARC_strtok_r(NULL, septic, &saveptr2);
					}
					entries[ecount] = NULL;
				}
			}

			for(uint_fast64_t i = 0; i < ecount; i++){
				crc_t crc = crc_init();
				crc_t tcrc = crc_init();
				char* entry = entries[i];
				char* sptr = NULL;
				char seps[2] = {structure->entry_elem2_sep_marker_or_magic_sep_marker, '\0'};
				char* crcstr = MPARC_strtok_r(entry, seps, &sptr);
				if(crcstr == NULL || sptr == NULL || strcmp(sptr, "") == 0){
					err = MPARC_NOTARCHIVE;
					goto errhandler;
				}
				if(sscanf(crcstr, "%"SCNuFAST32, &crc) != 1){ // failed to get checksum, we will never know the real checksum
					errno = EILSEQ;
					err = MPARC_CHKSUM;
					goto errhandler;
				}
				char* tok = sptr;
				tcrc = crc_update(tcrc, tok, strlen(tok));
				tcrc = crc_finalize(tcrc);
				if(tcrc != crc){
					errno = EILSEQ;
					err = MPARC_CHKSUM;
					goto errhandler;
				}
				json_entries[i] = tok;
			}

			jsmn_parser jsmn;
			jsmn_init(&jsmn);
			for(uint_fast64_t i = 0; i < ecount; i++){
				char* filename = NULL;
				char* blob = NULL;
				crc_t crc3 = crc_init();
				bool filename_parsed = false;
				bool blob_parsed = false;
				bool crc3_parsed = false;

				char* jse = json_entries[i];

				if(false){ // disable jsmn for now due to eroneous results (problem with the token ordering)
					static const uint_fast64_t jtokens_count = 128; // we only need 4 but we don't expect more than 128, we put more just in case for other metadata
					jsmntok_t jtokens[jtokens_count]; // this says no to C++
					int jsmn_err = jsmn_parse(&jsmn, jse, strlen(jse), jtokens, jtokens_count);
					if(jsmn_err < 0){
						err = MPARC_NOTARCHIVE;
						goto errhandler;
					}
					for(uint_fast64_t i_jse = 1; i_jse < 5; i_jse++){ // we only need 4 to scan
						jsmntok_t jtoken = jtokens[i_jse];
						char* tok1 = "";
						{
							char* start = &jse[jtoken.start];
							char* end = &jse[jtoken.end];
							char *substr = (char *)calloc(end - start + 1, sizeof(char));
							if(substr == NULL){
								err = MPARC_OOM;
								goto errhandler;
							}
							memcpy(substr, start, end - start);
							tok1 = substr;
						}
						if(tok1) free(tok1);
					}
				}
				else{
					// ((void)json_check);
					((void)json_prepend_member);
					((void)json_prepend_element);
					((void)json_find_member);
					((void)json_find_element);
					((void)json_validate);
					((void)json_encode_string);

					JsonNode* nd = json_decode(jse);
					JsonNode* node = NULL;
					json_foreach(node, nd){
						if(node->tag == JSON_STRING){

							if(strcmp(node->key, "filename") == 0){
								filename = node->store.string;
								filename_parsed = true;
							}
							else if(strcmp(node->key, "blob") == 0){
								blob = node->store.string;
								blob_parsed = true;
							}
							else if(strcmp(node->key, "crcsum") == 0){
								{
									char* nvalue = node->store.string;
									if(sscanf(nvalue, "%"SCNuFAST32, &crc3) != 1){
										errno = EILSEQ;
										err = MPARC_CHKSUM;
										goto errhandler;
									}
									crc3_parsed = true;
								}
							}

						}
					}
				}

				if(!filename_parsed || !blob_parsed || !crc3_parsed){
					err = MPARC_NOTARCHIVE;
					goto errhandler;
				}
				
				{
					uint_fast64_t bsize = 1;
					unsigned char* un64_blob = b64.atob(blob, strlen(blob), &bsize);
					if(un64_blob == NULL){
						err = MPARC_OOM;
						goto errhandler;
					}
					{
						MPARC_blob_store store = {
							bsize,
							un64_blob,
							crc3
						};

						crc_t crc = crc_init();
						crc = crc_update(crc, store.binary_blob, store.binary_size);
						crc = crc_finalize(crc);
						/* printf("%"PRIuFAST32" %"PRIuFAST32"\n", crc, crc3); */
						if(crc != crc3){
							/* errno = EILSEQ;
							err = MPARC_CHKSUM;
							goto errhandler; */
						}
						store.binary_crc = crc;

						// map_set(&structure->globby, filename, store);
						err = MPARC_i_push_ufilestr_advancea(structure, filename, 0, erronduplicate, store.binary_blob, store.binary_size, store.binary_crc);
						if(err != MPARC_OK){
							goto errhandler;
						}
					}
				}

			}

			goto errhandler; // redundant I know

			errhandler:
			for(uint_fast64_t i = 0; i < ecount; i++){
				if(entries[i] != NULL) free(entries[i]);
				// if(json_entries[i] != NULL) free(json_entries[i]);
			}

			return err;
		}

		static MXPSQL_MPARC_err MPARC_i_parse_ender(MXPSQL_MPARC_t* structure, char* stringy){
			char sep[2] = {structure->end_entry_marker, '\0'};
			char* sptr = NULL;
			char* tok = MPARC_strtok_r(stringy, sep, &sptr);
			if(tok == NULL){
				return MPARC_NOTARCHIVE;
			}
			char lastb[2] = {structure->end_file_marker, '\0'};
			if(strcmp(sptr, lastb) != 0){
				return MPARC_NOTARCHIVE;
			}
			return MPARC_OK;
		}


		

		MXPSQL_MPARC_err MPARC_init(MXPSQL_MPARC_t** structure){
				if(!(structure == NULL || *structure == NULL)) return MPARC_IVAL;

				void* memalloc = calloc(1, sizeof(MXPSQL_MPARC_t));
				if(memalloc == NULL) return MPARC_OOM;

				MXPSQL_MPARC_t* istructure = (MXPSQL_MPARC_t*) memalloc;
				if(istructure == NULL){
						return MPARC_IVAL;
				}

				map_init(&istructure->globby);

				// shall not change this
				istructure->magic_byte_sep = ';'; 
				istructure->meta_sep = '$'; 
				istructure->entry_sep_or_general_marker = '\n'; 
				istructure->comment_marker = '#';
				istructure->begin_entry_marker = '>';
				istructure->entry_elem2_sep_marker_or_magic_sep_marker = MPARC_MAGIC_CHKSM_SEP;
				istructure->end_entry_marker = '@';
				istructure->end_file_marker = '~';

				istructure->writerVersion = STANKY_MPAR_FILE_FORMAT_VERSION_NUMBER;
				istructure->loadedVersion = STANKY_MPAR_FILE_FORMAT_VERSION_NUMBER; // default same

				*structure = istructure;

				return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_copy(MXPSQL_MPARC_t** structure, MXPSQL_MPARC_t** targetdest){
			MXPSQL_MPARC_err err = MPARC_OK;
			if(structure == NULL || targetdest == NULL) return MPARC_IVAL;
			if(*targetdest != NULL){
				err = MPARC_destroy(targetdest);
				if(err != MPARC_OK){
					return err;
				}
			}
			MXPSQL_MPARC_t* cp_archive = NULL;
			err = MPARC_init(&cp_archive);
			if(err != MPARC_OK){
				return err;
			}
			// partial deep copy
			{
				char** listy_structure_out = NULL;
				uint_fast64_t listy_structure_sizy_sizey_size = 0;
				err = MPARC_list_array(*structure, &listy_structure_out, &listy_structure_sizy_sizey_size);
				if(err != MPARC_OK){
					return err;
				}
				for(uint_fast64_t i = 0; i < listy_structure_sizy_sizey_size; i++){
					char* filename = listy_structure_out[i];
					MPARC_blob_store e = {0};
					err = MPARC_peek_file(*structure, filename, &e.binary_blob, &e.binary_size);
					if(err != MPARC_OK) {
						goto my_err_handler;
					}
					err = MPARC_push_ufilestr(cp_archive, filename, e.binary_blob, e.binary_size);
				}

				goto my_err_handler;
				my_err_handler:
				if(listy_structure_out) free(listy_structure_out);
			}
			*targetdest = cp_archive;
			return err;
		}

		MXPSQL_MPARC_err MPARC_destroy(MXPSQL_MPARC_t** structure){
				if(structure == NULL || *structure == NULL) return MPARC_IVAL;

				map_deinit(&(*structure)->globby);

				if(*structure) free(*structure);

				*structure = NULL; // invalidation for security

				return MPARC_OK;
		}



		MXPSQL_MPARC_err MPARC_list_array(MXPSQL_MPARC_t* structure, char*** listout,	uint_fast64_t* length){
				if(structure == NULL) {
						return MPARC_IVAL;
				}

				typedef struct anystruct {
						uint_fast64_t len;
						const char* nam;
				} abufinfo;

				uint_fast64_t lentracker = 0;

				char** listout_structure = NULL;

				const char *key;
				map_iter_t iter = map_iter(&structure->globby);

				while ((key = map_next(&structure->globby, &iter))) {
						// printf("L> %s\n", key);
						// printf("%s\n\n", map_get(&structure->globby, key)->binary_blob);
						lentracker++;
				}

				if(length != NULL){
						*length = lentracker;
				}

				if(listout != NULL){

						uint_fast64_t index = 0;
						const char* key2;
						listout_structure = calloc(lentracker+1, sizeof(char*));

						/* map_iter_t iter2 = map_iter(&structure->globby);

						while ((key2 = map_next(&structure->globby, &iter2))) {
								// printf("L> %s\n", key);
								// printf("%s\n\n", map_get(&structure->globby, key)->binary_blob);
								abufinfo bi = {
										strlen(key2),
										key2
								};

								listout_structure[index] = const_strdup(bi.nam);

								// free((char*)bi.nam);

								index++;
						} */

						MXPSQL_MPARC_iter_t* iterator = NULL;
						{
							MXPSQL_MPARC_err err = MPARC_list_iterator_init(&structure, &iterator);
							if(err != MPARC_OK){
								if(listout_structure) free(listout_structure);
								return err;
							}
						}
						if(iterator == NULL) {
							if(listout_structure) free(listout_structure);
							return MPARC_IVAL;
						}
						while((MPARC_list_iterator_next(&iterator, &key2) == MPARC_OK)){
								abufinfo bi = {
										strlen(key2),
										key2
								};

								// printf("L> %s\n", key2);

								// printf("%de\n", key2 == NULL);
								// fflush(stdout);

								listout_structure[index] = const_strdup(bi.nam);

								// free((char*)bi.nam);

								index++;
						}

						listout_structure[index] = NULL;
				}

				MPARC_i_sort(listout_structure);

				if(listout != NULL){
						*listout = listout_structure;
				}
				else{
					for(uint_fast64_t i = 0; i < lentracker; i++){
						if(listout_structure) {
							if(listout_structure[i]) free(listout_structure[i]);
						}
					}

					if(listout_structure) free(listout_structure);
				}

				return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_list_iterator_init(MXPSQL_MPARC_t** structure, MXPSQL_MPARC_iter_t** iterator){
			if(!(structure == NULL || *structure == NULL || iterator == NULL || *iterator == NULL)) return MPARC_IVAL;

			void* memalloc = calloc(1, sizeof(MXPSQL_MPARC_iter_t));
			if(memalloc == NULL) return MPARC_OOM;

			MXPSQL_MPARC_iter_t* iter = (MXPSQL_MPARC_iter_t*) memalloc;
			if(iter == NULL) return MPARC_IVAL;

			iter->archive = *structure;
			iter->itery = map_iter(&(*structure)->globby);

			*iterator = iter;

			return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_list_iterator_next(MXPSQL_MPARC_iter_t** iterator, const char** outnam){
			if(iterator == NULL || *iterator == NULL) return MPARC_IVAL;

			MXPSQL_MPARC_iter_t* iterye = *iterator;

			const char* nkey = map_next(&iterye->archive->globby, &iterye->itery);

			// printf("eeeee\n");
			// fflush(stdout);

			if(nkey == NULL) return MPARC_KNOEXIST;

			if(outnam != NULL) *outnam = nkey;

			return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_list_iterator_destroy(MXPSQL_MPARC_iter_t** iterator){
			if(iterator == NULL || *iterator == NULL) return MPARC_IVAL;
			if(*iterator) free(*iterator);
			return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_exists(MXPSQL_MPARC_t* structure, char* filename){
				if(structure == NULL || filename == NULL) return MPARC_IVAL;
				if(true){
					if((map_get(&structure->globby, filename)) == NULL) return MPARC_KNOEXIST;
				}
				else{
					char** listy_out = NULL;
					uint_fast64_t listy_size = 0;
					{
						MXPSQL_MPARC_err err = MPARC_list_array(structure, &listy_out, &listy_size);
						if(err != MPARC_OK) return err;
					}
					{
						char* p = bsearch(filename, listy_out, listy_size, sizeof(*listy_out), voidstrcmp);
						if(listy_out) free(listy_out);
						if( p == NULL && map_get(&structure->globby, filename) == NULL) return MPARC_KNOEXIST;
					}
				}
				return MPARC_OK;
		}


		MXPSQL_MPARC_err MPARC_push_ufilestr_advance(MXPSQL_MPARC_t* structure, char* filename, int stripdir, int overwrite, unsigned char* ustringc, uint_fast64_t sizy){
			crc_t crc3 = crc_init();
			crc3 = crc_update(crc3, ustringc, sizy);
			crc3 = crc_finalize(crc3);

			MPARC_i_push_ufilestr_advancea(structure, filename, stripdir, overwrite, ustringc, sizy, crc3);

			return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_push_ufilestr(MXPSQL_MPARC_t* structure, char* filename, unsigned char* ustringc, uint_fast64_t sizy){
			return MPARC_push_ufilestr_advance(structure, filename, 0, 1, ustringc, sizy);
		}

		MXPSQL_MPARC_err MPARC_push_voidfile(MXPSQL_MPARC_t* structure, char* filename, void* buffer_guffer, uint_fast64_t sizey){
			return MPARC_push_ufilestr(structure, filename, (unsigned char*)buffer_guffer, sizey);
		}

		MXPSQL_MPARC_err MPARC_push_filestr(MXPSQL_MPARC_t* structure, char* filename, char* stringc, uint_fast64_t sizey){
				return MPARC_push_ufilestr(structure, filename, (unsigned char*) stringc, sizey);
		}

		MXPSQL_MPARC_err MPARC_push_filename(MXPSQL_MPARC_t* structure, char* filename){
				if(structure == NULL) {
					return MPARC_IVAL;
				}
				if(filename == NULL) {
					return MPARC_IVAL;
				}

				FILE* fpstream = fopen(filename, "rb");
				if(fpstream == NULL){
					return MPARC_FERROR;
				}

				MXPSQL_MPARC_err err = MPARC_push_filestream(structure, fpstream, filename);    

				fclose(fpstream);

				return err;
		}

		MXPSQL_MPARC_err MPARC_push_filestream(MXPSQL_MPARC_t* structure, FILE* filestream, char* filename){
				if(filestream == NULL){
					return MPARC_FERROR;
				}

				if(filename == NULL){
					return MPARC_IVAL;
				}
				

				unsigned char* binary = NULL;

				uint_fast64_t filesize = 0; // byte count
				if(fseek(filestream, 0, SEEK_SET) != 0){
					return MPARC_FERROR;
				}

				while(fgetc(filestream) != EOF && !ferror(filestream)){
						filesize += 1;
				}
				if(ferror(filestream)){
						return MPARC_FERROR;
				}

				if(fseek(filestream, 0, SEEK_SET) != 0){
						return MPARC_FERROR;
				}

				binary = calloc(filesize+1, sizeof(unsigned char));

				if(filesize >= MPARC_DIRECTF_MINIMUM){
					if(fread(binary, sizeof(unsigned char), filesize, filestream) < filesize && ferror(filestream)){
						if(binary) free(binary);
						return MPARC_FERROR;
					}
				}
				else{
					int c = 0;
					uint_fast64_t i = 0;
					while((c = fgetc(filestream)) != EOF){
						binary[i++] = c;
					}

					if(ferror(filestream)){
						if(binary) free(binary);
						return MPARC_FERROR;
					}

					binary[i] = '\0';
				}

				MXPSQL_MPARC_err err = MPARC_push_ufilestr(structure, filename, binary, filesize);

				if(binary) free(binary);
				return err;
		}


		MXPSQL_MPARC_err MPARC_pop_file(MXPSQL_MPARC_t* structure, char* filename){
				if(MPARC_exists(structure, filename) == MPARC_KNOEXIST) return MPARC_KNOEXIST;
				map_remove(&structure->globby, filename);
				return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_clear_file(MXPSQL_MPARC_t* structure){
			char** entryos = NULL;
			uint_fast64_t eos_s = 0;
			MXPSQL_MPARC_err err = MPARC_list_array(structure, &entryos, &eos_s);
			if(err != MPARC_OK) return err;
			for(uint_fast64_t i = 0; i < eos_s; i++){
				MPARC_pop_file(structure, entryos[i]);
			}
			if(entryos) free(entryos);
			return err;
		}


		static MXPSQL_MPARC_err MPARC_peek_file_advance(MXPSQL_MPARC_t* structure, char* filename, unsigned char** bout, uint_fast64_t* sout, crc_t* crout){ // users don't need to know the crc
				if(MPARC_exists(structure, filename) == MPARC_KNOEXIST) return MPARC_KNOEXIST;
				if(bout != NULL) *bout = map_get(&structure->globby, filename)->binary_blob;
				if(sout != NULL) *sout = map_get(&structure->globby, filename)->binary_size;
				if(crout != NULL) *crout = map_get(&structure->globby, filename)->binary_crc;
				return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_peek_file(MXPSQL_MPARC_t* structure, char* filename, unsigned char** bout, uint_fast64_t* sout){
			return MPARC_peek_file_advance(structure, filename, bout, sout, NULL);
		}

		/*
		 * 
		 * SEE THIS TO SEE THE FILE FORMAT OF THE ARCHIVE
		 * THIS PART IS IMPORTANT TO SEE HOW IT IS IMPLEMENTED AND THE FORMAT
		 * 
		 * How is the file constructed (along with little parsing information):
		 * 
		 * 1. Build the header:
		 * Format: MXPSQL's Portable Archive;[VERSION]${JSON_WHATEV_METADATA}>[NEWLINE]
		 * 
		 * The ';' character separates the Magic numbers (very long with 25 character I think) from the version number and json metadata
		 * 
		 * The '$' character separates the version and magic numbers from the metadata
		 * 
		 * The '>' character works to indicate the start of entries
		 * The newline is an anomaly though, but just put it in there
		 * 
		 * JSON_WHATEV_METADATA can be implementation defined
		 * This C implementation will ignore any extra metadata
		 * 
		 * Parsing tips:
		 * Split ';' from the whole archive to get the magic number first
		 * Then split '>' from to get the special info header
		 * The split '$' from the special info header to get the version and extra metadata
		 * 
		 * 
		 * 2. Build the entries
		 * Format: [CRC32_OF_JSON]%{"filename":[FILENAME],"blob":[BASE64_BINARY], "crcsum":[CRC32_OF_blob]}[NEWLINE]
		 * 
		 * The '%' character is to separate the checksum of the JSON from the JSON itself
		 * 
		 * You can add other metadata like date of creation, but there must be the entries "filename", "blob" and "crcsum" in the JSON
		 * This C implementation will ignore any extra metadata
		 * 
		 * "filename" should contain the filename. (don't do any effects and magic on this field called "filename")
		 * "blob" should contain the base64 of the binary or text file. (base64 to make it a text file and not binary)
		 * "crcsum" should contain the CRC32 checksum of the content of "blob" after converting it back to it's original form. ("blob" but wihtout base64)
		 * 
		 * Repeat this as required (how many entries are there you repeat)
		 * 
		 * Construction note:
		 * The anomaly mention aboved is because the newline is added before the main content
		 * 
		 * Parsing note:
		 * Make sure to split all the entries first (split by newline, '>' )
		 * 
		 * 
		 * 
		 * 3. Build the footer
		 * Format: @~
		 * 
		 * the '@' character is to signify end of entry
		 * the '~' character is to signify end of file
		 * 
		 * 
		 * Follow this (with placeholder) and you get this:
		 * MXPSQL's Portable Archive;[VERSION]${JSON_WHATEV_METADATA}>[NEWLINE][CRC32_OF_JSON]%{"filename":[FILENAME],"blob":[BASE64_BINARY], "crcsum":[CRC32_OF_blob]}[NEWLINE]@~
		 * 
		 * A real single entried one:
		 * MXPSQL's Portable Archive;1${"WhatsThis": "MPARC Logo lmao-Hahahaha"}>
		 * 134131812%{"filename":"./LICENSE.MIT","blob":"TUlUIExpY2Vuc2UKCkNvcHlyaWdodCAoYykgMjAyMiBNWFBTUUwKClBlcm1pc3Npb24gaXMgaGVyZWJ5IGdyYW50ZWQsIGZyZWUgb2YgY2hhcmdlLCB0byBhbnkgcGVyc29uIG9idGFpbmluZyBhIGNvcHkKb2YgdGhpcyBzb2Z0d2FyZSBhbmQgYXNzb2NpYXRlZCBkb2N1bWVudGF0aW9uIGZpbGVzICh0aGUgIlNvZnR3YXJlIiksIHRvIGRlYWwKaW4gdGhlIFNvZnR3YXJlIHdpdGhvdXQgcmVzdHJpY3Rpb24sIGluY2x1ZGluZyB3aXRob3V0IGxpbWl0YXRpb24gdGhlIHJpZ2h0cwp0byB1c2UsIGNvcHksIG1vZGlmeSwgbWVyZ2UsIHB1Ymxpc2gsIGRpc3RyaWJ1dGUsIHN1YmxpY2Vuc2UsIGFuZC9vciBzZWxsCmNvcGllcyBvZiB0aGUgU29mdHdhcmUsIGFuZCB0byBwZXJtaXQgcGVyc29ucyB0byB3aG9tIHRoZSBTb2Z0d2FyZSBpcwpmdXJuaXNoZWQgdG8gZG8gc28sIHN1YmplY3QgdG8gdGhlIGZvbGxvd2luZyBjb25kaXRpb25zOgoKVGhlIGFib3ZlIGNvcHlyaWdodCBub3RpY2UgYW5kIHRoaXMgcGVybWlzc2lvbiBub3RpY2Ugc2hhbGwgYmUgaW5jbHVkZWQgaW4gYWxsCmNvcGllcyBvciBzdWJzdGFudGlhbCBwb3J0aW9ucyBvZiB0aGUgU29mdHdhcmUuCgpUSEUgU09GVFdBUkUgSVMgUFJPVklERUQgIkFTIElTIiwgV0lUSE9VVCBXQVJSQU5UWSBPRiBBTlkgS0lORCwgRVhQUkVTUyBPUgpJTVBMSUVELCBJTkNMVURJTkcgQlVUIE5PVCBMSU1JVEVEIFRPIFRIRSBXQVJSQU5USUVTIE9GIE1FUkNIQU5UQUJJTElUWSwKRklUTkVTUyBGT1IgQSBQQVJUSUNVTEFSIFBVUlBPU0UgQU5EIE5PTklORlJJTkdFTUVOVC4gSU4gTk8gRVZFTlQgU0hBTEwgVEhFCkFVVEhPUlMgT1IgQ09QWVJJR0hUIEhPTERFUlMgQkUgTElBQkxFIEZPUiBBTlkgQ0xBSU0sIERBTUFHRVMgT1IgT1RIRVIKTElBQklMSVRZLCBXSEVUSEVSIElOIEFOIEFDVElPTiBPRiBDT05UUkFDVCwgVE9SVCBPUiBPVEhFUldJU0UsIEFSSVNJTkcgRlJPTSwKT1VUIE9GIE9SIElOIENPTk5FQ1RJT04gV0lUSCBUSEUgU09GVFdBUkUgT1IgVEhFIFVTRSBPUiBPVEhFUiBERUFMSU5HUyBJTiBUSEUKU09GVFdBUkUu","crcsum":"15584406"}@~
		 *
		 * A real (much more real) multi entried one:
		 * MXPSQL's Portable Archive;1${}>
		 * 3601911152%{"filename":"LICENSE","blob":"U2VlIExJQ0VOU0UuTEdQTCBhbmQgTElDRU5TRS5NSVQgYW5kIGNob29zZSBvbmUgb2YgdGhlbS4KCkxJQ0VOU0UuTEdQTCBjb250YWlucyBMR1BMLTIuMS1vci1sYXRlciBsaWNlbnNlLgpMSUNFTlNFLk1JVCBjb250YWlucyBNSVQgbGljZW5zZS4KCkxJQ0VOU0UuTEdQTCBhbmQgTElDRU5TRS5NSVQgc2hvdWxkIGJlIGRpc3RyaWJ1dGVkIHRvZ2V0aGVyIHdpdGggeW91ciBjb3B5LCBpZiBub3QsIHNvbWV0aGluZyBpcyB3cm9uZy4=","crcsum":"404921597"}
		 * 59879441%{"filename":"LICENSE.MIT","blob":"TUlUIExpY2Vuc2UKCkNvcHlyaWdodCAoYykgMjAyMiBNWFBTUUwKClBlcm1pc3Npb24gaXMgaGVyZWJ5IGdyYW50ZWQsIGZyZWUgb2YgY2hhcmdlLCB0byBhbnkgcGVyc29uIG9idGFpbmluZyBhIGNvcHkKb2YgdGhpcyBzb2Z0d2FyZSBhbmQgYXNzb2NpYXRlZCBkb2N1bWVudGF0aW9uIGZpbGVzICh0aGUgIlNvZnR3YXJlIiksIHRvIGRlYWwKaW4gdGhlIFNvZnR3YXJlIHdpdGhvdXQgcmVzdHJpY3Rpb24sIGluY2x1ZGluZyB3aXRob3V0IGxpbWl0YXRpb24gdGhlIHJpZ2h0cwp0byB1c2UsIGNvcHksIG1vZGlmeSwgbWVyZ2UsIHB1Ymxpc2gsIGRpc3RyaWJ1dGUsIHN1YmxpY2Vuc2UsIGFuZC9vciBzZWxsCmNvcGllcyBvZiB0aGUgU29mdHdhcmUsIGFuZCB0byBwZXJtaXQgcGVyc29ucyB0byB3aG9tIHRoZSBTb2Z0d2FyZSBpcwpmdXJuaXNoZWQgdG8gZG8gc28sIHN1YmplY3QgdG8gdGhlIGZvbGxvd2luZyBjb25kaXRpb25zOgoKVGhlIGFib3ZlIGNvcHlyaWdodCBub3RpY2UgYW5kIHRoaXMgcGVybWlzc2lvbiBub3RpY2Ugc2hhbGwgYmUgaW5jbHVkZWQgaW4gYWxsCmNvcGllcyBvciBzdWJzdGFudGlhbCBwb3J0aW9ucyBvZiB0aGUgU29mdHdhcmUuCgpUSEUgU09GVFdBUkUgSVMgUFJPVklERUQgIkFTIElTIiwgV0lUSE9VVCBXQVJSQU5UWSBPRiBBTlkgS0lORCwgRVhQUkVTUyBPUgpJTVBMSUVELCBJTkNMVURJTkcgQlVUIE5PVCBMSU1JVEVEIFRPIFRIRSBXQVJSQU5USUVTIE9GIE1FUkNIQU5UQUJJTElUWSwKRklUTkVTUyBGT1IgQSBQQVJUSUNVTEFSIFBVUlBPU0UgQU5EIE5PTklORlJJTkdFTUVOVC4gSU4gTk8gRVZFTlQgU0hBTEwgVEhFCkFVVEhPUlMgT1IgQ09QWVJJR0hUIEhPTERFUlMgQkUgTElBQkxFIEZPUiBBTlkgQ0xBSU0sIERBTUFHRVMgT1IgT1RIRVIKTElBQklMSVRZLCBXSEVUSEVSIElOIEFOIEFDVElPTiBPRiBDT05UUkFDVCwgVE9SVCBPUiBPVEhFUldJU0UsIEFSSVNJTkcgRlJPTSwKT1VUIE9GIE9SIElOIENPTk5FQ1RJT04gV0lUSCBUSEUgU09GVFdBUkUgT1IgVEhFIFVTRSBPUiBPVEhFUiBERUFMSU5HUyBJTiBUSEUKU09GVFdBUkUu","crcsum":"15584406"}@~
		*/
		MXPSQL_MPARC_err MPARC_construct_str(MXPSQL_MPARC_t* structure, char** output){
				static char* fmt = "%s%s%s";
				MXPSQL_MPARC_err err = MPARC_OK;

				char* top = MPARC_i_construct_header(structure);
				if(top == NULL) {
					return MPARC_CONSTRUCT_FAIL;
				}
				char* mid = MPARC_i_construct_entries(structure, &err);
				if(mid == NULL) {
					if(top) free(top);
					top = NULL;
					return err;
				}
				char* bottom = MPARC_i_construct_ender(structure);
				if(bottom == NULL) {
					if(top) free(top);
					if(mid) free(mid);
					top = NULL;
					mid = NULL;
					return MPARC_CONSTRUCT_FAIL;
				}

				{
					// fprintf(stdout, fmt, top, mid, bottom);
					int sizy = snprintf(NULL, 0, fmt, top, mid, bottom);
					if(sizy < 0){
						if(top) free(top);
						if(mid) free(mid);
						if(bottom) free(bottom);
						top = NULL;
						mid = NULL;
						bottom = NULL;
						return MPARC_CONSTRUCT_FAIL;
					}
					char* alloca_out = calloc(sizy+1, sizeof(char));
					if(alloca_out == NULL){
						if(top) free(top);
						if(mid) free(mid);
						if(bottom) free(bottom);
						top = NULL;
						mid = NULL;
						bottom = NULL;
						return MPARC_OOM;
					}
					if(snprintf(alloca_out, sizy+1, fmt, top, mid, bottom) < 0){
						if(top) free(top);
						if(mid) free(mid);
						if(bottom) free(bottom);
						if(alloca_out) free(alloca_out);
						top = NULL;
						mid = NULL;
						bottom = NULL;
						alloca_out = NULL;
						return MPARC_CONSTRUCT_FAIL;
					}
					*output = alloca_out;
				}
				if(top) free(top);
				if(mid) free(mid);
				if(bottom) free(bottom);
				top = NULL;
				mid = NULL;
				bottom = NULL;
				return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_construct_filename(MXPSQL_MPARC_t* structure, char* filename){
			FILE* fpstream = fopen(filename, "wb+");
			if(fpstream == NULL){
				return MPARC_FERROR;
			}
			MXPSQL_MPARC_err err = MPARC_construct_filestream(structure, fpstream);
			fclose(fpstream);
			return err;
		}

		MXPSQL_MPARC_err MPARC_construct_filestream(MXPSQL_MPARC_t* structure, FILE* fpstream){
			if(fpstream == NULL){
				return MPARC_IVAL;
			}

			char* archive = NULL;
			MXPSQL_MPARC_err err = MPARC_construct_str(structure, &archive);
			if(err != MPARC_OK){
				if(archive) free(archive);
				return err;
			}
			uint_fast64_t count = strlen(archive);
			if(count >= MPARC_DIRECTF_MINIMUM){
				if(fwrite(archive, sizeof(char), count, fpstream) < count && ferror(fpstream)){
					if(archive) free(archive);
					return MPARC_FERROR;
				}
			}
			else{
				for(uint_fast64_t i = 0; i < count; i++){
					if(fputc(archive[i], fpstream) == EOF){
						if(archive) free(archive);
						err = MPARC_FERROR;
						return err;
					}
				}
			}
			fflush(fpstream);
			if(archive) free(archive);
			return err;
		}

		
		MXPSQL_MPARC_err MPARC_extract_advance(MXPSQL_MPARC_t* structure, char* destdir, char** dir2make, void (*on_item)(const char*), int (*mk_dir)(char*)){
			{
				char** listy = NULL;
				uint_fast64_t listys = 0;
				if(MPARC_list_array(structure, &listy, &listys) != MPARC_OK){
					return MPARC_IVAL;
				}

				for(uint_fast64_t i = 0; i < listys; i++){
					if(dir2make != NULL) *dir2make = NULL;
					char* fname = NULL;
					const char* nkey = listy[i];
					FILE* fps = NULL;
					if(on_item) (*on_item)(nkey);
					rmkdir_goto_label_spot:
					{
						{
							fname = const_strdup(nkey);
							uint_fast64_t pathl = strlen(fname)+strlen(nkey)+1;
							void* nfname = realloc(fname, pathl+1);
							if(nfname == NULL){
								if(fname) free(fname);
								if(listy) free(listy);
								return MPARC_OOM;
							}
							fname = (char*) nfname;
							int splen = snprintf(fname, pathl, "%s/%s", destdir, nkey);
							if(splen < 0 || ((uint_fast64_t)splen) > pathl){
								if(fname) free(fname);
								if(listy) free(listy);
								return MPARC_IVAL;
							}
						}
						fps = fopen(fname, "wb+");
						if(fps == NULL){
							char* dname = MPARC_dirname((char*)fname);
							#if defined(ENOENT)
							if(errno == ENOENT){
								// this means "I request you to make me a directory and then call me when you are done so I can continue to do my own agenda which is to help you, basically I need your help for me to help you"
								if(mk_dir){
									if((*mk_dir)(dname) != 0){
										if(fname) free(fname);
										if(listy) free(listy);
										return MPARC_FERROR;
									}
									if(fname) free(fname);
									if(listy) free(listy);
									// i--; // hacky
									// continue;
									goto rmkdir_goto_label_spot; // much better (don't object to this method of using goto and labels, the old one involes decrmenting the index variable and that is a hacky solution)
								}
								else{
									if(dir2make != NULL) *dir2make = dname;
								}
								if(fname) free(fname);
								if(listy) free(listy);
								return MPARC_OPPART;
							}
							#else
							((void)mk_dir);
							if(dir2make != NULL) *dir2make = dname;
							#endif
							if(fname) free(fname);
							if(listy) free(listy);
							return MPARC_IVAL;
						}
						{
							unsigned char* bout = NULL;
							uint_fast64_t sout = 0;
							crc_t crc3 = 0;
							MXPSQL_MPARC_err err = MPARC_peek_file_advance(structure, (char*) nkey, &bout, &sout, &crc3);
							if(err != MPARC_OK){
								if(fname) free(fname);
								if(listy) free(listy);
								fclose(fps);
								return err;
							}
							if(sout >= MPARC_DIRECTF_MINIMUM){
								if(fwrite(bout, sizeof(unsigned char), sout, fps) < sout){
									if(ferror(fps)){
										if(fname) free(fname);
										if(listy) free(listy);
										fclose(fps);
										return MPARC_FERROR;
									}
								}
							}
							else{
								// char abc = {'a', 'b', 'c', '\0'};
								for(uint_fast64_t i = 0; i < sout; i++){
									if(fputc(bout[i], fps) == EOF){
										if(fname) free(fname);
										if(listy) free(listy);
										fclose(fps);
										return MPARC_FERROR;
									}
								}
							}
							fflush(fps);
							if(fseek(fps, 0, SEEK_SET) != 0){
								if(fname) free(fname);
								if(listy) free(listy);
								fclose(fps);
								return MPARC_FERROR;
							}
							unsigned char* binary = calloc(sout+1, sizeof(char));
							if(binary == NULL){
								if(fname) free(fname);
								if(listy) free(listy);
								fclose(fps);
								return MPARC_OOM;
							}
							if(sout >= MPARC_DIRECTF_MINIMUM){
								if(fread(binary, sizeof(unsigned char), sout, fps) < sout){
									if(ferror(fps)){
										if(fname) free(fname);
										if(binary) free(binary);
										if(listy) free(listy);
										fclose(fps);
										return MPARC_FERROR;
									}
								}
							}
							else{
								int c = 0;
								uint_fast64_t i = 0;
								while((c = fgetc(fps)) != EOF){
									binary[i++] = c;
								}

								if(ferror(fps)){
									if(binary) free(binary);
									return MPARC_FERROR;
								}

								binary[i] = '\0';
							}
							{
								crc_t crc = crc_init();
								crc = crc_update(crc, binary, sout);
								crc = crc_finalize(crc);
								if(crc != crc3){
									if(fname) free(fname);
									if(binary) free(binary);
									if(listy) free(listy);
									fclose(fps);
									errno = EILSEQ;
									return MPARC_CHKSUM;
								}
							}
						}
						if(fflush(fps) == EOF){
							if(fname) free(fname);
							if(listy) free(listy);
							fclose(fps);
							return MPARC_FERROR;
						}
					}

					if(fname) free(fname);
					fclose(fps);
				}
				if(listy) free(listy);
			}
			return MPARC_OK;
		}

		MXPSQL_MPARC_err MPARC_extract(MXPSQL_MPARC_t* structure, char* destdir, char** dir2make){
			return MPARC_extract_advance(structure, destdir, dir2make, NULL, NULL);
		}


		MXPSQL_MPARC_err MPARC_readdir(MXPSQL_MPARC_t* structure, char* srcdir, int recursive, int (*listdir)(char*, int, char**)){
			if(listdir == NULL) return MPARC_IVAL;

			char** flists = NULL;
			MXPSQL_MPARC_err err = MPARC_OK;

			if(listdir(srcdir, recursive, flists) != 0){
				err = MPARC_FERROR;
				goto main_errhandler;
			}

			for(uint_fast64_t i = 0; flists[i] != NULL; i++){
				err = MPARC_push_filename(structure, flists[i]);
				if(err != MPARC_OK){
					goto main_errhandler;
				}
			}


			goto main_errhandler;
			main_errhandler:
			for(uint_fast64_t i = 0; flists[i] != NULL; i++){
				if(flists[i] != NULL) {
					free(flists[i]);
				}
			}
			if(flists) free(flists);

			return err;
		}


		MXPSQL_MPARC_err MPARC_parse_str_advance(MXPSQL_MPARC_t* structure, char* stringy, int erronduplicate){
			MXPSQL_MPARC_err err = MPARC_OK;
			{
				char* s3 = const_strdup(stringy);
				if(s3 == NULL) {
					err = MPARC_OOM;
					goto endy;
				}
				err = MPARC_i_parse_header(structure, s3);
				if(s3) free(s3);
				if(err != MPARC_OK) {
					goto endy;
				}
			}
			{
				char* s3 = const_strdup(stringy);
				if(s3 == NULL) {
					err = MPARC_OOM;
					goto endy;
				}
				err = MPARC_i_parse_entries(structure, s3, erronduplicate);
				if(s3) free(s3);
				if(err != MPARC_OK) {
					goto endy;
				}
			}
			{
				char* s3 = const_strdup(stringy);
				if(s3 == NULL) {
					err = MPARC_OOM;
					goto endy;
				}
				err = MPARC_i_parse_ender(structure, s3);
				if(s3) free(s3);
				if(err != MPARC_OK) {
					goto endy;
				}
			}

			goto endy; // redundant

			endy:
			return err;
		}

		MXPSQL_MPARC_err MPARC_parse_str(MXPSQL_MPARC_t* structure, char* stringy){
			return MPARC_parse_str_advance(structure, stringy, 0);
		}

		MXPSQL_MPARC_err MPARC_parse_filestream(MXPSQL_MPARC_t* structure, FILE* fpstream){
			uint_fast64_t filesize = 0;
			if(fseek(fpstream, 0, SEEK_SET) != 0){
				return MPARC_FERROR;
			}

			while(fgetc(fpstream) != EOF && !ferror(fpstream)){
				filesize += 1;
			}
			if(ferror(fpstream)){
				return MPARC_FERROR;
			}

			if(fseek(fpstream, 0, SEEK_SET) != 0){
					return MPARC_FERROR;
			}

			char* binary = calloc(filesize+1, sizeof(unsigned char));
			if(binary == NULL){
				return MPARC_OOM;
			}

			if(filesize >= MPARC_DIRECTF_MINIMUM){
				if(fread(binary, sizeof(unsigned char), filesize, fpstream) < filesize && ferror(fpstream)){
					if(binary) free(binary);
					return MPARC_FERROR;
				}
			}
			else{
				int c = 0;
				uint_fast64_t i = 0;
				while((c = fgetc(fpstream)) != EOF){
					binary[i++] = c;
				}

				if(ferror(fpstream)){
					if(binary) free(binary);
					return MPARC_FERROR;
				}

				binary[i] = '\0';
			}

			MXPSQL_MPARC_err err = MPARC_parse_str(structure, binary);
			return err;
		}

		MXPSQL_MPARC_err MPARC_parse_filename(MXPSQL_MPARC_t* structure, char* filename){
			FILE* filepointerstream = fopen(filename, "r");
			if(filepointerstream == NULL) return MPARC_FERROR;
			MXPSQL_MPARC_err err = MPARC_parse_filestream(structure, filepointerstream);
			fclose(filepointerstream);
			return err;
		}
		/* END OF MAIN CODE */

/* END OF MY SECTION */


#endif
