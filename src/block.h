/** 
 * \file block.h
 * \brief nfdump block parsing and handling functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license. See the LICENSE file for details.
 */

#ifndef _BLOCK_H
#define _BLOCK_H

#include "types.h"
#include "record.h"


typedef struct {
  int status;
  data_block_header_t header;
  compression_t compression;
  compression_t file_compression;
  size_t compressed_size;
  size_t uncompressed_size;
  char* data;
  nf_record_p (*records)[];
} nf_block_t;
typedef nf_block_t* nf_block_p;


typedef void (*block_handler_p) (const int, nf_block_p);

extern nf_block_p block_new();
extern void block_free(nf_block_p *block);

#endif
