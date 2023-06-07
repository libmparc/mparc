/**
 * @brief The new C++11 (with a bit of C++17 if available) rewrite of MPARC from the spaghetti C99 code. Source file.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "mparc.hpp"
#include "nlohmann/json.hpp"
#include "base64.hpp"

// MACROS?????

// http://info.isl.ntt.co.jp/crypt/eng/camellia/dl/camellia-BSD-1.1.0.tar.gz
/**
 * Copyright (c) 2006
 * NTT (Nippon Telegraph and Telephone Corporation) . All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer as
 *   the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NTT ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NTT BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#if BYTE_ORDER == _LITTLE_ENDIAN || __i386__ || __alpha__ || _M_IX86
#define L_ENDIAN
#else
#undef L_ENDIAN
#endif

// http://info.isl.ntt.co.jp/crypt/eng/camellia/dl/camellia-BSD-1.1.0.tar.gz
/**
 * Copyright (c) 2006
 * NTT (Nippon Telegraph and Telephone Corporation) . All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer as
 *   the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NTT ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NTT BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define CAMELLIA_SIGMA1L (0xA09E667FL)
#define CAMELLIA_SIGMA1R (0x3BCC908BL)
#define CAMELLIA_SIGMA2L (0xB67AE858L)
#define CAMELLIA_SIGMA2R (0x4CAA73B2L)
#define CAMELLIA_SIGMA3L (0xC6EF372FL)
#define CAMELLIA_SIGMA3R (0xE94F82BEL)
#define CAMELLIA_SIGMA4L (0x54FF53A5L)
#define CAMELLIA_SIGMA4R (0xF1D36F1CL)
#define CAMELLIA_SIGMA5L (0x10E527FAL)
#define CAMELLIA_SIGMA5R (0xDE682D1DL)
#define CAMELLIA_SIGMA6L (0xB05688C2L)
#define CAMELLIA_SIGMA6R (0xB3E6C1FDL)

// http://info.isl.ntt.co.jp/crypt/eng/camellia/dl/camellia-BSD-1.1.0.tar.gz
/**
 * Copyright (c) 2006
 * NTT (Nippon Telegraph and Telephone Corporation) . All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer as
 *   the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NTT ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NTT BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define CAMELLIA_BLOCK_SIZE 16
#define CAMELLIA_TABLE_BYTE_LEN 272
#define CAMELLIA_TABLE_WORD_LEN (CAMELLIA_TABLE_BYTE_LEN / 4)



// NAMESPACE ALIASING
namespace MPARC11 = MXPSQL::MPARC11;
namespace Utils = MPARC11::Utils;
using ByteArray = MPARC11::ByteArray;
using Status = MPARC11::Status;
using MPARC = MPARC11::MPARC;
using Entry = MPARC11::Entry;


using json = nlohmann::json;
namespace b64 = base64;


#ifdef MXPSQL_MPARC_FSLIB
    #if MXPSQL_MPARC_FSLIB == 1
        namespace stdfs = std::filesystem;
        namespace fslib = stdfs;
    #elif MXPSQL_MPARC_FSLIB == 2
        namespace ghcfs = ghc::filesystem;
        namespace fslib = ghcfs;
    #endif
#endif

// TYPIST
typedef std::uint_fast32_t crc_t;
typedef std::uint_fast32_t fletcher32_t;
// http://info.isl.ntt.co.jp/crypt/eng/camellia/dl/camellia-BSD-1.1.0.tar.gz
/**
 * Copyright (c) 2006
 * NTT (Nippon Telegraph and Telephone Corporation) . All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer as
 *   the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NTT ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NTT BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#if defined(_MSC_VER)
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif
typedef unsigned int CAMELLIA_KEY_TABLE_TYPE[CAMELLIA_TABLE_WORD_LEN];

// Variables
static const constexpr int CAMELLIA_MAX_PAD_PLUS = 128;

// prototyping
static Status construct_entries(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_header(MPARC& archive, std::string& output, MPARC::version_type ver);
static Status construct_footer(MPARC& archive, std::string& output, MPARC::version_type ver);

static Status parse_header(MPARC& archive, std::string header_input);
static Status parse_entries(MPARC& archive, std::string entry_input);
static Status parse_footer(MPARC& archive, std::string footer_input);

static std::string encrypt_xor(std::string input, std::string key);
static std::string encrypt_rot(std::string input, std::vector<int> key);

[[noreturn]] static inline void my_unreachable(...);



// CRC32 implementation
namespace{
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
    static crc_t crc_update(crc_t crc, const void *data, size_t data_len)
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
     * \return     The
     */
    static inline crc_t crc_finalize(crc_t crc)
    {
    	return crc ^ 0xffffffff;
    }

    /**
     * Convert the crc value to a string.
     * 
     * \param[in] in The current crc value.
     * \return std::string The string.
     */
    static inline std::string crc_t_to_string(crc_t in){
        return std::to_string(in);
    }

    /**
     * Parse a string into the checksum 
     * 
     * \param checksum The string to parse from
     * \param[out] out The output from [checksum] parsing
     * \return true success
     * \return false failure
     */
    static inline bool scan_crc_t_from_string(std::string checksum, crc_t& out){
        return (sscanf(checksum.c_str(), "%" SCNuFAST32, &out) >= 1);
    }





    /**
     * @brief Calculate the fletcher32 checksum
     * 
     * @param data 16 bit blocks
     * @param len Length
     * @return fletcher32_t Fletcher checksum
     */
    fletcher32_t fletcher_t_update( const uint16_t *data, size_t len )
    {
            uint32_t sum1 = 0xffff, sum2 = 0xffff;

            while (len) {
                    unsigned tlen = len > 359 ? 359 : len;

                    len -= tlen;

                    do {
                      sum1 += *data++;
                      sum2 += sum1;
                    } while (--tlen);

                    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
                    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
            }
            /* Second reduction step to reduce sums to 16 bits */
            sum1 = (sum1 & 0xffff) + (sum1 >> 16);
            sum2 = (sum2 & 0xffff) + (sum2 >> 16);
            return sum2 << 16 | sum1;
    }

    /**
     * @brief Convert the fletcher32 value to a string
     * 
     * @param f Input fletcher32 checksum value.
     * @return std::string The fletcher32 checksum value as a string.
     */
    static inline std::string fletcher_t_to_string(fletcher32_t f){
        return std::to_string(f);
    }

    /**
     * @brief Parse a fletcher32 value from a string
     * 
     * @param checksum The string to be parsed
     * @param out The fletcher32 that was parsed to
     * @return true Success
     * @return false Failure
     */
    static inline bool scan_fletcher_t_from_string(std::string checksum, fletcher32_t& out){
        return (sscanf(checksum.c_str(), "%" SCNuFAST32, &out) >= 1);
    }
}



// Camellia cipher
namespace{
    #if defined(_MSC_VER)
    # define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
    # define GETU32(p) SWAP(*((uint32_t *)(p)))
    # define CAMELLIA_SWAP4(x) (x = (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00))
    #else /* not MS-VC */
    # define GETU32(pt) (((uint32_t)(pt)[0] << 24)		\
    		     ^ ((uint32_t)(pt)[1] << 16)	\
    		     ^ ((uint32_t)(pt)[2] <<  8)	\
    		     ^ ((uint32_t)(pt)[3]))
    
    #ifdef L_ENDIAN
    #define CAMELLIA_SWAP4(x)				\
        do {						\
    	x = ((uint32_t)x << 16) + ((uint32_t)x >> 16);	\
    	x = (((uint32_t)x & 0xff00ff) << 8) +		\
    	    (((uint32_t)x >> 8) & 0xff00ff);		\
        } while(0)
    #else /* big endian */
    #define CAMELLIA_SWAP4(x)
    #endif /* L_ENDIAN */
    #endif
    
    #define COPY4WORD(dst, src)			\
        do {					\
    	(dst)[0]=(src)[0];			\
    	(dst)[1]=(src)[1];			\
    	(dst)[2]=(src)[2];			\
    	(dst)[3]=(src)[3];			\
        }while(0)
    
    #define SWAP4WORD(word)				\
        do {					\
    	CAMELLIA_SWAP4((word)[0]);		\
    	CAMELLIA_SWAP4((word)[1]);		\
    	CAMELLIA_SWAP4((word)[2]);		\
    	CAMELLIA_SWAP4((word)[3]);		\
        }while(0)
    
    #define XOR4WORD(a, b)/* a = a ^ b */		\
        do {					\
    	(a)[0]^=(b)[0];				\
    	(a)[1]^=(b)[1];				\
    	(a)[2]^=(b)[2];				\
    	(a)[3]^=(b)[3];				\
        }while(0)
    
    #define XOR4WORD2(a, b, c)/* a = b ^ c */	\
        do {					\
    	(a)[0]=(b)[0]^(c)[0];			\
    	(a)[1]=(b)[1]^(c)[1];			\
    	(a)[2]=(b)[2]^(c)[2];			\
    	(a)[3]=(b)[3]^(c)[3];			\
        }while(0)
    
    
    #ifdef L_ENDIAN
    #define CamelliaSubkeyL(INDEX) (subkey[(INDEX)*2 + 1])
    #define CamelliaSubkeyR(INDEX) (subkey[(INDEX)*2])
    #else /* big endian */
    #define CamelliaSubkeyL(INDEX) (subkey[(INDEX)*2])
    #define CamelliaSubkeyR(INDEX) (subkey[(INDEX)*2 + 1])
    #endif /* L_ENDIAN */
    
    /* rotation right shift 1byte */
    #define CAMELLIA_RR8(x) (((x) >> 8) + ((x) << 24))
    /* rotation left shift 1bit */
    #define CAMELLIA_RL1(x) (((x) << 1) + ((x) >> 31))
    /* rotation left shift 1byte */
    #define CAMELLIA_RL8(x) (((x) << 8) + ((x) >> 24))
    
    #define CAMELLIA_ROLDQ(ll, lr, rl, rr, w0, w1, bits)	\
        do {						\
    	w0 = ll;					\
    	ll = (ll << bits) + (lr >> (32 - bits));	\
    	lr = (lr << bits) + (rl >> (32 - bits));	\
    	rl = (rl << bits) + (rr >> (32 - bits));	\
    	rr = (rr << bits) + (w0 >> (32 - bits));	\
        } while(0)
    
    #define CAMELLIA_ROLDQo32(ll, lr, rl, rr, w0, w1, bits)	\
        do {						\
    	w0 = ll;					\
    	w1 = lr;					\
    	ll = (lr << (bits - 32)) + (rl >> (64 - bits));	\
    	lr = (rl << (bits - 32)) + (rr >> (64 - bits));	\
    	rl = (rr << (bits - 32)) + (w0 >> (64 - bits));	\
    	rr = (w0 << (bits - 32)) + (w1 >> (64 - bits));	\
        } while(0)
    
    #define CAMELLIA_SP1110(INDEX) (camellia_sp1110[(INDEX)])
    #define CAMELLIA_SP0222(INDEX) (camellia_sp0222[(INDEX)])
    #define CAMELLIA_SP3033(INDEX) (camellia_sp3033[(INDEX)])
    #define CAMELLIA_SP4404(INDEX) (camellia_sp4404[(INDEX)])

    #define CAMELLIA_F(xl, xr, kl, kr, yl, yr, il, ir, t0, t1)	\
        do {							\
    	il = xl ^ kl;						\
    	ir = xr ^ kr;						\
    	t0 = il >> 16;						\
    	t1 = ir >> 16;						\
    	yl = CAMELLIA_SP1110(ir & 0xff)				\
    	    ^ CAMELLIA_SP0222((t1 >> 8) & 0xff)			\
    	    ^ CAMELLIA_SP3033(t1 & 0xff)			\
    	    ^ CAMELLIA_SP4404((ir >> 8) & 0xff);		\
    	yr = CAMELLIA_SP1110((t0 >> 8) & 0xff)			\
    	    ^ CAMELLIA_SP0222(t0 & 0xff)			\
    	    ^ CAMELLIA_SP3033((il >> 8) & 0xff)			\
    	    ^ CAMELLIA_SP4404(il & 0xff);			\
    	yl ^= yr;						\
    	yr = CAMELLIA_RR8(yr);					\
    	yr ^= yl;						\
        } while(0)


    /*
     * for speed up
     *
     */
    #if !defined(_MSC_VER)

    #define CAMELLIA_FLS(ll, lr, rl, rr, kll, klr, krl, krr, t0, t1, t2, t3) \
        do {								\
    	t0 = kll;							\
    	t2 = krr;							\
    	t0 &= ll;							\
    	t2 |= rr;							\
    	rl ^= t2;							\
    	lr ^= CAMELLIA_RL1(t0);						\
    	t3 = krl;							\
    	t1 = klr;							\
    	t3 &= rl;							\
    	t1 |= lr;							\
    	ll ^= t1;							\
    	rr ^= CAMELLIA_RL1(t3);						\
        } while(0)

    #define CAMELLIA_ROUNDSM(xl, xr, kl, kr, yl, yr, il, ir, t0, t1)	\
        do {								\
    	ir =  CAMELLIA_SP1110(xr & 0xff);				\
    	il =  CAMELLIA_SP1110((xl>>24) & 0xff);				\
    	ir ^= CAMELLIA_SP0222((xr>>24) & 0xff);				\
    	il ^= CAMELLIA_SP0222((xl>>16) & 0xff);				\
    	ir ^= CAMELLIA_SP3033((xr>>16) & 0xff);				\
    	il ^= CAMELLIA_SP3033((xl>>8) & 0xff);				\
    	ir ^= CAMELLIA_SP4404((xr>>8) & 0xff);				\
    	il ^= CAMELLIA_SP4404(xl & 0xff);				\
    	il ^= kl;							\
    	ir ^= il ^ kr;							\
    	yl ^= ir;							\
    	yr ^= CAMELLIA_RR8(il) ^ ir;					\
        } while(0)

    #else /* for MS-VC */

    #define CAMELLIA_FLS(ll, lr, rl, rr, kll, klr, krl, krr, t0, t1, t2, t3) \
        do {								\
    	t0 = kll;							\
    	t0 &= ll;							\
    	lr ^= CAMELLIA_RL1(t0);						\
    	t1 = klr;							\
    	t1 |= lr;							\
    	ll ^= t1;							\
    									\
    	t2 = krr;							\
    	t2 |= rr;							\
    	rl ^= t2;							\
    	t3 = krl;							\
    	t3 &= rl;							\
    	rr ^= CAMELLIA_RL1(t3);						\
        } while(0)

    #define CAMELLIA_ROUNDSM(xl, xr, kl, kr, yl, yr, il, ir, t0, t1)	\
        do {								\
    	il = xl;							\
    	ir = xr;							\
    	t0 = il >> 16;							\
    	t1 = ir >> 16;							\
    	ir = CAMELLIA_SP1110(ir & 0xff)					\
    	    ^ CAMELLIA_SP0222((t1 >> 8) & 0xff)				\
    	    ^ CAMELLIA_SP3033(t1 & 0xff)				\
    	    ^ CAMELLIA_SP4404((ir >> 8) & 0xff);			\
    	il = CAMELLIA_SP1110((t0 >> 8) & 0xff)				\
    	    ^ CAMELLIA_SP0222(t0 & 0xff)				\
    	    ^ CAMELLIA_SP3033((il >> 8) & 0xff)				\
    	    ^ CAMELLIA_SP4404(il & 0xff);				\
    	il ^= kl;							\
    	ir ^= kr;							\
    	ir ^= il;							\
    	il = CAMELLIA_RR8(il);						\
    	il ^= ir;							\
    	yl ^= ir;							\
    	yr ^= il;							\
        } while(0)
    #endif

    static const uint32_t camellia_sp1110[256] = {
        0x70707000,0x82828200,0x2c2c2c00,0xececec00,
        0xb3b3b300,0x27272700,0xc0c0c000,0xe5e5e500,
        0xe4e4e400,0x85858500,0x57575700,0x35353500,
        0xeaeaea00,0x0c0c0c00,0xaeaeae00,0x41414100,
        0x23232300,0xefefef00,0x6b6b6b00,0x93939300,
        0x45454500,0x19191900,0xa5a5a500,0x21212100,
        0xededed00,0x0e0e0e00,0x4f4f4f00,0x4e4e4e00,
        0x1d1d1d00,0x65656500,0x92929200,0xbdbdbd00,
        0x86868600,0xb8b8b800,0xafafaf00,0x8f8f8f00,
        0x7c7c7c00,0xebebeb00,0x1f1f1f00,0xcecece00,
        0x3e3e3e00,0x30303000,0xdcdcdc00,0x5f5f5f00,
        0x5e5e5e00,0xc5c5c500,0x0b0b0b00,0x1a1a1a00,
        0xa6a6a600,0xe1e1e100,0x39393900,0xcacaca00,
        0xd5d5d500,0x47474700,0x5d5d5d00,0x3d3d3d00,
        0xd9d9d900,0x01010100,0x5a5a5a00,0xd6d6d600,
        0x51515100,0x56565600,0x6c6c6c00,0x4d4d4d00,
        0x8b8b8b00,0x0d0d0d00,0x9a9a9a00,0x66666600,
        0xfbfbfb00,0xcccccc00,0xb0b0b000,0x2d2d2d00,
        0x74747400,0x12121200,0x2b2b2b00,0x20202000,
        0xf0f0f000,0xb1b1b100,0x84848400,0x99999900,
        0xdfdfdf00,0x4c4c4c00,0xcbcbcb00,0xc2c2c200,
        0x34343400,0x7e7e7e00,0x76767600,0x05050500,
        0x6d6d6d00,0xb7b7b700,0xa9a9a900,0x31313100,
        0xd1d1d100,0x17171700,0x04040400,0xd7d7d700,
        0x14141400,0x58585800,0x3a3a3a00,0x61616100,
        0xdedede00,0x1b1b1b00,0x11111100,0x1c1c1c00,
        0x32323200,0x0f0f0f00,0x9c9c9c00,0x16161600,
        0x53535300,0x18181800,0xf2f2f200,0x22222200,
        0xfefefe00,0x44444400,0xcfcfcf00,0xb2b2b200,
        0xc3c3c300,0xb5b5b500,0x7a7a7a00,0x91919100,
        0x24242400,0x08080800,0xe8e8e800,0xa8a8a800,
        0x60606000,0xfcfcfc00,0x69696900,0x50505000,
        0xaaaaaa00,0xd0d0d000,0xa0a0a000,0x7d7d7d00,
        0xa1a1a100,0x89898900,0x62626200,0x97979700,
        0x54545400,0x5b5b5b00,0x1e1e1e00,0x95959500,
        0xe0e0e000,0xffffff00,0x64646400,0xd2d2d200,
        0x10101000,0xc4c4c400,0x00000000,0x48484800,
        0xa3a3a300,0xf7f7f700,0x75757500,0xdbdbdb00,
        0x8a8a8a00,0x03030300,0xe6e6e600,0xdadada00,
        0x09090900,0x3f3f3f00,0xdddddd00,0x94949400,
        0x87878700,0x5c5c5c00,0x83838300,0x02020200,
        0xcdcdcd00,0x4a4a4a00,0x90909000,0x33333300,
        0x73737300,0x67676700,0xf6f6f600,0xf3f3f300,
        0x9d9d9d00,0x7f7f7f00,0xbfbfbf00,0xe2e2e200,
        0x52525200,0x9b9b9b00,0xd8d8d800,0x26262600,
        0xc8c8c800,0x37373700,0xc6c6c600,0x3b3b3b00,
        0x81818100,0x96969600,0x6f6f6f00,0x4b4b4b00,
        0x13131300,0xbebebe00,0x63636300,0x2e2e2e00,
        0xe9e9e900,0x79797900,0xa7a7a700,0x8c8c8c00,
        0x9f9f9f00,0x6e6e6e00,0xbcbcbc00,0x8e8e8e00,
        0x29292900,0xf5f5f500,0xf9f9f900,0xb6b6b600,
        0x2f2f2f00,0xfdfdfd00,0xb4b4b400,0x59595900,
        0x78787800,0x98989800,0x06060600,0x6a6a6a00,
        0xe7e7e700,0x46464600,0x71717100,0xbababa00,
        0xd4d4d400,0x25252500,0xababab00,0x42424200,
        0x88888800,0xa2a2a200,0x8d8d8d00,0xfafafa00,
        0x72727200,0x07070700,0xb9b9b900,0x55555500,
        0xf8f8f800,0xeeeeee00,0xacacac00,0x0a0a0a00,
        0x36363600,0x49494900,0x2a2a2a00,0x68686800,
        0x3c3c3c00,0x38383800,0xf1f1f100,0xa4a4a400,
        0x40404000,0x28282800,0xd3d3d300,0x7b7b7b00,
        0xbbbbbb00,0xc9c9c900,0x43434300,0xc1c1c100,
        0x15151500,0xe3e3e300,0xadadad00,0xf4f4f400,
        0x77777700,0xc7c7c700,0x80808000,0x9e9e9e00,
    };

    static const uint32_t camellia_sp0222[256] = {
        0x00e0e0e0,0x00050505,0x00585858,0x00d9d9d9,
        0x00676767,0x004e4e4e,0x00818181,0x00cbcbcb,
        0x00c9c9c9,0x000b0b0b,0x00aeaeae,0x006a6a6a,
        0x00d5d5d5,0x00181818,0x005d5d5d,0x00828282,
        0x00464646,0x00dfdfdf,0x00d6d6d6,0x00272727,
        0x008a8a8a,0x00323232,0x004b4b4b,0x00424242,
        0x00dbdbdb,0x001c1c1c,0x009e9e9e,0x009c9c9c,
        0x003a3a3a,0x00cacaca,0x00252525,0x007b7b7b,
        0x000d0d0d,0x00717171,0x005f5f5f,0x001f1f1f,
        0x00f8f8f8,0x00d7d7d7,0x003e3e3e,0x009d9d9d,
        0x007c7c7c,0x00606060,0x00b9b9b9,0x00bebebe,
        0x00bcbcbc,0x008b8b8b,0x00161616,0x00343434,
        0x004d4d4d,0x00c3c3c3,0x00727272,0x00959595,
        0x00ababab,0x008e8e8e,0x00bababa,0x007a7a7a,
        0x00b3b3b3,0x00020202,0x00b4b4b4,0x00adadad,
        0x00a2a2a2,0x00acacac,0x00d8d8d8,0x009a9a9a,
        0x00171717,0x001a1a1a,0x00353535,0x00cccccc,
        0x00f7f7f7,0x00999999,0x00616161,0x005a5a5a,
        0x00e8e8e8,0x00242424,0x00565656,0x00404040,
        0x00e1e1e1,0x00636363,0x00090909,0x00333333,
        0x00bfbfbf,0x00989898,0x00979797,0x00858585,
        0x00686868,0x00fcfcfc,0x00ececec,0x000a0a0a,
        0x00dadada,0x006f6f6f,0x00535353,0x00626262,
        0x00a3a3a3,0x002e2e2e,0x00080808,0x00afafaf,
        0x00282828,0x00b0b0b0,0x00747474,0x00c2c2c2,
        0x00bdbdbd,0x00363636,0x00222222,0x00383838,
        0x00646464,0x001e1e1e,0x00393939,0x002c2c2c,
        0x00a6a6a6,0x00303030,0x00e5e5e5,0x00444444,
        0x00fdfdfd,0x00888888,0x009f9f9f,0x00656565,
        0x00878787,0x006b6b6b,0x00f4f4f4,0x00232323,
        0x00484848,0x00101010,0x00d1d1d1,0x00515151,
        0x00c0c0c0,0x00f9f9f9,0x00d2d2d2,0x00a0a0a0,
        0x00555555,0x00a1a1a1,0x00414141,0x00fafafa,
        0x00434343,0x00131313,0x00c4c4c4,0x002f2f2f,
        0x00a8a8a8,0x00b6b6b6,0x003c3c3c,0x002b2b2b,
        0x00c1c1c1,0x00ffffff,0x00c8c8c8,0x00a5a5a5,
        0x00202020,0x00898989,0x00000000,0x00909090,
        0x00474747,0x00efefef,0x00eaeaea,0x00b7b7b7,
        0x00151515,0x00060606,0x00cdcdcd,0x00b5b5b5,
        0x00121212,0x007e7e7e,0x00bbbbbb,0x00292929,
        0x000f0f0f,0x00b8b8b8,0x00070707,0x00040404,
        0x009b9b9b,0x00949494,0x00212121,0x00666666,
        0x00e6e6e6,0x00cecece,0x00ededed,0x00e7e7e7,
        0x003b3b3b,0x00fefefe,0x007f7f7f,0x00c5c5c5,
        0x00a4a4a4,0x00373737,0x00b1b1b1,0x004c4c4c,
        0x00919191,0x006e6e6e,0x008d8d8d,0x00767676,
        0x00030303,0x002d2d2d,0x00dedede,0x00969696,
        0x00262626,0x007d7d7d,0x00c6c6c6,0x005c5c5c,
        0x00d3d3d3,0x00f2f2f2,0x004f4f4f,0x00191919,
        0x003f3f3f,0x00dcdcdc,0x00797979,0x001d1d1d,
        0x00525252,0x00ebebeb,0x00f3f3f3,0x006d6d6d,
        0x005e5e5e,0x00fbfbfb,0x00696969,0x00b2b2b2,
        0x00f0f0f0,0x00313131,0x000c0c0c,0x00d4d4d4,
        0x00cfcfcf,0x008c8c8c,0x00e2e2e2,0x00757575,
        0x00a9a9a9,0x004a4a4a,0x00575757,0x00848484,
        0x00111111,0x00454545,0x001b1b1b,0x00f5f5f5,
        0x00e4e4e4,0x000e0e0e,0x00737373,0x00aaaaaa,
        0x00f1f1f1,0x00dddddd,0x00595959,0x00141414,
        0x006c6c6c,0x00929292,0x00545454,0x00d0d0d0,
        0x00787878,0x00707070,0x00e3e3e3,0x00494949,
        0x00808080,0x00505050,0x00a7a7a7,0x00f6f6f6,
        0x00777777,0x00939393,0x00868686,0x00838383,
        0x002a2a2a,0x00c7c7c7,0x005b5b5b,0x00e9e9e9,
        0x00eeeeee,0x008f8f8f,0x00010101,0x003d3d3d,
    };

    static const uint32_t camellia_sp3033[256] = {
        0x38003838,0x41004141,0x16001616,0x76007676,
        0xd900d9d9,0x93009393,0x60006060,0xf200f2f2,
        0x72007272,0xc200c2c2,0xab00abab,0x9a009a9a,
        0x75007575,0x06000606,0x57005757,0xa000a0a0,
        0x91009191,0xf700f7f7,0xb500b5b5,0xc900c9c9,
        0xa200a2a2,0x8c008c8c,0xd200d2d2,0x90009090,
        0xf600f6f6,0x07000707,0xa700a7a7,0x27002727,
        0x8e008e8e,0xb200b2b2,0x49004949,0xde00dede,
        0x43004343,0x5c005c5c,0xd700d7d7,0xc700c7c7,
        0x3e003e3e,0xf500f5f5,0x8f008f8f,0x67006767,
        0x1f001f1f,0x18001818,0x6e006e6e,0xaf00afaf,
        0x2f002f2f,0xe200e2e2,0x85008585,0x0d000d0d,
        0x53005353,0xf000f0f0,0x9c009c9c,0x65006565,
        0xea00eaea,0xa300a3a3,0xae00aeae,0x9e009e9e,
        0xec00ecec,0x80008080,0x2d002d2d,0x6b006b6b,
        0xa800a8a8,0x2b002b2b,0x36003636,0xa600a6a6,
        0xc500c5c5,0x86008686,0x4d004d4d,0x33003333,
        0xfd00fdfd,0x66006666,0x58005858,0x96009696,
        0x3a003a3a,0x09000909,0x95009595,0x10001010,
        0x78007878,0xd800d8d8,0x42004242,0xcc00cccc,
        0xef00efef,0x26002626,0xe500e5e5,0x61006161,
        0x1a001a1a,0x3f003f3f,0x3b003b3b,0x82008282,
        0xb600b6b6,0xdb00dbdb,0xd400d4d4,0x98009898,
        0xe800e8e8,0x8b008b8b,0x02000202,0xeb00ebeb,
        0x0a000a0a,0x2c002c2c,0x1d001d1d,0xb000b0b0,
        0x6f006f6f,0x8d008d8d,0x88008888,0x0e000e0e,
        0x19001919,0x87008787,0x4e004e4e,0x0b000b0b,
        0xa900a9a9,0x0c000c0c,0x79007979,0x11001111,
        0x7f007f7f,0x22002222,0xe700e7e7,0x59005959,
        0xe100e1e1,0xda00dada,0x3d003d3d,0xc800c8c8,
        0x12001212,0x04000404,0x74007474,0x54005454,
        0x30003030,0x7e007e7e,0xb400b4b4,0x28002828,
        0x55005555,0x68006868,0x50005050,0xbe00bebe,
        0xd000d0d0,0xc400c4c4,0x31003131,0xcb00cbcb,
        0x2a002a2a,0xad00adad,0x0f000f0f,0xca00caca,
        0x70007070,0xff00ffff,0x32003232,0x69006969,
        0x08000808,0x62006262,0x00000000,0x24002424,
        0xd100d1d1,0xfb00fbfb,0xba00baba,0xed00eded,
        0x45004545,0x81008181,0x73007373,0x6d006d6d,
        0x84008484,0x9f009f9f,0xee00eeee,0x4a004a4a,
        0xc300c3c3,0x2e002e2e,0xc100c1c1,0x01000101,
        0xe600e6e6,0x25002525,0x48004848,0x99009999,
        0xb900b9b9,0xb300b3b3,0x7b007b7b,0xf900f9f9,
        0xce00cece,0xbf00bfbf,0xdf00dfdf,0x71007171,
        0x29002929,0xcd00cdcd,0x6c006c6c,0x13001313,
        0x64006464,0x9b009b9b,0x63006363,0x9d009d9d,
        0xc000c0c0,0x4b004b4b,0xb700b7b7,0xa500a5a5,
        0x89008989,0x5f005f5f,0xb100b1b1,0x17001717,
        0xf400f4f4,0xbc00bcbc,0xd300d3d3,0x46004646,
        0xcf00cfcf,0x37003737,0x5e005e5e,0x47004747,
        0x94009494,0xfa00fafa,0xfc00fcfc,0x5b005b5b,
        0x97009797,0xfe00fefe,0x5a005a5a,0xac00acac,
        0x3c003c3c,0x4c004c4c,0x03000303,0x35003535,
        0xf300f3f3,0x23002323,0xb800b8b8,0x5d005d5d,
        0x6a006a6a,0x92009292,0xd500d5d5,0x21002121,
        0x44004444,0x51005151,0xc600c6c6,0x7d007d7d,
        0x39003939,0x83008383,0xdc00dcdc,0xaa00aaaa,
        0x7c007c7c,0x77007777,0x56005656,0x05000505,
        0x1b001b1b,0xa400a4a4,0x15001515,0x34003434,
        0x1e001e1e,0x1c001c1c,0xf800f8f8,0x52005252,
        0x20002020,0x14001414,0xe900e9e9,0xbd00bdbd,
        0xdd00dddd,0xe400e4e4,0xa100a1a1,0xe000e0e0,
        0x8a008a8a,0xf100f1f1,0xd600d6d6,0x7a007a7a,
        0xbb00bbbb,0xe300e3e3,0x40004040,0x4f004f4f,
    };

    static const uint32_t camellia_sp4404[256] = {
        0x70700070,0x2c2c002c,0xb3b300b3,0xc0c000c0,
        0xe4e400e4,0x57570057,0xeaea00ea,0xaeae00ae,
        0x23230023,0x6b6b006b,0x45450045,0xa5a500a5,
        0xeded00ed,0x4f4f004f,0x1d1d001d,0x92920092,
        0x86860086,0xafaf00af,0x7c7c007c,0x1f1f001f,
        0x3e3e003e,0xdcdc00dc,0x5e5e005e,0x0b0b000b,
        0xa6a600a6,0x39390039,0xd5d500d5,0x5d5d005d,
        0xd9d900d9,0x5a5a005a,0x51510051,0x6c6c006c,
        0x8b8b008b,0x9a9a009a,0xfbfb00fb,0xb0b000b0,
        0x74740074,0x2b2b002b,0xf0f000f0,0x84840084,
        0xdfdf00df,0xcbcb00cb,0x34340034,0x76760076,
        0x6d6d006d,0xa9a900a9,0xd1d100d1,0x04040004,
        0x14140014,0x3a3a003a,0xdede00de,0x11110011,
        0x32320032,0x9c9c009c,0x53530053,0xf2f200f2,
        0xfefe00fe,0xcfcf00cf,0xc3c300c3,0x7a7a007a,
        0x24240024,0xe8e800e8,0x60600060,0x69690069,
        0xaaaa00aa,0xa0a000a0,0xa1a100a1,0x62620062,
        0x54540054,0x1e1e001e,0xe0e000e0,0x64640064,
        0x10100010,0x00000000,0xa3a300a3,0x75750075,
        0x8a8a008a,0xe6e600e6,0x09090009,0xdddd00dd,
        0x87870087,0x83830083,0xcdcd00cd,0x90900090,
        0x73730073,0xf6f600f6,0x9d9d009d,0xbfbf00bf,
        0x52520052,0xd8d800d8,0xc8c800c8,0xc6c600c6,
        0x81810081,0x6f6f006f,0x13130013,0x63630063,
        0xe9e900e9,0xa7a700a7,0x9f9f009f,0xbcbc00bc,
        0x29290029,0xf9f900f9,0x2f2f002f,0xb4b400b4,
        0x78780078,0x06060006,0xe7e700e7,0x71710071,
        0xd4d400d4,0xabab00ab,0x88880088,0x8d8d008d,
        0x72720072,0xb9b900b9,0xf8f800f8,0xacac00ac,
        0x36360036,0x2a2a002a,0x3c3c003c,0xf1f100f1,
        0x40400040,0xd3d300d3,0xbbbb00bb,0x43430043,
        0x15150015,0xadad00ad,0x77770077,0x80800080,
        0x82820082,0xecec00ec,0x27270027,0xe5e500e5,
        0x85850085,0x35350035,0x0c0c000c,0x41410041,
        0xefef00ef,0x93930093,0x19190019,0x21210021,
        0x0e0e000e,0x4e4e004e,0x65650065,0xbdbd00bd,
        0xb8b800b8,0x8f8f008f,0xebeb00eb,0xcece00ce,
        0x30300030,0x5f5f005f,0xc5c500c5,0x1a1a001a,
        0xe1e100e1,0xcaca00ca,0x47470047,0x3d3d003d,
        0x01010001,0xd6d600d6,0x56560056,0x4d4d004d,
        0x0d0d000d,0x66660066,0xcccc00cc,0x2d2d002d,
        0x12120012,0x20200020,0xb1b100b1,0x99990099,
        0x4c4c004c,0xc2c200c2,0x7e7e007e,0x05050005,
        0xb7b700b7,0x31310031,0x17170017,0xd7d700d7,
        0x58580058,0x61610061,0x1b1b001b,0x1c1c001c,
        0x0f0f000f,0x16160016,0x18180018,0x22220022,
        0x44440044,0xb2b200b2,0xb5b500b5,0x91910091,
        0x08080008,0xa8a800a8,0xfcfc00fc,0x50500050,
        0xd0d000d0,0x7d7d007d,0x89890089,0x97970097,
        0x5b5b005b,0x95950095,0xffff00ff,0xd2d200d2,
        0xc4c400c4,0x48480048,0xf7f700f7,0xdbdb00db,
        0x03030003,0xdada00da,0x3f3f003f,0x94940094,
        0x5c5c005c,0x02020002,0x4a4a004a,0x33330033,
        0x67670067,0xf3f300f3,0x7f7f007f,0xe2e200e2,
        0x9b9b009b,0x26260026,0x37370037,0x3b3b003b,
        0x96960096,0x4b4b004b,0xbebe00be,0x2e2e002e,
        0x79790079,0x8c8c008c,0x6e6e006e,0x8e8e008e,
        0xf5f500f5,0xb6b600b6,0xfdfd00fd,0x59590059,
        0x98980098,0x6a6a006a,0x46460046,0xbaba00ba,
        0x25250025,0x42420042,0xa2a200a2,0xfafa00fa,
        0x07070007,0x55550055,0xeeee00ee,0x0a0a000a,
        0x49490049,0x68680068,0x38380038,0xa4a400a4,
        0x28280028,0x7b7b007b,0xc9c900c9,0xc1c100c1,
        0xe3e300e3,0xf4f400f4,0xc7c700c7,0x9e9e009e,
    };


    /**
     * Stuff related to the Camellia key schedule
     */
    #define subl(x) subL[(x)]
    #define subr(x) subR[(x)]

    static void camellia_setup128(const unsigned char *key, uint32_t *subkey)
    {
        uint32_t kll, klr, krl, krr;
        uint32_t il, ir, t0, t1, w0, w1;
        uint32_t kw4l, kw4r, dw, tl, tr;
        uint32_t subL[26];
        uint32_t subR[26];

        /**
         *  k == kll || klr || krl || krr (|| is concatination)
         */
        kll = GETU32(key     );
        klr = GETU32(key +  4);
        krl = GETU32(key +  8);
        krr = GETU32(key + 12);
        /**
         * generate KL dependent subkeys
         */
        /* kw1 */
        subl(0) = kll; subr(0) = klr;
        /* kw2 */
        subl(1) = krl; subr(1) = krr;
        /* rotation left shift 15bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k3 */
        subl(4) = kll; subr(4) = klr;
        /* k4 */
        subl(5) = krl; subr(5) = krr;
        /* rotation left shift 15+30bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 30);
        /* k7 */
        subl(10) = kll; subr(10) = klr;
        /* k8 */
        subl(11) = krl; subr(11) = krr;
        /* rotation left shift 15+30+15bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k10 */
        subl(13) = krl; subr(13) = krr;
        /* rotation left shift 15+30+15+17 bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 17);
        /* kl3 */
        subl(16) = kll; subr(16) = klr;
        /* kl4 */
        subl(17) = krl; subr(17) = krr;
        /* rotation left shift 15+30+15+17+17 bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 17);
        /* k13 */
        subl(18) = kll; subr(18) = klr;
        /* k14 */
        subl(19) = krl; subr(19) = krr;
        /* rotation left shift 15+30+15+17+17+17 bit */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 17);
        /* k17 */
        subl(22) = kll; subr(22) = klr;
        /* k18 */
        subl(23) = krl; subr(23) = krr;

        /* generate KA */
        kll = subl(0); klr = subr(0);
        krl = subl(1); krr = subr(1);
        CAMELLIA_F(kll, klr,
    	       CAMELLIA_SIGMA1L, CAMELLIA_SIGMA1R,
    	       w0, w1, il, ir, t0, t1);
        krl ^= w0; krr ^= w1;
        CAMELLIA_F(krl, krr,
    	       CAMELLIA_SIGMA2L, CAMELLIA_SIGMA2R,
    	       kll, klr, il, ir, t0, t1);
        /* current status == (kll, klr, w0, w1) */
        CAMELLIA_F(kll, klr,
    	       CAMELLIA_SIGMA3L, CAMELLIA_SIGMA3R,
    	       krl, krr, il, ir, t0, t1);
        krl ^= w0; krr ^= w1;
        CAMELLIA_F(krl, krr,
    	       CAMELLIA_SIGMA4L, CAMELLIA_SIGMA4R,
    	       w0, w1, il, ir, t0, t1);
        kll ^= w0; klr ^= w1;

        /* generate KA dependent subkeys */
        /* k1, k2 */
        subl(2) = kll; subr(2) = klr;
        subl(3) = krl; subr(3) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k5,k6 */
        subl(6) = kll; subr(6) = klr;
        subl(7) = krl; subr(7) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* kl1, kl2 */
        subl(8) = kll; subr(8) = klr;
        subl(9) = krl; subr(9) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k9 */
        subl(12) = kll; subr(12) = klr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k11, k12 */
        subl(14) = kll; subr(14) = klr;
        subl(15) = krl; subr(15) = krr;
        CAMELLIA_ROLDQo32(kll, klr, krl, krr, w0, w1, 34);
        /* k15, k16 */
        subl(20) = kll; subr(20) = klr;
        subl(21) = krl; subr(21) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 17);
        /* kw3, kw4 */
        subl(24) = kll; subr(24) = klr;
        subl(25) = krl; subr(25) = krr;


        /* absorb kw2 to other subkeys */
    /* round 2 */
        subl(3) ^= subl(1); subr(3) ^= subr(1);
    /* round 4 */
        subl(5) ^= subl(1); subr(5) ^= subr(1);
    /* round 6 */
        subl(7) ^= subl(1); subr(7) ^= subr(1);
        subl(1) ^= subr(1) & ~subr(9);
        dw = subl(1) & subl(9),
    	subr(1) ^= CAMELLIA_RL1(dw); /* modified for FLinv(kl2) */
    /* round 8 */
        subl(11) ^= subl(1); subr(11) ^= subr(1);
    /* round 10 */
        subl(13) ^= subl(1); subr(13) ^= subr(1);
    /* round 12 */
        subl(15) ^= subl(1); subr(15) ^= subr(1);
        subl(1) ^= subr(1) & ~subr(17);
        dw = subl(1) & subl(17),
    	subr(1) ^= CAMELLIA_RL1(dw); /* modified for FLinv(kl4) */
    /* round 14 */
        subl(19) ^= subl(1); subr(19) ^= subr(1);
    /* round 16 */
        subl(21) ^= subl(1); subr(21) ^= subr(1);
    /* round 18 */
        subl(23) ^= subl(1); subr(23) ^= subr(1);
    /* kw3 */
        subl(24) ^= subl(1); subr(24) ^= subr(1);

        /* absorb kw4 to other subkeys */
        kw4l = subl(25); kw4r = subr(25);
    /* round 17 */
        subl(22) ^= kw4l; subr(22) ^= kw4r;
    /* round 15 */
        subl(20) ^= kw4l; subr(20) ^= kw4r;
    /* round 13 */
        subl(18) ^= kw4l; subr(18) ^= kw4r;
        kw4l ^= kw4r & ~subr(16);
        dw = kw4l & subl(16),
    	kw4r ^= CAMELLIA_RL1(dw); /* modified for FL(kl3) */
    /* round 11 */
        subl(14) ^= kw4l; subr(14) ^= kw4r;
    /* round 9 */
        subl(12) ^= kw4l; subr(12) ^= kw4r;
    /* round 7 */
        subl(10) ^= kw4l; subr(10) ^= kw4r;
        kw4l ^= kw4r & ~subr(8);
        dw = kw4l & subl(8),
    	kw4r ^= CAMELLIA_RL1(dw); /* modified for FL(kl1) */
    /* round 5 */
        subl(6) ^= kw4l; subr(6) ^= kw4r;
    /* round 3 */
        subl(4) ^= kw4l; subr(4) ^= kw4r;
    /* round 1 */
        subl(2) ^= kw4l; subr(2) ^= kw4r;
    /* kw1 */
        subl(0) ^= kw4l; subr(0) ^= kw4r;


        /* key XOR is end of F-function */
        CamelliaSubkeyL(0) = subl(0) ^ subl(2);/* kw1 */
        CamelliaSubkeyR(0) = subr(0) ^ subr(2);
        CamelliaSubkeyL(2) = subl(3);       /* round 1 */
        CamelliaSubkeyR(2) = subr(3);
        CamelliaSubkeyL(3) = subl(2) ^ subl(4); /* round 2 */
        CamelliaSubkeyR(3) = subr(2) ^ subr(4);
        CamelliaSubkeyL(4) = subl(3) ^ subl(5); /* round 3 */
        CamelliaSubkeyR(4) = subr(3) ^ subr(5);
        CamelliaSubkeyL(5) = subl(4) ^ subl(6); /* round 4 */
        CamelliaSubkeyR(5) = subr(4) ^ subr(6);
        CamelliaSubkeyL(6) = subl(5) ^ subl(7); /* round 5 */
        CamelliaSubkeyR(6) = subr(5) ^ subr(7);
        tl = subl(10) ^ (subr(10) & ~subr(8));
        dw = tl & subl(8),  /* FL(kl1) */
    	tr = subr(10) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(7) = subl(6) ^ tl; /* round 6 */
        CamelliaSubkeyR(7) = subr(6) ^ tr;
        CamelliaSubkeyL(8) = subl(8);       /* FL(kl1) */
        CamelliaSubkeyR(8) = subr(8);
        CamelliaSubkeyL(9) = subl(9);       /* FLinv(kl2) */
        CamelliaSubkeyR(9) = subr(9);
        tl = subl(7) ^ (subr(7) & ~subr(9));
        dw = tl & subl(9),  /* FLinv(kl2) */
    	tr = subr(7) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(10) = tl ^ subl(11); /* round 7 */
        CamelliaSubkeyR(10) = tr ^ subr(11);
        CamelliaSubkeyL(11) = subl(10) ^ subl(12); /* round 8 */
        CamelliaSubkeyR(11) = subr(10) ^ subr(12);
        CamelliaSubkeyL(12) = subl(11) ^ subl(13); /* round 9 */
        CamelliaSubkeyR(12) = subr(11) ^ subr(13);
        CamelliaSubkeyL(13) = subl(12) ^ subl(14); /* round 10 */
        CamelliaSubkeyR(13) = subr(12) ^ subr(14);
        CamelliaSubkeyL(14) = subl(13) ^ subl(15); /* round 11 */
        CamelliaSubkeyR(14) = subr(13) ^ subr(15);
        tl = subl(18) ^ (subr(18) & ~subr(16));
        dw = tl & subl(16), /* FL(kl3) */
    	tr = subr(18) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(15) = subl(14) ^ tl; /* round 12 */
        CamelliaSubkeyR(15) = subr(14) ^ tr;
        CamelliaSubkeyL(16) = subl(16);     /* FL(kl3) */
        CamelliaSubkeyR(16) = subr(16);
        CamelliaSubkeyL(17) = subl(17);     /* FLinv(kl4) */
        CamelliaSubkeyR(17) = subr(17);
        tl = subl(15) ^ (subr(15) & ~subr(17));
        dw = tl & subl(17), /* FLinv(kl4) */
    	tr = subr(15) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(18) = tl ^ subl(19); /* round 13 */
        CamelliaSubkeyR(18) = tr ^ subr(19);
        CamelliaSubkeyL(19) = subl(18) ^ subl(20); /* round 14 */
        CamelliaSubkeyR(19) = subr(18) ^ subr(20);
        CamelliaSubkeyL(20) = subl(19) ^ subl(21); /* round 15 */
        CamelliaSubkeyR(20) = subr(19) ^ subr(21);
        CamelliaSubkeyL(21) = subl(20) ^ subl(22); /* round 16 */
        CamelliaSubkeyR(21) = subr(20) ^ subr(22);
        CamelliaSubkeyL(22) = subl(21) ^ subl(23); /* round 17 */
        CamelliaSubkeyR(22) = subr(21) ^ subr(23);
        CamelliaSubkeyL(23) = subl(22);     /* round 18 */
        CamelliaSubkeyR(23) = subr(22);
        CamelliaSubkeyL(24) = subl(24) ^ subl(23); /* kw3 */
        CamelliaSubkeyR(24) = subr(24) ^ subr(23);

        /* apply the inverse of the last half of P-function */
        dw = CamelliaSubkeyL(2) ^ CamelliaSubkeyR(2),
    	dw = CAMELLIA_RL8(dw);/* round 1 */
        CamelliaSubkeyR(2) = CamelliaSubkeyL(2) ^ dw,
    	CamelliaSubkeyL(2) = dw;
        dw = CamelliaSubkeyL(3) ^ CamelliaSubkeyR(3),
    	dw = CAMELLIA_RL8(dw);/* round 2 */
        CamelliaSubkeyR(3) = CamelliaSubkeyL(3) ^ dw,
    	CamelliaSubkeyL(3) = dw;
        dw = CamelliaSubkeyL(4) ^ CamelliaSubkeyR(4),
    	dw = CAMELLIA_RL8(dw);/* round 3 */
        CamelliaSubkeyR(4) = CamelliaSubkeyL(4) ^ dw,
    	CamelliaSubkeyL(4) = dw;
        dw = CamelliaSubkeyL(5) ^ CamelliaSubkeyR(5),
    	dw = CAMELLIA_RL8(dw);/* round 4 */
        CamelliaSubkeyR(5) = CamelliaSubkeyL(5) ^ dw,
    	CamelliaSubkeyL(5) = dw;
        dw = CamelliaSubkeyL(6) ^ CamelliaSubkeyR(6),
    	dw = CAMELLIA_RL8(dw);/* round 5 */
        CamelliaSubkeyR(6) = CamelliaSubkeyL(6) ^ dw,
    	CamelliaSubkeyL(6) = dw;
        dw = CamelliaSubkeyL(7) ^ CamelliaSubkeyR(7),
    	dw = CAMELLIA_RL8(dw);/* round 6 */
        CamelliaSubkeyR(7) = CamelliaSubkeyL(7) ^ dw,
    	CamelliaSubkeyL(7) = dw;
        dw = CamelliaSubkeyL(10) ^ CamelliaSubkeyR(10),
    	dw = CAMELLIA_RL8(dw);/* round 7 */
        CamelliaSubkeyR(10) = CamelliaSubkeyL(10) ^ dw,
    	CamelliaSubkeyL(10) = dw;
        dw = CamelliaSubkeyL(11) ^ CamelliaSubkeyR(11),
    	dw = CAMELLIA_RL8(dw);/* round 8 */
        CamelliaSubkeyR(11) = CamelliaSubkeyL(11) ^ dw,
    	CamelliaSubkeyL(11) = dw;
        dw = CamelliaSubkeyL(12) ^ CamelliaSubkeyR(12),
    	dw = CAMELLIA_RL8(dw);/* round 9 */
        CamelliaSubkeyR(12) = CamelliaSubkeyL(12) ^ dw,
    	CamelliaSubkeyL(12) = dw;
        dw = CamelliaSubkeyL(13) ^ CamelliaSubkeyR(13),
    	dw = CAMELLIA_RL8(dw);/* round 10 */
        CamelliaSubkeyR(13) = CamelliaSubkeyL(13) ^ dw,
    	CamelliaSubkeyL(13) = dw;
        dw = CamelliaSubkeyL(14) ^ CamelliaSubkeyR(14),
    	dw = CAMELLIA_RL8(dw);/* round 11 */
        CamelliaSubkeyR(14) = CamelliaSubkeyL(14) ^ dw,
    	CamelliaSubkeyL(14) = dw;
        dw = CamelliaSubkeyL(15) ^ CamelliaSubkeyR(15),
    	dw = CAMELLIA_RL8(dw);/* round 12 */
        CamelliaSubkeyR(15) = CamelliaSubkeyL(15) ^ dw,
    	CamelliaSubkeyL(15) = dw;
        dw = CamelliaSubkeyL(18) ^ CamelliaSubkeyR(18),
    	dw = CAMELLIA_RL8(dw);/* round 13 */
        CamelliaSubkeyR(18) = CamelliaSubkeyL(18) ^ dw,
    	CamelliaSubkeyL(18) = dw;
        dw = CamelliaSubkeyL(19) ^ CamelliaSubkeyR(19),
    	dw = CAMELLIA_RL8(dw);/* round 14 */
        CamelliaSubkeyR(19) = CamelliaSubkeyL(19) ^ dw,
    	CamelliaSubkeyL(19) = dw;
        dw = CamelliaSubkeyL(20) ^ CamelliaSubkeyR(20),
    	dw = CAMELLIA_RL8(dw);/* round 15 */
        CamelliaSubkeyR(20) = CamelliaSubkeyL(20) ^ dw,
    	CamelliaSubkeyL(20) = dw;
        dw = CamelliaSubkeyL(21) ^ CamelliaSubkeyR(21),
    	dw = CAMELLIA_RL8(dw);/* round 16 */
        CamelliaSubkeyR(21) = CamelliaSubkeyL(21) ^ dw,
    	CamelliaSubkeyL(21) = dw;
        dw = CamelliaSubkeyL(22) ^ CamelliaSubkeyR(22),
    	dw = CAMELLIA_RL8(dw);/* round 17 */
        CamelliaSubkeyR(22) = CamelliaSubkeyL(22) ^ dw,
    	CamelliaSubkeyL(22) = dw;
        dw = CamelliaSubkeyL(23) ^ CamelliaSubkeyR(23),
    	dw = CAMELLIA_RL8(dw);/* round 18 */
        CamelliaSubkeyR(23) = CamelliaSubkeyL(23) ^ dw,
    	CamelliaSubkeyL(23) = dw;

        return;
    }

    static void camellia_setup256(const unsigned char *key, uint32_t *subkey)
    {
        uint32_t kll,klr,krl,krr;           /* left half of key */
        uint32_t krll,krlr,krrl,krrr;       /* right half of key */
        uint32_t il, ir, t0, t1, w0, w1;    /* temporary variables */
        uint32_t kw4l, kw4r, dw, tl, tr;
        uint32_t subL[34];
        uint32_t subR[34];

        /**
         *  key = (kll || klr || krl || krr || krll || krlr || krrl || krrr)
         *  (|| is concatination)
         */

        kll  = GETU32(key     );
        klr  = GETU32(key +  4);
        krl  = GETU32(key +  8);
        krr  = GETU32(key + 12);
        krll = GETU32(key + 16);
        krlr = GETU32(key + 20);
        krrl = GETU32(key + 24);
        krrr = GETU32(key + 28);

        /* generate KL dependent subkeys */
        /* kw1 */
        subl(0) = kll; subr(0) = klr;
        /* kw2 */
        subl(1) = krl; subr(1) = krr;
        CAMELLIA_ROLDQo32(kll, klr, krl, krr, w0, w1, 45);
        /* k9 */
        subl(12) = kll; subr(12) = klr;
        /* k10 */
        subl(13) = krl; subr(13) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* kl3 */
        subl(16) = kll; subr(16) = klr;
        /* kl4 */
        subl(17) = krl; subr(17) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 17);
        /* k17 */
        subl(22) = kll; subr(22) = klr;
        /* k18 */
        subl(23) = krl; subr(23) = krr;
        CAMELLIA_ROLDQo32(kll, klr, krl, krr, w0, w1, 34);
        /* k23 */
        subl(30) = kll; subr(30) = klr;
        /* k24 */
        subl(31) = krl; subr(31) = krr;

        /* generate KR dependent subkeys */
        CAMELLIA_ROLDQ(krll, krlr, krrl, krrr, w0, w1, 15);
        /* k3 */
        subl(4) = krll; subr(4) = krlr;
        /* k4 */
        subl(5) = krrl; subr(5) = krrr;
        CAMELLIA_ROLDQ(krll, krlr, krrl, krrr, w0, w1, 15);
        /* kl1 */
        subl(8) = krll; subr(8) = krlr;
        /* kl2 */
        subl(9) = krrl; subr(9) = krrr;
        CAMELLIA_ROLDQ(krll, krlr, krrl, krrr, w0, w1, 30);
        /* k13 */
        subl(18) = krll; subr(18) = krlr;
        /* k14 */
        subl(19) = krrl; subr(19) = krrr;
        CAMELLIA_ROLDQo32(krll, krlr, krrl, krrr, w0, w1, 34);
        /* k19 */
        subl(26) = krll; subr(26) = krlr;
        /* k20 */
        subl(27) = krrl; subr(27) = krrr;
        CAMELLIA_ROLDQo32(krll, krlr, krrl, krrr, w0, w1, 34);

        /* generate KA */
        kll = subl(0) ^ krll; klr = subr(0) ^ krlr;
        krl = subl(1) ^ krrl; krr = subr(1) ^ krrr;
        CAMELLIA_F(kll, klr,
    	       CAMELLIA_SIGMA1L, CAMELLIA_SIGMA1R,
    	       w0, w1, il, ir, t0, t1);
        krl ^= w0; krr ^= w1;
        CAMELLIA_F(krl, krr,
    	       CAMELLIA_SIGMA2L, CAMELLIA_SIGMA2R,
    	       kll, klr, il, ir, t0, t1);
        kll ^= krll; klr ^= krlr;
        CAMELLIA_F(kll, klr,
    	       CAMELLIA_SIGMA3L, CAMELLIA_SIGMA3R,
    	       krl, krr, il, ir, t0, t1);
        krl ^= w0 ^ krrl; krr ^= w1 ^ krrr;
        CAMELLIA_F(krl, krr,
    	       CAMELLIA_SIGMA4L, CAMELLIA_SIGMA4R,
    	       w0, w1, il, ir, t0, t1);
        kll ^= w0; klr ^= w1;

        /* generate KB */
        krll ^= kll; krlr ^= klr;
        krrl ^= krl; krrr ^= krr;
        CAMELLIA_F(krll, krlr,
    	       CAMELLIA_SIGMA5L, CAMELLIA_SIGMA5R,
    	       w0, w1, il, ir, t0, t1);
        krrl ^= w0; krrr ^= w1;
        CAMELLIA_F(krrl, krrr,
    	       CAMELLIA_SIGMA6L, CAMELLIA_SIGMA6R,
    	       w0, w1, il, ir, t0, t1);
        krll ^= w0; krlr ^= w1;

        /* generate KA dependent subkeys */
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 15);
        /* k5 */
        subl(6) = kll; subr(6) = klr;
        /* k6 */
        subl(7) = krl; subr(7) = krr;
        CAMELLIA_ROLDQ(kll, klr, krl, krr, w0, w1, 30);
        /* k11 */
        subl(14) = kll; subr(14) = klr;
        /* k12 */
        subl(15) = krl; subr(15) = krr;
        /* rotation left shift 32bit */
        /* kl5 */
        subl(24) = klr; subr(24) = krl;
        /* kl6 */
        subl(25) = krr; subr(25) = kll;
        /* rotation left shift 49 from k11,k12 -> k21,k22 */
        CAMELLIA_ROLDQo32(kll, klr, krl, krr, w0, w1, 49);
        /* k21 */
        subl(28) = kll; subr(28) = klr;
        /* k22 */
        subl(29) = krl; subr(29) = krr;

        /* generate KB dependent subkeys */
        /* k1 */
        subl(2) = krll; subr(2) = krlr;
        /* k2 */
        subl(3) = krrl; subr(3) = krrr;
        CAMELLIA_ROLDQ(krll, krlr, krrl, krrr, w0, w1, 30);
        /* k7 */
        subl(10) = krll; subr(10) = krlr;
        /* k8 */
        subl(11) = krrl; subr(11) = krrr;
        CAMELLIA_ROLDQ(krll, krlr, krrl, krrr, w0, w1, 30);
        /* k15 */
        subl(20) = krll; subr(20) = krlr;
        /* k16 */
        subl(21) = krrl; subr(21) = krrr;
        CAMELLIA_ROLDQo32(krll, krlr, krrl, krrr, w0, w1, 51);
        /* kw3 */
        subl(32) = krll; subr(32) = krlr;
        /* kw4 */
        subl(33) = krrl; subr(33) = krrr;

        /* absorb kw2 to other subkeys */
    /* round 2 */
        subl(3) ^= subl(1); subr(3) ^= subr(1);
    /* round 4 */
        subl(5) ^= subl(1); subr(5) ^= subr(1);
    /* round 6 */
        subl(7) ^= subl(1); subr(7) ^= subr(1);
        subl(1) ^= subr(1) & ~subr(9);
        dw = subl(1) & subl(9),
    	subr(1) ^= CAMELLIA_RL1(dw); /* modified for FLinv(kl2) */
    /* round 8 */
        subl(11) ^= subl(1); subr(11) ^= subr(1);
    /* round 10 */
        subl(13) ^= subl(1); subr(13) ^= subr(1);
    /* round 12 */
        subl(15) ^= subl(1); subr(15) ^= subr(1);
        subl(1) ^= subr(1) & ~subr(17);
        dw = subl(1) & subl(17),
    	subr(1) ^= CAMELLIA_RL1(dw); /* modified for FLinv(kl4) */
    /* round 14 */
        subl(19) ^= subl(1); subr(19) ^= subr(1);
    /* round 16 */
        subl(21) ^= subl(1); subr(21) ^= subr(1);
    /* round 18 */
        subl(23) ^= subl(1); subr(23) ^= subr(1);
        subl(1) ^= subr(1) & ~subr(25);
        dw = subl(1) & subl(25),
    	subr(1) ^= CAMELLIA_RL1(dw); /* modified for FLinv(kl6) */
    /* round 20 */
        subl(27) ^= subl(1); subr(27) ^= subr(1);
    /* round 22 */
        subl(29) ^= subl(1); subr(29) ^= subr(1);
    /* round 24 */
        subl(31) ^= subl(1); subr(31) ^= subr(1);
    /* kw3 */
        subl(32) ^= subl(1); subr(32) ^= subr(1);


        /* absorb kw4 to other subkeys */
        kw4l = subl(33); kw4r = subr(33);
    /* round 23 */
        subl(30) ^= kw4l; subr(30) ^= kw4r;
    /* round 21 */
        subl(28) ^= kw4l; subr(28) ^= kw4r;
    /* round 19 */
        subl(26) ^= kw4l; subr(26) ^= kw4r;
        kw4l ^= kw4r & ~subr(24);
        dw = kw4l & subl(24),
    	kw4r ^= CAMELLIA_RL1(dw); /* modified for FL(kl5) */
    /* round 17 */
        subl(22) ^= kw4l; subr(22) ^= kw4r;
    /* round 15 */
        subl(20) ^= kw4l; subr(20) ^= kw4r;
    /* round 13 */
        subl(18) ^= kw4l; subr(18) ^= kw4r;
        kw4l ^= kw4r & ~subr(16);
        dw = kw4l & subl(16),
    	kw4r ^= CAMELLIA_RL1(dw); /* modified for FL(kl3) */
    /* round 11 */
        subl(14) ^= kw4l; subr(14) ^= kw4r;
    /* round 9 */
        subl(12) ^= kw4l; subr(12) ^= kw4r;
    /* round 7 */
        subl(10) ^= kw4l; subr(10) ^= kw4r;
        kw4l ^= kw4r & ~subr(8);
        dw = kw4l & subl(8),
    	kw4r ^= CAMELLIA_RL1(dw); /* modified for FL(kl1) */
    /* round 5 */
        subl(6) ^= kw4l; subr(6) ^= kw4r;
    /* round 3 */
        subl(4) ^= kw4l; subr(4) ^= kw4r;
    /* round 1 */
        subl(2) ^= kw4l; subr(2) ^= kw4r;
    /* kw1 */
        subl(0) ^= kw4l; subr(0) ^= kw4r;

        /* key XOR is end of F-function */
        CamelliaSubkeyL(0) = subl(0) ^ subl(2);/* kw1 */
        CamelliaSubkeyR(0) = subr(0) ^ subr(2);
        CamelliaSubkeyL(2) = subl(3);       /* round 1 */
        CamelliaSubkeyR(2) = subr(3);
        CamelliaSubkeyL(3) = subl(2) ^ subl(4); /* round 2 */
        CamelliaSubkeyR(3) = subr(2) ^ subr(4);
        CamelliaSubkeyL(4) = subl(3) ^ subl(5); /* round 3 */
        CamelliaSubkeyR(4) = subr(3) ^ subr(5);
        CamelliaSubkeyL(5) = subl(4) ^ subl(6); /* round 4 */
        CamelliaSubkeyR(5) = subr(4) ^ subr(6);
        CamelliaSubkeyL(6) = subl(5) ^ subl(7); /* round 5 */
        CamelliaSubkeyR(6) = subr(5) ^ subr(7);
        tl = subl(10) ^ (subr(10) & ~subr(8));
        dw = tl & subl(8),  /* FL(kl1) */
    	tr = subr(10) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(7) = subl(6) ^ tl; /* round 6 */
        CamelliaSubkeyR(7) = subr(6) ^ tr;
        CamelliaSubkeyL(8) = subl(8);       /* FL(kl1) */
        CamelliaSubkeyR(8) = subr(8);
        CamelliaSubkeyL(9) = subl(9);       /* FLinv(kl2) */
        CamelliaSubkeyR(9) = subr(9);
        tl = subl(7) ^ (subr(7) & ~subr(9));
        dw = tl & subl(9),  /* FLinv(kl2) */
    	tr = subr(7) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(10) = tl ^ subl(11); /* round 7 */
        CamelliaSubkeyR(10) = tr ^ subr(11);
        CamelliaSubkeyL(11) = subl(10) ^ subl(12); /* round 8 */
        CamelliaSubkeyR(11) = subr(10) ^ subr(12);
        CamelliaSubkeyL(12) = subl(11) ^ subl(13); /* round 9 */
        CamelliaSubkeyR(12) = subr(11) ^ subr(13);
        CamelliaSubkeyL(13) = subl(12) ^ subl(14); /* round 10 */
        CamelliaSubkeyR(13) = subr(12) ^ subr(14);
        CamelliaSubkeyL(14) = subl(13) ^ subl(15); /* round 11 */
        CamelliaSubkeyR(14) = subr(13) ^ subr(15);
        tl = subl(18) ^ (subr(18) & ~subr(16));
        dw = tl & subl(16), /* FL(kl3) */
    	tr = subr(18) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(15) = subl(14) ^ tl; /* round 12 */
        CamelliaSubkeyR(15) = subr(14) ^ tr;
        CamelliaSubkeyL(16) = subl(16);     /* FL(kl3) */
        CamelliaSubkeyR(16) = subr(16);
        CamelliaSubkeyL(17) = subl(17);     /* FLinv(kl4) */
        CamelliaSubkeyR(17) = subr(17);
        tl = subl(15) ^ (subr(15) & ~subr(17));
        dw = tl & subl(17), /* FLinv(kl4) */
    	tr = subr(15) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(18) = tl ^ subl(19); /* round 13 */
        CamelliaSubkeyR(18) = tr ^ subr(19);
        CamelliaSubkeyL(19) = subl(18) ^ subl(20); /* round 14 */
        CamelliaSubkeyR(19) = subr(18) ^ subr(20);
        CamelliaSubkeyL(20) = subl(19) ^ subl(21); /* round 15 */
        CamelliaSubkeyR(20) = subr(19) ^ subr(21);
        CamelliaSubkeyL(21) = subl(20) ^ subl(22); /* round 16 */
        CamelliaSubkeyR(21) = subr(20) ^ subr(22);
        CamelliaSubkeyL(22) = subl(21) ^ subl(23); /* round 17 */
        CamelliaSubkeyR(22) = subr(21) ^ subr(23);
        tl = subl(26) ^ (subr(26)
    		     & ~subr(24));
        dw = tl & subl(24), /* FL(kl5) */
    	tr = subr(26) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(23) = subl(22) ^ tl; /* round 18 */
        CamelliaSubkeyR(23) = subr(22) ^ tr;
        CamelliaSubkeyL(24) = subl(24);     /* FL(kl5) */
        CamelliaSubkeyR(24) = subr(24);
        CamelliaSubkeyL(25) = subl(25);     /* FLinv(kl6) */
        CamelliaSubkeyR(25) = subr(25);
        tl = subl(23) ^ (subr(23) &
    		     ~subr(25));
        dw = tl & subl(25), /* FLinv(kl6) */
    	tr = subr(23) ^ CAMELLIA_RL1(dw);
        CamelliaSubkeyL(26) = tl ^ subl(27); /* round 19 */
        CamelliaSubkeyR(26) = tr ^ subr(27);
        CamelliaSubkeyL(27) = subl(26) ^ subl(28); /* round 20 */
        CamelliaSubkeyR(27) = subr(26) ^ subr(28);
        CamelliaSubkeyL(28) = subl(27) ^ subl(29); /* round 21 */
        CamelliaSubkeyR(28) = subr(27) ^ subr(29);
        CamelliaSubkeyL(29) = subl(28) ^ subl(30); /* round 22 */
        CamelliaSubkeyR(29) = subr(28) ^ subr(30);
        CamelliaSubkeyL(30) = subl(29) ^ subl(31); /* round 23 */
        CamelliaSubkeyR(30) = subr(29) ^ subr(31);
        CamelliaSubkeyL(31) = subl(30);     /* round 24 */
        CamelliaSubkeyR(31) = subr(30);
        CamelliaSubkeyL(32) = subl(32) ^ subl(31); /* kw3 */
        CamelliaSubkeyR(32) = subr(32) ^ subr(31);

        /* apply the inverse of the last half of P-function */
        dw = CamelliaSubkeyL(2) ^ CamelliaSubkeyR(2),
    	dw = CAMELLIA_RL8(dw);/* round 1 */
        CamelliaSubkeyR(2) = CamelliaSubkeyL(2) ^ dw,
    	CamelliaSubkeyL(2) = dw;
        dw = CamelliaSubkeyL(3) ^ CamelliaSubkeyR(3),
    	dw = CAMELLIA_RL8(dw);/* round 2 */
        CamelliaSubkeyR(3) = CamelliaSubkeyL(3) ^ dw,
    	CamelliaSubkeyL(3) = dw;
        dw = CamelliaSubkeyL(4) ^ CamelliaSubkeyR(4),
    	dw = CAMELLIA_RL8(dw);/* round 3 */
        CamelliaSubkeyR(4) = CamelliaSubkeyL(4) ^ dw,
    	CamelliaSubkeyL(4) = dw;
        dw = CamelliaSubkeyL(5) ^ CamelliaSubkeyR(5),
    	dw = CAMELLIA_RL8(dw);/* round 4 */
        CamelliaSubkeyR(5) = CamelliaSubkeyL(5) ^ dw,
    	CamelliaSubkeyL(5) = dw;
        dw = CamelliaSubkeyL(6) ^ CamelliaSubkeyR(6),
    	dw = CAMELLIA_RL8(dw);/* round 5 */
        CamelliaSubkeyR(6) = CamelliaSubkeyL(6) ^ dw,
    	CamelliaSubkeyL(6) = dw;
        dw = CamelliaSubkeyL(7) ^ CamelliaSubkeyR(7),
    	dw = CAMELLIA_RL8(dw);/* round 6 */
        CamelliaSubkeyR(7) = CamelliaSubkeyL(7) ^ dw,
    	CamelliaSubkeyL(7) = dw;
        dw = CamelliaSubkeyL(10) ^ CamelliaSubkeyR(10),
    	dw = CAMELLIA_RL8(dw);/* round 7 */
        CamelliaSubkeyR(10) = CamelliaSubkeyL(10) ^ dw,
    	CamelliaSubkeyL(10) = dw;
        dw = CamelliaSubkeyL(11) ^ CamelliaSubkeyR(11),
    	dw = CAMELLIA_RL8(dw);/* round 8 */
        CamelliaSubkeyR(11) = CamelliaSubkeyL(11) ^ dw,
    	CamelliaSubkeyL(11) = dw;
        dw = CamelliaSubkeyL(12) ^ CamelliaSubkeyR(12),
    	dw = CAMELLIA_RL8(dw);/* round 9 */
        CamelliaSubkeyR(12) = CamelliaSubkeyL(12) ^ dw,
    	CamelliaSubkeyL(12) = dw;
        dw = CamelliaSubkeyL(13) ^ CamelliaSubkeyR(13),
    	dw = CAMELLIA_RL8(dw);/* round 10 */
        CamelliaSubkeyR(13) = CamelliaSubkeyL(13) ^ dw,
    	CamelliaSubkeyL(13) = dw;
        dw = CamelliaSubkeyL(14) ^ CamelliaSubkeyR(14),
    	dw = CAMELLIA_RL8(dw);/* round 11 */
        CamelliaSubkeyR(14) = CamelliaSubkeyL(14) ^ dw,
    	CamelliaSubkeyL(14) = dw;
        dw = CamelliaSubkeyL(15) ^ CamelliaSubkeyR(15),
    	dw = CAMELLIA_RL8(dw);/* round 12 */
        CamelliaSubkeyR(15) = CamelliaSubkeyL(15) ^ dw,
    	CamelliaSubkeyL(15) = dw;
        dw = CamelliaSubkeyL(18) ^ CamelliaSubkeyR(18),
    	dw = CAMELLIA_RL8(dw);/* round 13 */
        CamelliaSubkeyR(18) = CamelliaSubkeyL(18) ^ dw,
    	CamelliaSubkeyL(18) = dw;
        dw = CamelliaSubkeyL(19) ^ CamelliaSubkeyR(19),
    	dw = CAMELLIA_RL8(dw);/* round 14 */
        CamelliaSubkeyR(19) = CamelliaSubkeyL(19) ^ dw,
    	CamelliaSubkeyL(19) = dw;
        dw = CamelliaSubkeyL(20) ^ CamelliaSubkeyR(20),
    	dw = CAMELLIA_RL8(dw);/* round 15 */
        CamelliaSubkeyR(20) = CamelliaSubkeyL(20) ^ dw,
    	CamelliaSubkeyL(20) = dw;
        dw = CamelliaSubkeyL(21) ^ CamelliaSubkeyR(21),
    	dw = CAMELLIA_RL8(dw);/* round 16 */
        CamelliaSubkeyR(21) = CamelliaSubkeyL(21) ^ dw,
    	CamelliaSubkeyL(21) = dw;
        dw = CamelliaSubkeyL(22) ^ CamelliaSubkeyR(22),
    	dw = CAMELLIA_RL8(dw);/* round 17 */
        CamelliaSubkeyR(22) = CamelliaSubkeyL(22) ^ dw,
    	CamelliaSubkeyL(22) = dw;
        dw = CamelliaSubkeyL(23) ^ CamelliaSubkeyR(23),
    	dw = CAMELLIA_RL8(dw);/* round 18 */
        CamelliaSubkeyR(23) = CamelliaSubkeyL(23) ^ dw,
    	CamelliaSubkeyL(23) = dw;
        dw = CamelliaSubkeyL(26) ^ CamelliaSubkeyR(26),
    	dw = CAMELLIA_RL8(dw);/* round 19 */
        CamelliaSubkeyR(26) = CamelliaSubkeyL(26) ^ dw,
    	CamelliaSubkeyL(26) = dw;
        dw = CamelliaSubkeyL(27) ^ CamelliaSubkeyR(27),
    	dw = CAMELLIA_RL8(dw);/* round 20 */
        CamelliaSubkeyR(27) = CamelliaSubkeyL(27) ^ dw,
    	CamelliaSubkeyL(27) = dw;
        dw = CamelliaSubkeyL(28) ^ CamelliaSubkeyR(28),
    	dw = CAMELLIA_RL8(dw);/* round 21 */
        CamelliaSubkeyR(28) = CamelliaSubkeyL(28) ^ dw,
    	CamelliaSubkeyL(28) = dw;
        dw = CamelliaSubkeyL(29) ^ CamelliaSubkeyR(29),
    	dw = CAMELLIA_RL8(dw);/* round 22 */
        CamelliaSubkeyR(29) = CamelliaSubkeyL(29) ^ dw,
    	CamelliaSubkeyL(29) = dw;
        dw = CamelliaSubkeyL(30) ^ CamelliaSubkeyR(30),
    	dw = CAMELLIA_RL8(dw);/* round 23 */
        CamelliaSubkeyR(30) = CamelliaSubkeyL(30) ^ dw,
    	CamelliaSubkeyL(30) = dw;
        dw = CamelliaSubkeyL(31) ^ CamelliaSubkeyR(31),
    	dw = CAMELLIA_RL8(dw);/* round 24 */
        CamelliaSubkeyR(31) = CamelliaSubkeyL(31) ^ dw,
    	CamelliaSubkeyL(31) = dw;


        return;
    }

    static void camellia_setup192(const unsigned char *key, uint32_t *subkey)
    {
        unsigned char kk[32];
        uint32_t krll, krlr, krrl,krrr;

        memcpy(kk, key, 24);
        memcpy((unsigned char *)&krll, key+16,4);
        memcpy((unsigned char *)&krlr, key+20,4);
        krrl = ~krll;
        krrr = ~krlr;
        memcpy(kk+24, (unsigned char *)&krrl, 4);
        memcpy(kk+28, (unsigned char *)&krrr, 4);
        camellia_setup256(kk, subkey);
        return;
    }


    /**
     * Stuff related to camellia encryption/decryption
     */
    static void camellia_encrypt128(const uint32_t *subkey, uint32_t *io)
    {
        uint32_t il, ir, t0, t1;

        SWAP4WORD(io);
        /* pre whitening but absorb kw2*/
        io[0] ^= CamelliaSubkeyL(0);
        io[1] ^= CamelliaSubkeyR(0);
        /* main iteration */

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(2),CamelliaSubkeyR(2),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(3),CamelliaSubkeyR(3),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(4),CamelliaSubkeyR(4),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(5),CamelliaSubkeyR(5),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(6),CamelliaSubkeyR(6),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(7),CamelliaSubkeyR(7),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(8),CamelliaSubkeyR(8),
    		 CamelliaSubkeyL(9),CamelliaSubkeyR(9),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(10),CamelliaSubkeyR(10),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(11),CamelliaSubkeyR(11),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(12),CamelliaSubkeyR(12),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(13),CamelliaSubkeyR(13),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(14),CamelliaSubkeyR(14),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(15),CamelliaSubkeyR(15),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(16),CamelliaSubkeyR(16),
    		 CamelliaSubkeyL(17),CamelliaSubkeyR(17),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(18),CamelliaSubkeyR(18),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(19),CamelliaSubkeyR(19),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(20),CamelliaSubkeyR(20),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(21),CamelliaSubkeyR(21),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(22),CamelliaSubkeyR(22),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(23),CamelliaSubkeyR(23),
    		     io[0],io[1],il,ir,t0,t1);

        /* post whitening but kw4 */
        io[2] ^= CamelliaSubkeyL(24);
        io[3] ^= CamelliaSubkeyR(24);

        t0 = io[0];
        t1 = io[1];
        io[0] = io[2];
        io[1] = io[3];
        io[2] = t0;
        io[3] = t1;
        SWAP4WORD(io);
    
        return;
    }

    static void camellia_decrypt128(const uint32_t *subkey, uint32_t *io)
    {
        uint32_t il,ir,t0,t1;               /* temporary valiables */

        SWAP4WORD(io);

        /* pre whitening but absorb kw2*/
        io[0] ^= CamelliaSubkeyL(24);
        io[1] ^= CamelliaSubkeyR(24);

        /* main iteration */
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(23),CamelliaSubkeyR(23),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(22),CamelliaSubkeyR(22),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(21),CamelliaSubkeyR(21),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(20),CamelliaSubkeyR(20),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(19),CamelliaSubkeyR(19),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(18),CamelliaSubkeyR(18),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(17),CamelliaSubkeyR(17),
    		 CamelliaSubkeyL(16),CamelliaSubkeyR(16),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(15),CamelliaSubkeyR(15),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(14),CamelliaSubkeyR(14),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(13),CamelliaSubkeyR(13),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(12),CamelliaSubkeyR(12),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(11),CamelliaSubkeyR(11),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(10),CamelliaSubkeyR(10),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(9),CamelliaSubkeyR(9),
    		 CamelliaSubkeyL(8),CamelliaSubkeyR(8),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(7),CamelliaSubkeyR(7),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(6),CamelliaSubkeyR(6),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(5),CamelliaSubkeyR(5),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(4),CamelliaSubkeyR(4),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(3),CamelliaSubkeyR(3),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(2),CamelliaSubkeyR(2),
    		     io[0],io[1],il,ir,t0,t1);

        /* post whitening but kw4 */
        io[2] ^= CamelliaSubkeyL(0);
        io[3] ^= CamelliaSubkeyR(0);

        t0 = io[0];
        t1 = io[1];
        io[0] = io[2];
        io[1] = io[3];
        io[2] = t0;
        io[3] = t1;
        SWAP4WORD(io);

        return;
    }

    /**
     * stuff for 192 and 256bit encryption/decryption
     */
    static void camellia_encrypt256(const uint32_t *subkey, uint32_t *io)
    {
        uint32_t il,ir,t0,t1;           /* temporary valiables */

        SWAP4WORD(io);

        /* pre whitening but absorb kw2*/
        io[0] ^= CamelliaSubkeyL(0);
        io[1] ^= CamelliaSubkeyR(0);

        /* main iteration */
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(2),CamelliaSubkeyR(2),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(3),CamelliaSubkeyR(3),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(4),CamelliaSubkeyR(4),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(5),CamelliaSubkeyR(5),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(6),CamelliaSubkeyR(6),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(7),CamelliaSubkeyR(7),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(8),CamelliaSubkeyR(8),
    		 CamelliaSubkeyL(9),CamelliaSubkeyR(9),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(10),CamelliaSubkeyR(10),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(11),CamelliaSubkeyR(11),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(12),CamelliaSubkeyR(12),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(13),CamelliaSubkeyR(13),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(14),CamelliaSubkeyR(14),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(15),CamelliaSubkeyR(15),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(16),CamelliaSubkeyR(16),
    		 CamelliaSubkeyL(17),CamelliaSubkeyR(17),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(18),CamelliaSubkeyR(18),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(19),CamelliaSubkeyR(19),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(20),CamelliaSubkeyR(20),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(21),CamelliaSubkeyR(21),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(22),CamelliaSubkeyR(22),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(23),CamelliaSubkeyR(23),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(24),CamelliaSubkeyR(24),
    		 CamelliaSubkeyL(25),CamelliaSubkeyR(25),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(26),CamelliaSubkeyR(26),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(27),CamelliaSubkeyR(27),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(28),CamelliaSubkeyR(28),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(29),CamelliaSubkeyR(29),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(30),CamelliaSubkeyR(30),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(31),CamelliaSubkeyR(31),
    		     io[0],io[1],il,ir,t0,t1);

        /* post whitening but kw4 */
        io[2] ^= CamelliaSubkeyL(32);
        io[3] ^= CamelliaSubkeyR(32);

        t0 = io[0];
        t1 = io[1];
        io[0] = io[2];
        io[1] = io[3];
        io[2] = t0;
        io[3] = t1;
        SWAP4WORD(io);

        return;
    }

    static void camellia_decrypt256(const uint32_t *subkey, uint32_t *io)
    {
        uint32_t il,ir,t0,t1;           /* temporary valiables */

        SWAP4WORD(io);
        /* pre whitening but absorb kw2*/
        io[0] ^= CamelliaSubkeyL(32);
        io[1] ^= CamelliaSubkeyR(32);
    
        /* main iteration */
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(31),CamelliaSubkeyR(31),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(30),CamelliaSubkeyR(30),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(29),CamelliaSubkeyR(29),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(28),CamelliaSubkeyR(28),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(27),CamelliaSubkeyR(27),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(26),CamelliaSubkeyR(26),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(25),CamelliaSubkeyR(25),
    		 CamelliaSubkeyL(24),CamelliaSubkeyR(24),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(23),CamelliaSubkeyR(23),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(22),CamelliaSubkeyR(22),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(21),CamelliaSubkeyR(21),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(20),CamelliaSubkeyR(20),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(19),CamelliaSubkeyR(19),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(18),CamelliaSubkeyR(18),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(17),CamelliaSubkeyR(17),
    		 CamelliaSubkeyL(16),CamelliaSubkeyR(16),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(15),CamelliaSubkeyR(15),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(14),CamelliaSubkeyR(14),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(13),CamelliaSubkeyR(13),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(12),CamelliaSubkeyR(12),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(11),CamelliaSubkeyR(11),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(10),CamelliaSubkeyR(10),
    		     io[0],io[1],il,ir,t0,t1);

        CAMELLIA_FLS(io[0],io[1],io[2],io[3],
    		 CamelliaSubkeyL(9),CamelliaSubkeyR(9),
    		 CamelliaSubkeyL(8),CamelliaSubkeyR(8),
    		 t0,t1,il,ir);

        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(7),CamelliaSubkeyR(7),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(6),CamelliaSubkeyR(6),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(5),CamelliaSubkeyR(5),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(4),CamelliaSubkeyR(4),
    		     io[0],io[1],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[0],io[1],
    		     CamelliaSubkeyL(3),CamelliaSubkeyR(3),
    		     io[2],io[3],il,ir,t0,t1);
        CAMELLIA_ROUNDSM(io[2],io[3],
    		     CamelliaSubkeyL(2),CamelliaSubkeyR(2),
    		     io[0],io[1],il,ir,t0,t1);

        /* post whitening but kw4 */
        io[2] ^= CamelliaSubkeyL(0);
        io[3] ^= CamelliaSubkeyR(0);

        t0 = io[0];
        t1 = io[1];
        io[0] = io[2];
        io[1] = io[3];
        io[2] = t0;
        io[3] = t1;
        SWAP4WORD(io);

        return;
    }



    static void Camellia_Ekeygen(const int keyBitLength, 
    		      const unsigned char *rawKey, 
    		      CAMELLIA_KEY_TABLE_TYPE keyTable)
    {
        switch(keyBitLength) {
        case 128:
    	camellia_setup128(rawKey, keyTable);
    	break;
        case 192:
    	camellia_setup192(rawKey, keyTable);
    	break;
        case 256:
    	camellia_setup256(rawKey, keyTable);
    	break;
        default:
    	break;
        }
    }


    static void Camellia_EncryptBlock(const int keyBitLength, 
    			   const unsigned char *plaintext, 
    			   const CAMELLIA_KEY_TABLE_TYPE keyTable, 
    			   unsigned char *ciphertext)
    {
        uint32_t tmp[4];

        memcpy((unsigned char *)tmp, plaintext, CAMELLIA_BLOCK_SIZE);

        switch (keyBitLength) {
        case 128:
    	camellia_encrypt128(keyTable, tmp);
    	break;
        case 192:
    	/* fall through */
        case 256:
    	camellia_encrypt256(keyTable, tmp);
    	break;
        default:
    	break;
        }

        memcpy(ciphertext, (unsigned char *)tmp, CAMELLIA_BLOCK_SIZE);
    }

    static void Camellia_DecryptBlock(const int keyBitLength, 
    			   const unsigned char *ciphertext, 
    			   const CAMELLIA_KEY_TABLE_TYPE keyTable, 
    			   unsigned char *plaintext)
    {
        uint32_t tmp[4];

        memcpy((unsigned char *)tmp, ciphertext, CAMELLIA_BLOCK_SIZE);

        switch (keyBitLength) {
        case 128:
    	camellia_decrypt128(keyTable, tmp);
    	break;
        case 192:
    	/* fall through */
        case 256:
    	camellia_decrypt256(keyTable, tmp);
	break;
    default:
	break;
    }
    memcpy(plaintext, (unsigned char *)tmp, CAMELLIA_BLOCK_SIZE);
    }
}



namespace{ // Mine
    static ByteArray Camellia_padOrTruncateKey(ByteArray key, const ByteArray::size_type desiredLength)
    {
        if (key.size() > desiredLength) {
            // Key is too long, truncate it
            key.resize(desiredLength);
        }
        else if (key.size() < desiredLength) {
            // Key is too short, pad it with zeros
            const size_t numZerosToAdd = desiredLength - key.size();
            key.resize(desiredLength);
            std::fill(key.end() - numZerosToAdd, key.end(), '\0');
        }

        return key;
    }

    static ByteArray Camellia_autoPadOrTruncateKey(ByteArray key){
        ByteArray::size_type l = 0;

        if(key.size() < 128){
            l = 128;
        }
        else if(key.size() < 192){
            l = 192;
        }
        else if(key.size() < 256 || key.size() > 256){
            l = 256;
        }
        else{
            l = key.size();
        }

        return Camellia_padOrTruncateKey(key, l);
    }
}






// Utilities
ByteArray Utils::StringToByteArray(std::string content){
    return ByteArray(content.begin(), content.end());
}

std::string Utils::ByteArrayToString(ByteArray bytearr){
    return std::string(bytearr.begin(), bytearr.end());
}

bool Utils::VersionTypeToString(MPARC::version_type input, std::string& output){
    output = std::to_string(input);
    return true;
}

bool Utils::StringToVersionType(std::string input, MPARC::version_type& output){
    try{
        output = std::stoull(input);
        return true;
    }
    catch(...){
        return false;
    }
}

Status::Code Utils::isDirectoryDefaultImplementation(std::string path){
    #ifdef MXPSQL_MPARC_FSLIB
        #if MXPSQL_MPARC_FSLIB == 1 // C++17 fs
        return (
            fslib::is_directory(fslib::path(path)) ?
            Status::Code::OK :
            Status::Code::ISDIR
        );
        #else
        (static_cast<void>(path));
        return Status::Code::NOT_IMPLEMENTED;
        #endif
    #else
    (static_cast<void>(path));
    return Status::Code::NOT_IMPLEMENTED;
    #endif
}


// STATUS FUNCTIONS
bool Status::isOK(){
    return getCode() == Status::Code::OK;
}

void Status::assertion(bool throw_err=true){
    if(!isOK()){
        if(throw_err){
            throw std::runtime_error(str() + ": " + std::to_string(getCode()));
        }
        else{
            std::cerr << "MPARC11[ABORT] " << str() << " (" << std::to_string(getCode()) << ")" << std::endl;
            std::abort();
        }
    }
}

std::string Status::str(Status::Code* code){
    Status::Code cod = Status::Code::OK;

    cod = (code ? (*code) : getCode());

    if(cod & OK){
        return "OK";
    }
    else if(cod & GENERIC) {
        return "A generic error has been detected.";
    }
    else if(cod & INTERNAL) {
        return "An internal error has been detected.";
    }
    else if(cod & NOT_IMPLEMENTED) {
        return "Not implemented.";
    }
    else if(cod & FALSE) {
        return "False";
    }
    else if(cod & INVALID_VALUE) {
        if(cod & NULL_VALUE) return "A null value has been provided at an inappropriate time.";
        return "An invalid value has been provided.";
    }
    else if(cod & KEY) {
        if(cod & KEY_EXISTS) return "The key exists.";
        if(cod & KEY_NOEXISTS) return "The key does not exist.";
        return "A generic/unknown key related error has been detected.";
    }
    else if(cod & FERROR) {
        return "A File I/O related error has been detected.";
    }
    else if(cod & ISDIR) {
        return "The object is a directory.";
    }
    else if(cod & CONSTRUCT_FAIL) {
        return "Archive failed during construction.";
    }
    else if (cod & PARSE_FAIL) {
        if(cod & NOT_MPAR_ARCHIVE) return "What was parsed is not an MPAR archive.";
        if(cod & CHECKSUM_ERROR) return "The checksum did not match or it was not obtained successfully.";
        if(cod & VERSION_ERROR) return "The version either was too new, invalid or it was not obtained successfully.";
        return "A generic parsing error has been detected.";
    }
    if(cod & CRYPT_ERROR){
        if(cod & CRYPT_NONE) return "No encryption was set.";
        if(cod & CRYPT_MISUSE) return "A misuse of the encryption facility was detected.";
        if(cod & CRYPT_FAIL) return "Failure was detected during cryptography.";
        return "A generic cryptography error has been detected.";
    }
    else{
        my_unreachable();
        return "Unknown code";
    }
}

std::string Status::str(){
    return str(nullptr);
}

Status::Code Status::getCode(){
    return stat_code;
}

Status::operator bool(){
    return isOK();
}


// MAIN ARCHIVE STRUCTURE
const std::string MPARC::filename_field = "filename"; // Compatibility with the C99 library
const std::string MPARC::content_field = "blob"; // Compatibility with the C99 library
const std::string MPARC::checksum_field = "crcsum"; // Compatibility with the C99 library
const std::string MPARC::fletcher_checksum_field = "fletcher32sum";
const std::string MPARC::processed_checksum_field = "crcsum.processed";
const std::string MPARC::meta_field = "metadata";

const std::string MPARC::encrypt_meta_field = "encrypt"; // Compatibility with the C99 library
const std::string MPARC::extra_meta_field = "extra";

const std::string MPARC::magic_number = "MXPSQL's Portable Archive"; // Compatibility with the C99 library


std::map<std::string, std::string> MPARC::dummy_extra_metadata;

MPARC::MPARC(){};
MPARC::MPARC(std::vector<std::string> entries){
    Status stat;
    for(std::string entry : entries){
        if(!(stat = push(entry, true))){
            stat.assertion(true);
        }
    }
}
MPARC::MPARC(MPARC& other){
    std::vector<std::string> entries;
    Status stat;
    if(!(stat = other.list(entries))){
        stat.assertion(true);
    }
}

void MPARC::init(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
}


Status MPARC::clear(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::vector<std::string> ee;
    Status stat;
    if(!(stat = list(ee))){
        return stat;
    }

    for(auto e : ee){
        if(!(stat = pop(e))){
            return stat;
        }
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::exists(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return Status(
        (this->my_code = (
                static_cast<Status::Code>
                ( 
                    (entries.count(name) == 0 || entries.find(name) == entries.end()) ?
                    (Status::Code::KEY | Status::Code::KEY_NOEXISTS) :
                    (Status::Code::OK)
                )
            )
        )
    );
}


Status MPARC::push(std::string name, Entry entry, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(!overwrite && exists(name)){
        return Status(
            (this->my_code = static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS))
        );
    }

    entries[name] = entry;

    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::push(std::string name, ByteArray content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Entry entreh;
    entreh.content = content;
    return push(name, entreh, overwrite);
}

Status MPARC::push(std::string name, std::string content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return push(name, Utils::StringToByteArray(content), overwrite);
}

Status MPARC::push(std::string name, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);

    std::string content;

    {
        std::ifstream ifs(name, std::ios::binary);
        if(!ifs.is_open() || !ifs.good()){
            return Status(Status::Code::FERROR);
        }

        std::stringstream ssbuf;
        ssbuf << ifs.rdbuf();


        content = ssbuf.str();
    }
    
    return push(name, content, overwrite);
}


Status MPARC::pop(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    entries.erase(entries.find(name));
    
    return Status(
        (this->my_code = Status::Code::OK)  
    );
}


Status MPARC::peek(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}

Status MPARC::peek(std::string name, Entry& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = peek(name);
    if(!stat) return stat;

    if(!(stat = peek(name))) return stat;

    try{
        output = entries.at(name);
    }
    catch(...){
        return (stat = Status(
            (this->my_code = 
                static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_NOEXISTS)
            )
        ));
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}

Status MPARC::peek(std::string name, std::string* output_str, ByteArray* output_ba, std::map<std::string, std::string>* output_meta){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = peek(name);
    if(!stat) return stat;

    if(!(stat = peek(name))) return stat;

    Entry entreh;
    ByteArray ba;
    std::map<std::string, std::string> meta;

    if(!(stat = peek(name, entreh))){
        return stat;
    }

    ba = entreh.content;
    meta = entreh.metadata;
    
    if(output_ba){
        *output_ba = ba;
    }
    if(output_str){
        *output_str = Utils::ByteArrayToString(ba);
    }
    if(output_meta){
        *output_meta = meta;
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}


Status MPARC::swap(std::string name, std::string name2){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    if(
        !(
        (stat = peek(name)) &&
        (stat = peek(name2))
        )
    ) return stat;

    Entry entreh, entreh2;

    if(
        !(
            (
                stat = peek(name, entreh)
            ) &&
            (
                stat = peek(name2, entreh2)
            )
        )
    ) return stat;

    if(
        !(
            (
                stat = pop(name)
            ) &&
            (
                stat = pop(name2)
            )
        )
    ) return stat;

    if(
        !(
            (
                stat = push(name, entreh2, true)
            ) &&
            (
                stat = push(name2, entreh, true)
            )
        )
    ) return stat;

    return stat;
}

Status MPARC::copy(std::string name, std::string name2, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    {
        Status e = (
            (
                (stat = peek(name)) && !overwrite
            ) ?
            static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS) :
            Status::Code::OK
        );

        if(
            !(
                (stat = peek(name)) &&
                (stat = e)
            )
        ) return stat;
    }

    Entry entreh;

    if(
        !(stat = peek(name, entreh))
    ) return stat;

    if(
        !(stat = pop(name2))
    ) return stat;  

    if(
        !(stat = push(name2, entreh, overwrite))
    )  return stat;

    return stat;
}

Status MPARC::rename(std::string name, std::string name2, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    {
        Status e = (
            (
                (stat = peek(name)) && !overwrite
            ) ?
            static_cast<Status::Code>(Status::Code::KEY | Status::Code::KEY_EXISTS) :
            Status::Code::OK
        );

        if(
            !(
                (stat = peek(name)) &&
                (stat = e)
            )
        ) return stat;
    }

    Entry entreh;

    if(
        !(stat = peek(name, entreh))
    ) return stat;

    if(
        !(
            (stat = pop(name)) &&
            (stat = pop(name2))
        )
    ) return stat;  

    if(
        !(
            (stat = push(name2, entreh, overwrite))
        )
    )  return stat;

    return stat;
}


Status MPARC::list(std::vector<std::string>& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    output.clear();

    for(auto pair : entries){
        output.push_back(pair.first);
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::extra_metadata_setter_getter(std::map<std::string, std::string>& output, std::map<std::string, std::string>& input){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(std::addressof(input) != std::addressof(MPARC::dummy_extra_metadata)) this->extra_meta_data = input;
    
    if(std::addressof(output) != std::addressof(MPARC::dummy_extra_metadata)) output = this->extra_meta_data;

    return Status(
        this->my_code = Status::Code::OK
    );
}


Status MPARC::construct(std::string& output, MPARC::version_type ver){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    std::stringstream archive;
    Status stat;

    if(ver < 1 || ver > MPARC::mpar_version){
        return Status(
            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::VERSION_ERROR)
        );
    }

    { // Construct the header
        std::string header = "";
        if(!(stat = construct_header(*this, header, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
        }
        archive << header;
    }
    { // Construct the tnreis
        std::string entries = "";
        if(!(stat = construct_entries(*this, entries, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
        }
        archive << entries;
    }
    { // Construct the footer
        std::string footer = "";
        if(!(stat = construct_footer(*this, footer, ver))){
            return (this->my_code = static_cast<Status::Code>(Status::Code::CONSTRUCT_FAIL | stat.getCode()));
        }
        archive << footer;
    }

    output = archive.str();


    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::construct(std::string &output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return construct(output, MPARC::mpar_version);
}


Status MPARC::parse(std::string input){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);

    Status stat;

    std::string header = "";
    std::string entries = "";
    std::string footer = "";

    // This section would be handled by a function that would perform steganography by searching for the archive if embedded in files, but it is not yet implemented.
    // Maybe never will, maybe will be implemented, I don't know, lets see later.
    // I realized that the function would require to be extremly smart at finding the archive, it cannot be some "find a character" thing.
    // "Scan a block to see if it is an archive, then move one character if not an archive, put it in if its an archive" would be nice, but inefficient.
    // I really need a good/better search function.
    // For now, it will just be a cranker input.
    std::string searchInput = input;

    // find the header and footer marks
    size_t header_marksep_pos = searchInput.find(MPARC::post_header_separator);
    size_t footer_marksep_pos = searchInput.find_last_of(MPARC::end_of_entries_separator);

    // Check if either both marker exist and that the header marker position is before the footer marker position
    if(header_marksep_pos == std::string::npos || footer_marksep_pos == std::string::npos || header_marksep_pos >= footer_marksep_pos){
        return Status(
            (this->my_code = static_cast<Status::Code>(
                Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE
            ))
        );
    }

    // Slice the strings into their parts
    header = searchInput.substr(0, header_marksep_pos);
    entries = searchInput.substr(header_marksep_pos+1, (footer_marksep_pos-header_marksep_pos-1));
    footer = searchInput.substr(footer_marksep_pos+1, (input.size()-footer_marksep_pos));

    // Parse the header
    if(!(stat = parse_header(*this, header))){
        return (this->my_code = stat.getCode());
    }

    // Parse the entries
    if(!(stat = parse_entries(*this, entries))){
        return (this->my_code = stat.getCode());
    }

    // Parse the footer
    if(!(stat = parse_footer(*this, footer))){
        return (this->my_code = stat.getCode());
    }

    return Status(
        (this->my_code = Status::Code::OK)
    );
}


Status MPARC::get_status(Status& output){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    output = this->my_code;
    return Status::Code::OK;
}

Status MPARC::isOK(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat;
    get_status(stat);
    return(
        (stat.isOK()) ?
        Status::Code::OK :
        Status::Code::FALSE
    );
}


Status MPARC::set_xor_encryption(std::string key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->XOR_key = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_xor_encryption(std::string& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->XOR_key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::set_rot_encryption(std::vector<int> key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->ROT_key = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_rot_encryption(std::vector<int>& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->ROT_key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::set_camellia_encryption(std::string key){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(false){
        return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::CRYPT_NONE); // Unavailable
    }
    if(ByteArray(key.begin(), key.end()).size() > 256) return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);
    this->Camellia_k = key;
    return (this->my_code = Status::Code::OK);
}

Status MPARC::get_camellia_encryption(std::string& okey){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    okey = this->Camellia_k;
    return (this->my_code = Status::Code::OK);
}


Status MPARC::set_locale(std::locale loc){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    this->locale = loc;
    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::get_locale(std::locale& locput){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    locput = this->locale;
    return Status(
        (this->my_code = Status::Code::OK)
    );
}



MPARC::operator bool(){
    return ((this->isOK() == Status::Code::OK) ? true : false);
}

MPARC::operator void*(){
    return ((this->isOK() == Status::Code::OK) ? const_cast<MPARC*>(this) : nullptr);
}




// UTILITIES
[[noreturn]] static inline void my_unreachable(...){
    #if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
        std::unreachable(); // Very standard
    #else

        #ifdef __GNUC__ // GCC, Clang, ICC
            __builtin_unreachable(); // NO
        #elif defined(_MSC_VER) // MSVC
            __assume(false); // FALSE
        #else // NULL / ABORT
            int stupid = 21; // Stupid number for reinterpret_cast
            int* f = static_cast<int*>(NULL); // Undefined 1: Null dereference
            *f = reinterpret_cast<long int*>(&stupid); // Undefined 2+3: Pointer aliasing? By missusing C++'s reinterpret_cast? Yep, with missusing reinterpret_cast. Double uh-oh's. That other one is assigning to NULL.
            std::abort(); // Undefined '4': ABRT if not possible.
        #endif

    #endif
}



// ARCHIVE BUILDER
static Status construct_header(MPARC& archive, std::string& output, MPARC::version_type ver){
    Status stat;
    // String builder
    std::stringstream ssb;

    // Build the initial metadata
    ssb << MPARC::magic_number << MPARC::magic_number_separator;
    {
        std::string shadow_ver = "";
        Utils::VersionTypeToString(ver, shadow_ver);
        ssb << shadow_ver;
    }

    // Build the extra JSON metadata
    {
        json j = json::object();

        // Encryption field
        {
            std::string encrypt_field = ((ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION)
                ? b64::to_base64(MPARC::encrypt_meta_field)
                : MPARC::encrypt_meta_field
            );
            j[encrypt_field] = json::array();

            {
                std::string XOR_k;
                std::vector<int> ROT_k;
                std::string Camellia_ks;
                ByteArray Camellia_k;

                archive.get_xor_encryption(XOR_k);
                archive.get_rot_encryption(ROT_k);
                archive.get_camellia_encryption(Camellia_ks);

                Camellia_k = Utils::StringToByteArray(Camellia_ks);
                (static_cast<void>(Camellia_k));

                std::vector<std::string> encrypt;

                // Initial encryption
                if(!XOR_k.empty()){
                    encrypt.push_back("XOR");
                }
                if(!ROT_k.empty()){
                    encrypt.push_back("ROT");
                }

                // Extended encryption: Camellia
                if(ver >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty()){
                    encrypt.push_back("Camellia");
                }

                j[encrypt_field] = encrypt;
            }
        }

        if(ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION){// Extra user defined metadata
            j[b64::to_base64(MPARC::extra_meta_field)] = json::object();
            std::map<std::string, std::string> extras;
            archive.extra_metadata_setter_getter(extras, MPARC::dummy_extra_metadata);
            for(auto extra_pair : extras){
                j[b64::to_base64(MPARC::extra_meta_field)][b64::to_base64(extra_pair.first)] = b64::to_base64(extra_pair.second);
            }
        }

        // Finally put the JSON
        ssb << MPARC::header_meta_magic_separator << j.dump();
    }
    // Cap it off
    ssb << MPARC::post_header_separator;

    output = ssb.str();
    return stat;
}

static Status construct_entries(MPARC& archive, std::string& output, MPARC::version_type ver){
    std::stringstream ssb;
    std::vector<std::string> entries;
    Status stat;

    // A typedef to make it less verbose
    using jty = std::pair<json, crc_t>; // The first side is the JSON document. The second side is the checksum of the JSON document (CRC32).

    // Sort function
    static auto sortcmp = [](const jty& lh, const jty rh){
        json jlh = lh.first;
        json jrh = rh.first;

        return jlh.at(MPARC::filename_field) > jrh.at(MPARC::filename_field);
    };

    // List the archive
    {
        stat = archive.list(entries);
        if(!stat){
            return stat;
        }
    }

    // Loop over the entries
    std::vector<jty> jentries;
    for(std::string entry : entries){
        json jentry;
        Entry entreh;
        // Read the entry
        {
            stat = archive.peek(entry, entreh);
            if(!stat){
                return stat;
            }
        }

        // Get the contents
        std::string strcontent = Utils::ByteArrayToString(entreh.content);

        { // Unprocessed checksum calculate (CRC32)
            std::string csum = "";

            crc_t crc = crc_init();

            crc = crc_update(crc, strcontent.c_str(), strcontent.length());

            crc = crc_finalize(crc);

            csum = crc_t_to_string(crc);

            jentry[MPARC::checksum_field] = csum;
        }

        if(ver >= MPARC::FLETCHER32_INITIAL_UPDATE_VERSION){ // Unprocessed checksum calculate (Fletcher32)
            std::string fsum;

            fletcher32_t fletcher = 0;

            {
                std::vector<uint16_t> uv16(strcontent.begin(), strcontent.end());

                fletcher = fletcher_t_update(&uv16[0], uv16.size());
            }

            fsum = fletcher_t_to_string(fletcher);

            jentry[MPARC::fletcher_checksum_field] = fsum;
        }

        std::string estrcontent = strcontent;
        {
            std::string XOR_k;
            std::vector<int> ROT_k;
            std::string Camellia_ks;
            ByteArray Camellia_k;


            archive.get_xor_encryption(XOR_k);
            archive.get_rot_encryption(ROT_k);
            archive.get_camellia_encryption(Camellia_ks);

            Camellia_k = Utils::StringToByteArray(Camellia_ks);

            Camellia_k = Camellia_autoPadOrTruncateKey(Camellia_k);

            ByteArray::size_type Camellia_kbl = Camellia_k.size();


            if(!XOR_k.empty()){
                estrcontent = encrypt_xor(estrcontent, XOR_k);
            }
            if(!ROT_k.empty()){
                estrcontent = encrypt_rot(estrcontent, ROT_k);
            }

            if(ver >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty() && !estrcontent.empty()){
                if(Camellia_kbl > 256) return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);

                // Make key
                CAMELLIA_KEY_TABLE_TYPE camellia_tl;
                memset(&camellia_tl, '\0', sizeof(camellia_tl));
                Camellia_Ekeygen(static_cast<int>(Camellia_kbl), const_cast<const unsigned char*>(&Camellia_k[0]), camellia_tl);

                { // Encryption magic
                    ByteArray raw;
                    raw = Utils::StringToByteArray(estrcontent);

                    const static ByteArray::size_type allignment = 4;
                    ByteArray::size_type pad_raw = raw.size() + (raw.size() % allignment); // Pad up the vector
                    raw.reserve(pad_raw+CAMELLIA_MAX_PAD_PLUS);

                    ByteArray ec;
                    ec.resize(pad_raw+CAMELLIA_MAX_PAD_PLUS);
                    std::fill(ec.begin(), ec.end(), '\0');

                    for(ByteArray::size_type i = 0; i < pad_raw; i += allignment){
                        Camellia_EncryptBlock(Camellia_kbl, &raw[i], camellia_tl, &ec[i]);
                    }

                    { // Jank packing
                        ByteArray::size_type pack_raw = ec.size() - raw.size();
                        for(ByteArray::size_type j = 0; j < pack_raw; j++){
                            if(!ec.empty()){
                                ec.pop_back();
                            }
                            else{
                                return (stat = static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_FAIL));
                            }
                        }
                    }

                    estrcontent = Utils::ByteArrayToString(ec);
                }
            }
        }

        // Put in the obvious ones (filename, contents)
        jentry[MPARC::filename_field] = b64::to_base64(entry);
        jentry[MPARC::content_field] = b64::to_base64(estrcontent);


        if(ver >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Put in the checksum after it is processed
            std::string csum = "";
            std::string b64content = jentry.at(MPARC::content_field);

            crc_t crc = crc_init();

            crc = crc_update(crc, b64content.c_str(), b64content.length());

            crc = crc_finalize(crc);

            csum = crc_t_to_string(crc);

            jentry[MPARC::processed_checksum_field] = csum;
        }

        if(ver >= MPARC::CAMELLIA_UPDATE_VERSION){ // Add the user defined metadata thing
            jentry[MPARC::meta_field] = json::object();
            for(auto meta : entreh.metadata){
                jentry[MPARC::meta_field][b64::to_base64(meta.first)] = b64::to_base64(meta.second);
            }
        }

        jty jentriy;
        { // Calculate the JSON's checksum, not the invidual content
            crc_t crc = crc_init();

            crc = crc_update(crc, jentry.dump().c_str(), strlen(jentry.dump().c_str()));

            crc = crc_finalize(crc);

            jentriy = std::make_pair(jentry, crc);
        }

        jentries.push_back(jentriy);
    }

    try{ // Sort the JSONs
        std::sort(jentries.begin(), jentries.end(), sortcmp);
    }
    catch(...){
        return (stat = Status(Status::Code::CONSTRUCT_FAIL));
    }

    {
        // Dump the entries
        for(jty jenty : jentries){
            ssb << jenty.second << MPARC::entry_checksum_content_separator << jenty.first.dump() << MPARC::entries_entry_separator;
        }
    }

    // Cap it
    ssb << MPARC::end_of_entries_separator;

    output = ssb.str();

    return stat;
}

static Status construct_footer(MPARC& archive, std::string& output, MPARC::version_type ver){
    (static_cast<void>(archive));
    (static_cast<void>(ver));

    std::stringstream ssb;

    // Just one character
    ssb << MPARC::end_of_archive_marker;

    output = ssb.str();

    return Status(Status::Code::OK);
}



// ARCHIVE PARSER
static Status parse_header(MPARC& archive, std::string header_input){
    ((void)archive);

    std::string magic = "";
    std::string vf_other = "";

    { // Separate the magic number and the usable content and then test it
        size_t magic_separator_pos = header_input.find(MPARC::magic_number_separator); // Grab the magic separator
        if(magic_separator_pos == std::string::npos){ // Check if it exists
            return Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            );
        }
        // Extract the magic number and the other useful ones
        magic = header_input.substr(0, magic_separator_pos);
        vf_other = header_input.substr(magic_separator_pos+1, std::string::npos);

        if(magic != MPARC::magic_number){
            return Status(
                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
            );
        }
    }

    {
        std::string version = "";
        std::string meta = "";
        { // Separate the version and the metadata and then grab it
            size_t vm_header_meta_separator_pos = vf_other.find(MPARC::header_meta_magic_separator); // Grab the metadata and version separator
            if(vm_header_meta_separator_pos == std::string::npos){ // Check if it exists
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                );
            }

            // Extract the version and the metadata
            version = vf_other.substr(0, vm_header_meta_separator_pos);
            meta = vf_other.substr(vm_header_meta_separator_pos+1, std::string::npos);
        }

        { // test the version
            MPARC::version_type ck_ver = 0;
            if(!Utils::StringToVersionType(version, ck_ver) || (ck_ver > MPARC::mpar_version)){
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::VERSION_ERROR)
                );
            }

            archive.loaded_version = ck_ver;
        }

        { // Parse the metadata
            json j = json::parse(meta, nullptr, false);
            if(j.is_discarded()){
                return Status(
                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                );
            }

            {
                std::string encrypt_field = ((archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION)
                    ? b64::to_base64(MPARC::encrypt_meta_field)
                    : MPARC::encrypt_meta_field
                );

                if(j.contains(encrypt_field)){ // encryption thingy
                    std::vector<std::string> encrypt_algos = j.at(encrypt_field);
                    { // Check for encryption presence

                    }
                }
                else{
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }
            }

            if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Extra global metadata, only if version 2 is reached
                if(!j.contains(b64::to_base64(MPARC::extra_meta_field))){ // check if the field is available
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                std::map<std::string, std::string> extras;

                for(auto meta : j[b64::to_base64(MPARC::extra_meta_field)].items()){
                    extras[b64::from_base64(meta.key())] = b64::from_base64(meta.value());
                }
                archive.extra_metadata_setter_getter(MPARC::dummy_extra_metadata, extras);
            }
        }
    }
    return Status::Code::OK;
}

static Status parse_entries(MPARC& archive, std::string entry_input){
    std::vector<std::string> lines;
    { // Read line by line
        std::stringstream ss(entry_input);
        std::string str;
        while(std::getline(ss, str, '\n')){
            lines.push_back(str);
        }
    }

    { // Loop over each line        
        for(auto line : lines){
            { // Trim early whitespace
                std::locale locale;
                archive.get_locale(locale);
                auto it =  std::find_if_not(line.begin(), line.end(), 
                    [&locale](char ch){ 
                        return std::isspace<char>(ch, locale); 
                    }
                );
                line.erase(line.begin(), it);
            }

            if(
                (line[0] == MPARC::comment_marker) // skip comments
                ||
                (line.empty() || line.length() < 1) // skip empty line
            )
            {
                continue;
            }

            std::string entry = "";
            { // Parse the checksum of the current JSON entry (not including the checksum)
                crc_t crc = 0;

                // Check for the presence of the checksum and get its position
                size_t checksum_entry_marksep_pos = line.find(MPARC::entry_checksum_content_separator);
                if(checksum_entry_marksep_pos == std::string::npos){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                // Grab the checksum and the actual entry
                std::string checksum = line.substr(0, checksum_entry_marksep_pos);
                entry = line.substr(checksum_entry_marksep_pos+1, (line.size()-checksum_entry_marksep_pos));

                // Grab the actual checksum value
                if(!scan_crc_t_from_string(checksum, crc)){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    );
                }

                { // Calculate and check the checksum
                    crc_t crc_check_now = crc_init();
                    crc_check_now = crc_update(crc_check_now, entry.c_str(), strlen(entry.c_str()));
                    crc_check_now = crc_finalize(crc_check_now);

                    if(crc != crc_check_now){
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                    }
                }
            }

            { // Parse the JSON entry

                // Parse it and check for errors
                json j = json::parse(entry, nullptr, false);
                if(j.is_discarded()){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                    );
                }

                if(
                    !(
                        j.contains(MPARC::content_field) && 
                        j.contains(MPARC::checksum_field) &&
                        j.contains(MPARC::filename_field)
                    )
                ){
                    return Status(
                        static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                    );
                }

                // Grab the processed base64 string
                std::string processed_b64_str = j.at(MPARC::content_field);

                if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // Check if archive is version 2 or more
                    if(!j.contains(MPARC::processed_checksum_field)){ // check if the field is available
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
                    }

                    { // Calculate the processed (base64'd) string's checksum
                        crc_t processed_checksum = 0;
                        crc_t calculated_checksum = crc_init();

                        // Calculate
                        std::string pstr = processed_b64_str;

                        calculated_checksum = crc_update(calculated_checksum, pstr.c_str(), pstr.length());

                        calculated_checksum = crc_finalize(calculated_checksum);


                        { // Grab the checksum from the entry and parse it
                            std::string strsum = j.at(MPARC::processed_checksum_field);

                            if(!scan_crc_t_from_string(strsum, processed_checksum)){
                                return Status(
                                    static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                                );
                            }
                        }

                        // Check
                        if(processed_checksum != calculated_checksum){
                            return Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                        }
                    }
                }

                // Decryption and unprocessed string grab
                std::string raw_unprocessed_str = "";
                {
                    std::string ecrypt_str = b64::from_base64(processed_b64_str);

                    std::string XOR_k;
                    std::vector<int> ROT_k;
                    std::string Camellia_ks;
                    ByteArray Camellia_k;
                    ByteArray::size_type Camellia_kbl;

                    archive.get_xor_encryption(XOR_k);
                    archive.get_rot_encryption(ROT_k);
                    archive.get_camellia_encryption(Camellia_ks);

                    Camellia_k = Utils::StringToByteArray(Camellia_ks);

                    Camellia_k = Camellia_autoPadOrTruncateKey(Camellia_k);

                    Camellia_kbl = Camellia_k.size();

                    if(archive.loaded_version >= MPARC::CAMELLIA_UPDATE_VERSION && !Camellia_k.empty() && !ecrypt_str.empty()){
                        if(Camellia_kbl > 256) return static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_MISUSE);

                        // Make key
                        CAMELLIA_KEY_TABLE_TYPE camellia_tl;
                        memset(&camellia_tl, '\0', sizeof(camellia_tl));
                        Camellia_Ekeygen(static_cast<int>(Camellia_kbl), const_cast<const unsigned char*>(&Camellia_k[0]), camellia_tl);

                        { // DEcryption magic
                            ByteArray ec;
                            ec = Utils::StringToByteArray(ecrypt_str);

                            const constexpr static ByteArray::size_type allignment = 4;
                            ByteArray::size_type pad_ec_plus = (ec.size() % allignment);
                            ByteArray::size_type pad_ec = ec.size() + pad_ec_plus; // Pad up the vector
                            ec.reserve(pad_ec);

                            ByteArray raw;
                            raw.resize(pad_ec+CAMELLIA_MAX_PAD_PLUS); // Jank fix
                            std::fill(raw.begin(), raw.end(), '\0');

                            for(ByteArray::size_type i = 0; i < pad_ec; i += allignment){
                                Camellia_DecryptBlock(Camellia_kbl, &ec[i], camellia_tl, &raw[i]);
                            }

                            { // Jank packing
                                ByteArray::size_type pack_ec = raw.size() - ec.size();
                                for(ByteArray::size_type j = 0; j < pack_ec; j++){
                                    if(!raw.empty()){
                                        raw.pop_back();
                                    }
                                    else{
                                        return (static_cast<Status::Code>(Status::Code::CRYPT_ERROR | Status::Code::CRYPT_FAIL));
                                    }
                                }
                            }

                            ecrypt_str = Utils::ByteArrayToString(raw);
                        }
                    }

                    if(!ROT_k.empty()){
                        for(auto& key : ROT_k){
                            int tmp_key = (-1 * key);
                            key = tmp_key;
                        }
                        ecrypt_str = encrypt_rot(ecrypt_str, ROT_k);
                    }
                    if(!XOR_k.empty()){
                        ecrypt_str = encrypt_xor(ecrypt_str, XOR_k);
                    }

                    raw_unprocessed_str = ecrypt_str;
                }
                Entry entreh;
                entreh.content = Utils::StringToByteArray(raw_unprocessed_str);

                { // Calculate the checksum (CRC32) of the unprocessed (no Base64 and encryption) string
                    crc_t unprocessed_checksum = 0;
                    crc_t calculated_checksum = crc_init();

                    // Calculate
                    calculated_checksum = crc_update(calculated_checksum, raw_unprocessed_str.c_str(), raw_unprocessed_str.length());

                    calculated_checksum = crc_finalize(calculated_checksum);

                    {
                        // Grab the checksum from the entry
                        std::string upstrsum = j.at(MPARC::checksum_field);

                        // Parse and scan it
                        if(!scan_crc_t_from_string(upstrsum, unprocessed_checksum)){
                            return Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                        }
                    }

                    // Check
                    if(unprocessed_checksum != calculated_checksum){
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                    }
                }

                if(archive.loaded_version >= MPARC::FLETCHER32_INITIAL_UPDATE_VERSION){ // Calculate the Fletcher32 version of the unprocessed string
                    fletcher32_t sum = 0; // Scanned/Parsed sum
                    fletcher32_t lsum = 0; // Local sum

                    { // Calculate locally
                        std::vector<uint16_t> uvec16(raw_unprocessed_str.begin(), raw_unprocessed_str.end());
                        lsum = fletcher_t_update(&uvec16[0], uvec16.size());
                    }

                    {
                        // Grab the fletcher
                        std::string fletch = j.at(MPARC::fletcher_checksum_field);

                        if(!scan_fletcher_t_from_string(fletch, sum)){
                            return Status(
                                static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                            );
                        }
                    }

                    // Check
                    if(lsum != sum){
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::CHECKSUM_ERROR)
                        );
                    }
                }

                if(archive.loaded_version >= MPARC::EXTENSIBILITY_UPDATE_VERSION){ // check if archive is version 2 or more
                    if(!j.contains(MPARC::meta_field)){ // check if the field is available
                        return Status(
                            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
                        );
                    }
                    // Set the metadata
                    for(auto meta : j[MPARC::meta_field].items()){
                        entreh.metadata[b64::from_base64(meta.key())] = b64::from_base64(meta.value());
                    }
                }

                // Push it in
                Status stat;
                if(!(
                    stat = archive.push(b64::from_base64(
                        j.at(MPARC::filename_field)
                    ), entreh, true)
                )){
                    return stat;
                }
            }
        }
    }
    return Status(Status::Code::OK);
}

static Status parse_footer(MPARC& archive, std::string footer_input){
    (static_cast<void>(archive));
    size_t footer_pos = footer_input.find(MPARC::end_of_archive_marker);
    if(footer_pos == std::string::npos || footer_input[footer_pos] != MPARC::end_of_archive_marker) {
        return Status(
            static_cast<Status::Code>(Status::Code::PARSE_FAIL | Status::Code::NOT_MPAR_ARCHIVE)
        );
    }
    return Status(Status::Code::OK);
}



static std::string encrypt_xor(std::string input, std::string key){
    std::string outstr = input;
    std::string::size_type input_len = input.size();
    std::string::size_type key_len = key.size();

    if(!key.empty()){
        for(std::string::size_type i = 0; i < input_len; i++){
            outstr[i] = (input[i] ^ key[i % key_len]);
        }
    }

    return outstr;
}

static std::string encrypt_rot(std::string input, std::vector<int> key){
    std::string outstr = input;
    std::string::size_type input_len = input.size();
    std::vector<int>::size_type key_len = key.size();

    if(!key.empty()){
        for(std::string::size_type i = 0; i < input_len; i++){
            outstr[i] = (input[i] + key.at(i % key_len));
        }
    }

    return outstr;
}
