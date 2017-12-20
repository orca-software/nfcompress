/** 
 * \file compress.h
 * \brief Block (de)compression functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license.  See the LICENSE file for details.
 */

#ifndef _COMPRESS_H
#define _COMPRESS_H

#include "block.h"


static const uint32_t compression_flags[] = {
  0x0,
  FLAG_LZO_COMPRESSED,
  FLAG_BZ2_COMPRESSED,
  FLAG_LZ4_COMPRESSED,
  FLAG_LZMA_COMPRESSED,
  0 // terminator: leave as last element
};

#define DEFAULT_BZ2_PRESET 9
#define DEFAULT_LZMA_PRESET 6

extern int bz2_preset;
extern int lzma_preset;

int compress(nf_block_t* block, compression_t compression);
int decompress(nf_block_t* block);

extern void decompressor(const int blocknum, nf_block_t* block);

extern void lzo_compressor(const int blocknum, nf_block_t* block);
extern void bz2_compressor(const int blocknum, nf_block_t* block);
extern void lz4_compressor(const int blocknum, nf_block_t* block);
extern void lzma_compressor(const int blocknum, nf_block_t* block);

typedef int (*transform_fun_p) (const char*, const size_t, char*, size_t*);
typedef size_t (*size_fun_p) (const size_t);
typedef struct {
  transform_fun_p transform;
  size_fun_p size;
  const char* name;
  const int ok_result;
  const int buffer_error;
} transform_funs_t;
extern const transform_funs_t compress_funs_list[];

#endif

