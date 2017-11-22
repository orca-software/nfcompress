#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

// Include nffile for header, stat and block header structures
#include <sys/types.h>
#include "../nfdump/bin/nffile.h"

#define FLAG_LZ4_COMPRESSED 0x10
#define FLAG_LZMA_COMPRESSED 0x20


typedef enum {
  compressed_none, 
  compressed_lzo,
  compressed_bz2,
  compressed_lz4,
  compressed_lzma,
  compressed_term // terminator: leave as last element
} compression_t;

static const uint32_t compression_flags[] = {
  0x0,
  FLAG_LZO_COMPRESSED,
  FLAG_BZ2_COMPRESSED,
  FLAG_LZ4_COMPRESSED,
  FLAG_LZMA_COMPRESSED,
  0 // terminator: leave as last element
};


typedef struct {
  int status;
  data_block_header_t header;
  compression_t compression;
  char* data;
} nf_block_t;


typedef void (*block_handler_p) (const int blocknum, nf_block_t* block);
  

typedef struct {
  file_header_t header;
  stat_record_t stats;
  nf_block_t blocks[];
} nf_file_t;

#endif

