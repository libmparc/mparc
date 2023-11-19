#ifndef MXPSQL_MPARC_SUM_HPP
#define MXPSQL_MPARC_SUM_HPP

#include <cstdint>
#include <string>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

// TYPIST for checksums
typedef std::uint_fast32_t crc_t;
typedef std::uint_fast32_t fletcher32_t;

// Checksum implementation
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

// Hash implementations and classes
namespace {
// CLASSES FOR HASHES

// THIS FROM https://github.com/stbrumme/hash-library/blob/master/md5.h
// ZLIB LICENSE. THIS GOES TO Stephan Brumme.
class MD5 //: public Hash
{
public:
  /// split into 64 byte blocks (=> 512 bits), hash is 16 bytes long
  enum { BlockSize = 512 / 8, HashBytes = 16 };

  /// same as reset()
  MD5();

  /// add arbitrary number of bytes
  void add(const void* data, size_t numBytes);

  /// return latest hash as 32 hex characters
  std::string getHash();
  /// return latest hash as bytes
  void        getHash(unsigned char buffer[HashBytes]);

  /// restart
  void reset();

private:
  /// process 64 bytes
  void processBlock(const void* data);
  /// process everything left in the internal buffer
  void processBuffer();

  /// size of processed data in bytes
  uint64_t m_numBytes;
  /// valid bytes in m_buffer
  size_t   m_bufferSize;
  /// bytes not processed yet
  uint8_t  m_buffer[BlockSize];

  enum { HashValues = HashBytes / 4 };
  /// hash, stored as integers
  uint32_t m_hash[HashValues];
};

// THIS FROM https://github.com/stbrumme/hash-library/blob/master/sha256.h
// also ZLIB LICENSE. THIS also GOES TO Stephan Brumme.
class SHA256 //: public Hash
{
public:
  /// split into 64 byte blocks (=> 512 bits), hash is 32 bytes long
  enum { BlockSize = 512 / 8, HashBytes = 32 };

  /// same as reset()
  SHA256();


  /// add arbitrary number of bytes
  void add(const void* data, size_t numBytes);

  /// return latest hash as 64 hex characters
  std::string getHash();
  /// return latest hash as bytes
  void        getHash(unsigned char buffer[HashBytes]);

  /// restart
  void reset();

private:
  /// process 64 bytes
  void processBlock(const void* data);
  /// process everything left in the internal buffer
  void processBuffer();

  /// size of processed data in bytes
  uint64_t m_numBytes;
  /// valid bytes in m_buffer
  size_t   m_bufferSize;
  /// bytes not processed yet
  uint8_t  m_buffer[BlockSize];

  enum { HashValues = HashBytes / 4 };
  /// hash, stored as integers
  uint32_t m_hash[HashValues];
};



// THIS IS ALSO FROM Stephan Brumme. Its from the same hash-library repository. This is for the swap functions.

  // mix functions for processBlock()
  inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d)
  {
    return d ^ (b & (c ^ d)); // original: f = (b & c) | ((~b) & d);
  }

  inline uint32_t f2(uint32_t b, uint32_t c, uint32_t d)
  {
    return c ^ (d & (b ^ c)); // original: f = (b & d) | (c & (~d));
  }

  inline uint32_t f3(uint32_t b, uint32_t c, uint32_t d)
  {
    return b ^ c ^ d;
  }

  inline uint32_t f4(uint32_t b, uint32_t c, uint32_t d)
  {
    return c ^ (b | ~d);
  }

  inline uint32_t rotate(uint32_t a, uint32_t c)
  {
    return (a << c) | (a >> (32 - c));
  }
  
  inline uint32_t swap(uint32_t x)
  {
    return (x >> 24) |
          ((x >>  8) & 0x0000FF00) |
          ((x <<  8) & 0x00FF0000) |
           (x << 24);
  }

  // THIS IS ALSO FROM Stephan Brumme. Its from the same hash-library repository. This is MD5
/// same as reset()
MD5::MD5()
{
  reset();
}


/// restart
void MD5::reset()
{
  m_numBytes   = 0;
  m_bufferSize = 0;

  // according to RFC 1321
  m_hash[0] = 0x67452301;
  m_hash[1] = 0xefcdab89;
  m_hash[2] = 0x98badcfe;
  m_hash[3] = 0x10325476;
}


/// process 64 bytes
void MD5::processBlock(const void* data)
{
  // get last hash
  uint32_t a = m_hash[0];
  uint32_t b = m_hash[1];
  uint32_t c = m_hash[2];
  uint32_t d = m_hash[3];

  // data represented as 16x 32-bit words
  const uint32_t* words = (uint32_t*) data;

  // computations are little endian, swap data if necessary
#if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
#define LITTLEENDIAN(x) swap(x)
#else
#define LITTLEENDIAN(x) (x)
#endif

  // first round
  uint32_t word0  = LITTLEENDIAN(words[ 0]);
  a = rotate(a + f1(b,c,d) + word0  + 0xd76aa478,  7) + b;
  uint32_t word1  = LITTLEENDIAN(words[ 1]);
  d = rotate(d + f1(a,b,c) + word1  + 0xe8c7b756, 12) + a;
  uint32_t word2  = LITTLEENDIAN(words[ 2]);
  c = rotate(c + f1(d,a,b) + word2  + 0x242070db, 17) + d;
  uint32_t word3  = LITTLEENDIAN(words[ 3]);
  b = rotate(b + f1(c,d,a) + word3  + 0xc1bdceee, 22) + c;

  uint32_t word4  = LITTLEENDIAN(words[ 4]);
  a = rotate(a + f1(b,c,d) + word4  + 0xf57c0faf,  7) + b;
  uint32_t word5  = LITTLEENDIAN(words[ 5]);
  d = rotate(d + f1(a,b,c) + word5  + 0x4787c62a, 12) + a;
  uint32_t word6  = LITTLEENDIAN(words[ 6]);
  c = rotate(c + f1(d,a,b) + word6  + 0xa8304613, 17) + d;
  uint32_t word7  = LITTLEENDIAN(words[ 7]);
  b = rotate(b + f1(c,d,a) + word7  + 0xfd469501, 22) + c;

  uint32_t word8  = LITTLEENDIAN(words[ 8]);
  a = rotate(a + f1(b,c,d) + word8  + 0x698098d8,  7) + b;
  uint32_t word9  = LITTLEENDIAN(words[ 9]);
  d = rotate(d + f1(a,b,c) + word9  + 0x8b44f7af, 12) + a;
  uint32_t word10 = LITTLEENDIAN(words[10]);
  c = rotate(c + f1(d,a,b) + word10 + 0xffff5bb1, 17) + d;
  uint32_t word11 = LITTLEENDIAN(words[11]);
  b = rotate(b + f1(c,d,a) + word11 + 0x895cd7be, 22) + c;

  uint32_t word12 = LITTLEENDIAN(words[12]);
  a = rotate(a + f1(b,c,d) + word12 + 0x6b901122,  7) + b;
  uint32_t word13 = LITTLEENDIAN(words[13]);
  d = rotate(d + f1(a,b,c) + word13 + 0xfd987193, 12) + a;
  uint32_t word14 = LITTLEENDIAN(words[14]);
  c = rotate(c + f1(d,a,b) + word14 + 0xa679438e, 17) + d;
  uint32_t word15 = LITTLEENDIAN(words[15]);
  b = rotate(b + f1(c,d,a) + word15 + 0x49b40821, 22) + c;

  // second round
  a = rotate(a + f2(b,c,d) + word1  + 0xf61e2562,  5) + b;
  d = rotate(d + f2(a,b,c) + word6  + 0xc040b340,  9) + a;
  c = rotate(c + f2(d,a,b) + word11 + 0x265e5a51, 14) + d;
  b = rotate(b + f2(c,d,a) + word0  + 0xe9b6c7aa, 20) + c;

  a = rotate(a + f2(b,c,d) + word5  + 0xd62f105d,  5) + b;
  d = rotate(d + f2(a,b,c) + word10 + 0x02441453,  9) + a;
  c = rotate(c + f2(d,a,b) + word15 + 0xd8a1e681, 14) + d;
  b = rotate(b + f2(c,d,a) + word4  + 0xe7d3fbc8, 20) + c;

  a = rotate(a + f2(b,c,d) + word9  + 0x21e1cde6,  5) + b;
  d = rotate(d + f2(a,b,c) + word14 + 0xc33707d6,  9) + a;
  c = rotate(c + f2(d,a,b) + word3  + 0xf4d50d87, 14) + d;
  b = rotate(b + f2(c,d,a) + word8  + 0x455a14ed, 20) + c;

  a = rotate(a + f2(b,c,d) + word13 + 0xa9e3e905,  5) + b;
  d = rotate(d + f2(a,b,c) + word2  + 0xfcefa3f8,  9) + a;
  c = rotate(c + f2(d,a,b) + word7  + 0x676f02d9, 14) + d;
  b = rotate(b + f2(c,d,a) + word12 + 0x8d2a4c8a, 20) + c;

  // third round
  a = rotate(a + f3(b,c,d) + word5  + 0xfffa3942,  4) + b;
  d = rotate(d + f3(a,b,c) + word8  + 0x8771f681, 11) + a;
  c = rotate(c + f3(d,a,b) + word11 + 0x6d9d6122, 16) + d;
  b = rotate(b + f3(c,d,a) + word14 + 0xfde5380c, 23) + c;

  a = rotate(a + f3(b,c,d) + word1  + 0xa4beea44,  4) + b;
  d = rotate(d + f3(a,b,c) + word4  + 0x4bdecfa9, 11) + a;
  c = rotate(c + f3(d,a,b) + word7  + 0xf6bb4b60, 16) + d;
  b = rotate(b + f3(c,d,a) + word10 + 0xbebfbc70, 23) + c;

  a = rotate(a + f3(b,c,d) + word13 + 0x289b7ec6,  4) + b;
  d = rotate(d + f3(a,b,c) + word0  + 0xeaa127fa, 11) + a;
  c = rotate(c + f3(d,a,b) + word3  + 0xd4ef3085, 16) + d;
  b = rotate(b + f3(c,d,a) + word6  + 0x04881d05, 23) + c;

  a = rotate(a + f3(b,c,d) + word9  + 0xd9d4d039,  4) + b;
  d = rotate(d + f3(a,b,c) + word12 + 0xe6db99e5, 11) + a;
  c = rotate(c + f3(d,a,b) + word15 + 0x1fa27cf8, 16) + d;
  b = rotate(b + f3(c,d,a) + word2  + 0xc4ac5665, 23) + c;

  // fourth round
  a = rotate(a + f4(b,c,d) + word0  + 0xf4292244,  6) + b;
  d = rotate(d + f4(a,b,c) + word7  + 0x432aff97, 10) + a;
  c = rotate(c + f4(d,a,b) + word14 + 0xab9423a7, 15) + d;
  b = rotate(b + f4(c,d,a) + word5  + 0xfc93a039, 21) + c;

  a = rotate(a + f4(b,c,d) + word12 + 0x655b59c3,  6) + b;
  d = rotate(d + f4(a,b,c) + word3  + 0x8f0ccc92, 10) + a;
  c = rotate(c + f4(d,a,b) + word10 + 0xffeff47d, 15) + d;
  b = rotate(b + f4(c,d,a) + word1  + 0x85845dd1, 21) + c;

  a = rotate(a + f4(b,c,d) + word8  + 0x6fa87e4f,  6) + b;
  d = rotate(d + f4(a,b,c) + word15 + 0xfe2ce6e0, 10) + a;
  c = rotate(c + f4(d,a,b) + word6  + 0xa3014314, 15) + d;
  b = rotate(b + f4(c,d,a) + word13 + 0x4e0811a1, 21) + c;

  a = rotate(a + f4(b,c,d) + word4  + 0xf7537e82,  6) + b;
  d = rotate(d + f4(a,b,c) + word11 + 0xbd3af235, 10) + a;
  c = rotate(c + f4(d,a,b) + word2  + 0x2ad7d2bb, 15) + d;
  b = rotate(b + f4(c,d,a) + word9  + 0xeb86d391, 21) + c;

  // update hash
  m_hash[0] += a;
  m_hash[1] += b;
  m_hash[2] += c;
  m_hash[3] += d;
}


/// add arbitrary number of bytes
void MD5::add(const void* data, size_t numBytes)
{
  const uint8_t* current = (const uint8_t*) data;

  if (m_bufferSize > 0)
  {
    while (numBytes > 0 && m_bufferSize < BlockSize)
    {
      m_buffer[m_bufferSize++] = *current++;
      numBytes--;
    }
  }

  // full buffer
  if (m_bufferSize == BlockSize)
  {
    processBlock(m_buffer);
    m_numBytes  += BlockSize;
    m_bufferSize = 0;
  }

  // no more data ?
  if (numBytes == 0)
    return;

  // process full blocks
  while (numBytes >= BlockSize)
  {
    processBlock(current);
    current    += BlockSize;
    m_numBytes += BlockSize;
    numBytes   -= BlockSize;
  }

  // keep remaining bytes in buffer
  while (numBytes > 0)
  {
    m_buffer[m_bufferSize++] = *current++;
    numBytes--;
  }
}


/// process final block, less than 64 bytes
void MD5::processBuffer()
{
  // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

  // - append "1" bit to message
  // - append "0" bits until message length in bit mod 512 is 448
  // - append length as 64 bit integer

  // number of bits
  size_t paddedLength = m_bufferSize * 8;

  // plus one bit set to 1 (always appended)
  paddedLength++;

  // number of bits must be (numBits % 512) = 448
  size_t lower11Bits = paddedLength & 511;
  if (lower11Bits <= 448)
    paddedLength +=       448 - lower11Bits;
  else
    paddedLength += 512 + 448 - lower11Bits;
  // convert from bits to bytes
  paddedLength /= 8;

  // only needed if additional data flows over into a second block
  unsigned char extra[BlockSize];

  // append a "1" bit, 128 => binary 10000000
  if (m_bufferSize < BlockSize)
    m_buffer[m_bufferSize] = 128;
  else
    extra[0] = 128;

  size_t i;
  for (i = m_bufferSize + 1; i < BlockSize; i++)
    m_buffer[i] = 0;
  for (; i < paddedLength; i++)
    extra[i - BlockSize] = 0;

  // add message length in bits as 64 bit number
  uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
  // find right position
  unsigned char* addLength;
  if (paddedLength < BlockSize)
    addLength = m_buffer + paddedLength;
  else
    addLength = extra + paddedLength - BlockSize;

  // must be little endian
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF; msgBits >>= 8;
  *addLength++ = msgBits & 0xFF;

  // process blocks
  processBlock(m_buffer);
  // flowed over into a second block ?
  if (paddedLength > BlockSize)
    processBlock(extra);
}


/// return latest hash as 32 hex characters
std::string MD5::getHash()
{
  // compute hash (as raw bytes)
  unsigned char rawHash[HashBytes];
  getHash(rawHash);

  // convert to hex string
  std::string result;
  result.reserve(2 * HashBytes);
  for (int i = 0; i < HashBytes; i++)
  {
    static const char dec2hex[16+1] = "0123456789abcdef";
    result += dec2hex[(rawHash[i] >> 4) & 15];
    result += dec2hex[ rawHash[i]       & 15];
  }

  return result;
}


/// return latest hash as bytes
void MD5::getHash(unsigned char buffer[MD5::HashBytes])
{
  // save old hash if buffer is partially filled
  uint32_t oldHash[HashValues];
  for (int i = 0; i < HashValues; i++)
    oldHash[i] = m_hash[i];

  // process remaining bytes
  processBuffer();

  unsigned char* current = buffer;
  for (int i = 0; i < HashValues; i++)
  {
    *current++ =  m_hash[i]        & 0xFF;
    *current++ = (m_hash[i] >>  8) & 0xFF;
    *current++ = (m_hash[i] >> 16) & 0xFF;
    *current++ = (m_hash[i] >> 24) & 0xFF;

    // restore old hash
    m_hash[i] = oldHash[i];
  }
}



/// same as reset()
SHA256::SHA256()
{
  reset();
}


/// restart
void SHA256::reset()
{
  m_numBytes   = 0;
  m_bufferSize = 0;

  // according to RFC 1321
  // "These words were obtained by taking the first thirty-two bits of the
  //  fractional parts of the square roots of the first eight prime numbers"
  m_hash[0] = 0x6a09e667;
  m_hash[1] = 0xbb67ae85;
  m_hash[2] = 0x3c6ef372;
  m_hash[3] = 0xa54ff53a;
  m_hash[4] = 0x510e527f;
  m_hash[5] = 0x9b05688c;
  m_hash[6] = 0x1f83d9ab;
  m_hash[7] = 0x5be0cd19;

#ifdef SHA2_224_SEED_VECTOR
  // if you want SHA2-224 instead then use these seeds
  // and throw away the last 32 bits of getHash
  m_hash[0] = 0xc1059ed8;
  m_hash[1] = 0x367cd507;
  m_hash[2] = 0x3070dd17;
  m_hash[3] = 0xf70e5939;
  m_hash[4] = 0xffc00b31;
  m_hash[5] = 0x68581511;
  m_hash[6] = 0x64f98fa7;
  m_hash[7] = 0xbefa4fa4;
#endif
}


/// process 64 bytes
void SHA256::processBlock(const void* data)
{
  // get last hash
  uint32_t a = m_hash[0];
  uint32_t b = m_hash[1];
  uint32_t c = m_hash[2];
  uint32_t d = m_hash[3];
  uint32_t e = m_hash[4];
  uint32_t f = m_hash[5];
  uint32_t g = m_hash[6];
  uint32_t h = m_hash[7];

  // data represented as 16x 32-bit words
  const uint32_t* input = (uint32_t*) data;
  // convert to big endian
  uint32_t words[64];
  int i;
  for (i = 0; i < 16; i++)
#if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
    words[i] =      input[i];
#else
    words[i] = swap(input[i]);
#endif

  uint32_t x,y; // temporaries

  // first round
  x = h + f1(e,f,g) + 0x428a2f98 + words[ 0]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0x71374491 + words[ 1]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0xb5c0fbcf + words[ 2]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0xe9b5dba5 + words[ 3]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x3956c25b + words[ 4]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0x59f111f1 + words[ 5]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x923f82a4 + words[ 6]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0xab1c5ed5 + words[ 7]; y = f2(b,c,d); e += x; a = x + y;

  // secound round
  x = h + f1(e,f,g) + 0xd807aa98 + words[ 8]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0x12835b01 + words[ 9]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0x243185be + words[10]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0x550c7dc3 + words[11]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x72be5d74 + words[12]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0x80deb1fe + words[13]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x9bdc06a7 + words[14]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0xc19bf174 + words[15]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 24 words
  for (; i < 24; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // third round
  x = h + f1(e,f,g) + 0xe49b69c1 + words[16]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0xefbe4786 + words[17]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0x0fc19dc6 + words[18]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0x240ca1cc + words[19]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x2de92c6f + words[20]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0x4a7484aa + words[21]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x5cb0a9dc + words[22]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0x76f988da + words[23]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 32 words
  for (; i < 32; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // fourth round
  x = h + f1(e,f,g) + 0x983e5152 + words[24]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0xa831c66d + words[25]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0xb00327c8 + words[26]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0xbf597fc7 + words[27]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0xc6e00bf3 + words[28]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0xd5a79147 + words[29]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x06ca6351 + words[30]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0x14292967 + words[31]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 40 words
  for (; i < 40; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // fifth round
  x = h + f1(e,f,g) + 0x27b70a85 + words[32]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0x2e1b2138 + words[33]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0x4d2c6dfc + words[34]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0x53380d13 + words[35]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x650a7354 + words[36]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0x766a0abb + words[37]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x81c2c92e + words[38]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0x92722c85 + words[39]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 48 words
  for (; i < 48; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // sixth round
  x = h + f1(e,f,g) + 0xa2bfe8a1 + words[40]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0xa81a664b + words[41]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0xc24b8b70 + words[42]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0xc76c51a3 + words[43]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0xd192e819 + words[44]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0xd6990624 + words[45]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0xf40e3585 + words[46]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0x106aa070 + words[47]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 56 words
  for (; i < 56; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // seventh round
  x = h + f1(e,f,g) + 0x19a4c116 + words[48]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0x1e376c08 + words[49]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0x2748774c + words[50]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0x34b0bcb5 + words[51]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x391c0cb3 + words[52]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0x4ed8aa4a + words[53]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0x5b9cca4f + words[54]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0x682e6ff3 + words[55]; y = f2(b,c,d); e += x; a = x + y;

  // extend to 64 words
  for (; i < 64; i++)
    words[i] = words[i-16] +
               (rotate(words[i-15],  7) ^ rotate(words[i-15], 18) ^ (words[i-15] >>  3)) +
               words[i-7] +
               (rotate(words[i- 2], 17) ^ rotate(words[i- 2], 19) ^ (words[i- 2] >> 10));

  // eigth round
  x = h + f1(e,f,g) + 0x748f82ee + words[56]; y = f2(a,b,c); d += x; h = x + y;
  x = g + f1(d,e,f) + 0x78a5636f + words[57]; y = f2(h,a,b); c += x; g = x + y;
  x = f + f1(c,d,e) + 0x84c87814 + words[58]; y = f2(g,h,a); b += x; f = x + y;
  x = e + f1(b,c,d) + 0x8cc70208 + words[59]; y = f2(f,g,h); a += x; e = x + y;
  x = d + f1(a,b,c) + 0x90befffa + words[60]; y = f2(e,f,g); h += x; d = x + y;
  x = c + f1(h,a,b) + 0xa4506ceb + words[61]; y = f2(d,e,f); g += x; c = x + y;
  x = b + f1(g,h,a) + 0xbef9a3f7 + words[62]; y = f2(c,d,e); f += x; b = x + y;
  x = a + f1(f,g,h) + 0xc67178f2 + words[63]; y = f2(b,c,d); e += x; a = x + y;

  // update hash
  m_hash[0] += a;
  m_hash[1] += b;
  m_hash[2] += c;
  m_hash[3] += d;
  m_hash[4] += e;
  m_hash[5] += f;
  m_hash[6] += g;
  m_hash[7] += h;
}


/// add arbitrary number of bytes
void SHA256::add(const void* data, size_t numBytes)
{
  const uint8_t* current = (const uint8_t*) data;

  if (m_bufferSize > 0)
  {
    while (numBytes > 0 && m_bufferSize < BlockSize)
    {
      m_buffer[m_bufferSize++] = *current++;
      numBytes--;
    }
  }

  // full buffer
  if (m_bufferSize == BlockSize)
  {
    processBlock(m_buffer);
    m_numBytes  += BlockSize;
    m_bufferSize = 0;
  }

  // no more data ?
  if (numBytes == 0)
    return;

  // process full blocks
  while (numBytes >= BlockSize)
  {
    processBlock(current);
    current    += BlockSize;
    m_numBytes += BlockSize;
    numBytes   -= BlockSize;
  }

  // keep remaining bytes in buffer
  while (numBytes > 0)
  {
    m_buffer[m_bufferSize++] = *current++;
    numBytes--;
  }
}


/// process final block, less than 64 bytes
void SHA256::processBuffer()
{
  // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

  // - append "1" bit to message
  // - append "0" bits until message length in bit mod 512 is 448
  // - append length as 64 bit integer

  // number of bits
  size_t paddedLength = m_bufferSize * 8;

  // plus one bit set to 1 (always appended)
  paddedLength++;

  // number of bits must be (numBits % 512) = 448
  size_t lower11Bits = paddedLength & 511;
  if (lower11Bits <= 448)
    paddedLength +=       448 - lower11Bits;
  else
    paddedLength += 512 + 448 - lower11Bits;
  // convert from bits to bytes
  paddedLength /= 8;

  // only needed if additional data flows over into a second block
  unsigned char extra[BlockSize];

  // append a "1" bit, 128 => binary 10000000
  if (m_bufferSize < BlockSize)
    m_buffer[m_bufferSize] = 128;
  else
    extra[0] = 128;

  size_t i;
  for (i = m_bufferSize + 1; i < BlockSize; i++)
    m_buffer[i] = 0;
  for (; i < paddedLength; i++)
    extra[i - BlockSize] = 0;

  // add message length in bits as 64 bit number
  uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
  // find right position
  unsigned char* addLength;
  if (paddedLength < BlockSize)
    addLength = m_buffer + paddedLength;
  else
    addLength = extra + paddedLength - BlockSize;

  // must be big endian
  *addLength++ = (unsigned char)((msgBits >> 56) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 48) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 40) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 32) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 24) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 16) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >>  8) & 0xFF);
  *addLength   = (unsigned char)( msgBits        & 0xFF);

  // process blocks
  processBlock(m_buffer);
  // flowed over into a second block ?
  if (paddedLength > BlockSize)
    processBlock(extra);
}


/// return latest hash as 64 hex characters
std::string SHA256::getHash()
{
  // compute hash (as raw bytes)
  unsigned char rawHash[HashBytes];
  getHash(rawHash);

  // convert to hex string
  std::string result;
  result.reserve(2 * HashBytes);
  for (int i = 0; i < HashBytes; i++)
  {
    static const char dec2hex[16+1] = "0123456789abcdef";
    result += dec2hex[(rawHash[i] >> 4) & 15];
    result += dec2hex[ rawHash[i]       & 15];
  }

  return result;
}


/// return latest hash as bytes
void SHA256::getHash(unsigned char buffer[SHA256::HashBytes])
{
  // save old hash if buffer is partially filled
  uint32_t oldHash[HashValues];
  for (int i = 0; i < HashValues; i++)
    oldHash[i] = m_hash[i];

  // process remaining bytes
  processBuffer();

  unsigned char* current = buffer;
  for (int i = 0; i < HashValues; i++)
  {
    *current++ = (m_hash[i] >> 24) & 0xFF;
    *current++ = (m_hash[i] >> 16) & 0xFF;
    *current++ = (m_hash[i] >>  8) & 0xFF;
    *current++ =  m_hash[i]        & 0xFF;

    // restore old hash
    m_hash[i] = oldHash[i];
  }
}

}

#endif
