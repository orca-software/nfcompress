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

#include "types.h"

#define DEFAULT_BZ2_PRESET 9
#define DEFAULTS_LZMA_PRESET 6

extern int bz2_preset;
extern int lzma_preset;

extern int compress_bz2(nf_block_t* block);
extern int decompress_bz2(nf_block_t* block);

extern int compress_lz4(nf_block_t* block);
extern int decompress_lz4(nf_block_t* block);

extern int compress_lzo(nf_block_t* block);
extern int decompress_lzo(nf_block_t* block);

extern int compress_lzma(nf_block_t* block);
extern int decompress_lzma(nf_block_t* block);

extern void decompressor(const int blocknum, nf_block_t* block);

extern void lzo_compressor(const int blocknum, nf_block_t* block);
extern void bz2_compressor(const int blocknum, nf_block_t* block);
extern void lz4_compressor(const int blocknum, nf_block_t* block);
extern void lzma_compressor(const int blocknum, nf_block_t* block);

#endif

