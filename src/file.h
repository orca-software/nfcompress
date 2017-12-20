/** 
 * \file file.h
 * \brief nfdump file loading, handling and saving functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license. See the LICENSE file for details.
 */

#ifndef _FILE_H
#define _FILE_H

#include "block.h"

typedef struct {
  size_t size;
  file_header_t header;
  stat_record_t stats;
  nf_block_p blocks[];
} nf_file_t;
typedef nf_file_t* nf_file_p;

extern nf_file_t* load(const char* filename, block_handler_p handle_block);
extern int save(const char* filename, nf_file_t* fl);
extern void for_each_block(nf_file_t* fl, block_handler_p handle_block);
extern int blocks_status(nf_file_t* fl);

extern nf_file_p new_file();
extern void free_file(nf_file_p *fl);

#endif
