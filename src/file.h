/**
 * \file file.h
 * \brief nfdump file loading, handling and saving functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright
 * (C) 2017 Jaap Versteegh. All rights reserved.
 * (C) 2017 SURFnet. All rights reserved.
 * \license
 * This software may be modified and distributed under the
 * terms of the BSD license. See the LICENSE file for details.
 */

#ifndef _FILE_H
#define _FILE_H

#include "block.h"

typedef struct {
  // Meta data
  size_t size;
  char* name;
  // Data
  file_header_t header;
  stat_record_t stats;
  nf_block_p blocks[];
} nf_file_t;
typedef nf_file_t* nf_file_p;


extern nf_file_p file_new();
extern nf_file_p file_load(const char* filename, block_handler_p handle_block);
extern void file_free(nf_file_p *file);

extern int file_save(const nf_file_p file);
extern int file_save_as(nf_file_p file, const char* filename);
extern int file_for_each_block(const nf_file_p file, block_handler_p handle_block);

#endif
