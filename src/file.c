/** 
 * \file file.c
 * \brief nfdump file loading, handling and saving functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license. See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "compress.h"
#include "file.h"

int read_block(FILE *f, nf_block_t* block) {
  size_t bytes_read = fread(&block->header, 1, sizeof(block->header), f);
  if (bytes_read != sizeof(block->header)) {
    // Only whine when not immediately at end of file. 
    if (bytes_read != 0)
      msg(log_error, "Failed to read block header\n");
    goto failure;
  }
  block->data = (char*)malloc(block->header.size);
  if (block->data == NULL) {
    msg(log_error, "Failed to allocate block data\n");
    goto failure;
  }
  bytes_read = fread(block->data, 1, block->header.size, f);
  if (bytes_read != block->header.size) {
    msg(log_error, "Failed to read block data\n");
    goto failure;
  }
  block->status = 0;
  return 0;
failure:
  free(block->data);
  block->data = NULL;
  block->header.size = 0;
  block->status = -1;
  return -1;
}


int write_block(FILE *f, nf_block_t* block) {
  if (block->status != 0) {
    msg(log_error, "Invalid block\n");
    goto failure;
  }
  size_t bytes_written = fwrite(&block->header, 1, sizeof(block->header), f);
  if (bytes_written != sizeof(block->header)) {
    msg(log_error, "Failed to write block header\n");
    goto failure;
  }
  bytes_written = fwrite(block->data, 1, block->header.size, f);
  if (bytes_written != block->header.size) {
    msg(log_error, "Failed to write block data\n");
    goto failure;
  }
  return 0;
failure:
  return -1;
}


void for_each_block(nf_file_p fl, block_handler_p handle_block) {
  #pragma omp parallel for
  for (int i = 0; i < fl->header.NumBlocks; ++i) {
    handle_block(i, fl->blocks[i]);
  }
}


int blocks_status(nf_file_p fl) {
  int result = 0;
  for (int i = 0; i < fl->header.NumBlocks; ++i) {
    if (fl->blocks[i]->status < result) 
      result = fl->blocks[i]->status;
  }
  return result;
}


nf_file_t* load(const char* filename, block_handler_p handle_block) {
  nf_file_p fl = new_file();
  if (fl == NULL) {
    msg(log_error, "Failed to allocate file buffer\n");
    return NULL;
  }

  msg(log_info, "Reading %s\n", filename);

  FILE *f = fopen(filename, "rb");
  if (!f) {
    msg(log_error, "Failed to open: %s\n", filename);
    goto failure;
  }

  size_t bytes_read = fread(&fl->header, 1, sizeof(fl->header), f);
  if (bytes_read != sizeof(fl->header)) {
    msg(log_error, "Failed to read file header\n");
    goto failure;
  }
  msg(log_debug, "Read file header\n");

  bytes_read = fread(&fl->stats, 1, sizeof(fl->stats), f);
  if (bytes_read != sizeof(fl->stats)) {
    msg(log_error, "Failed to read file stats\n");
    goto failure;
  }

  msg(log_debug, "Read file stats\n");

  size_t blocks_size = fl->header.NumBlocks * sizeof(nf_block_p);
  nf_file_p new_fl = (nf_file_p)realloc(fl, sizeof(nf_file_t) + blocks_size);
  if (new_fl == NULL) {
    msg(log_error, "Failed to re-allocate file buffer\n");
    goto failure;
  }
  fl = new_fl;
  memset(&fl->blocks, 0, blocks_size);

  compression_t file_compression =
      fl->header.flags & FLAG_LZO_COMPRESSED ? compressed_lzo : 
      fl->header.flags & FLAG_BZ2_COMPRESSED ? compressed_bz2 :
      fl->header.flags & FLAG_LZ4_COMPRESSED ? compressed_lz4 :
      fl->header.flags & FLAG_LZMA_COMPRESSED ? compressed_lzma :
        compressed_none;
  msg(log_info, "File compression: %d  flags: %u\n", file_compression, fl->header.flags);

  int blocks_read = 0;
  #pragma omp parallel
  #pragma omp master
  for (;;) {
    nf_block_p block = new_block();
    if (block == NULL) {
      msg(log_error, "Failed to allocate block buffer\n");
      break;
    }
    if (read_block(f, block) != 0) {
      free(block);
      break;
    }
    int block_idx = blocks_read++;
    if (blocks_read > fl->header.NumBlocks) {
      blocks_size = blocks_read * sizeof(nf_block_p);
      new_fl = (nf_file_p)realloc(fl, sizeof(nf_file_t) + blocks_size);
      if (new_fl == NULL) {
        msg(log_error, "Failed to re-allocate file buffer\n");
        break;
      }
      fl = new_fl;
      msg(log_info, "Fixed block count in header. found %d, header %d\n", blocks_read, fl->header.NumBlocks);
      fl->header.NumBlocks = blocks_read;
    }
    fl->blocks[block_idx] = block;
    // Catalog blocks are not compressed
    block->compression = block->header.id == CATALOG_BLOCK ? compressed_none : file_compression;
    block->file_compression = block->compression;
    size_t size = block->header.size;
    block->compressed_size = size;
    block->uncompressed_size = size;
    if (handle_block != NULL) {
      #pragma omp task firstprivate(block_idx, block)
      handle_block(block_idx, block);
    }
  }

  if (blocks_read < fl->header.NumBlocks) {
    msg(log_error, "Missing blocks in file. found %d, expected %d\n", blocks_read, fl->header.NumBlocks);
    goto failure;
  }

  if (blocks_status(fl) < 0) {
    msg(log_error, "One or more blocks failed to load properly\n");
    goto failure;
  }

  fl->size = ftell(f);
  fclose(f);
  return fl;
failure:
  if (f)
    fclose(f);
  free_file(&fl);
  return NULL;
}


int save(const char* filename, nf_file_t* fl) {
  msg(log_info, "Writing %s\n", filename);

  if (fl->header.NumBlocks == 0) {
    msg(log_error, "Not saving empty file");
    return -1;
  }

  compression_t file_compression = fl->blocks[0]->compression;
  // Switch of all compression flags
  for (compression_t cmpr = compressed_none; cmpr < compressed_term; ++cmpr) {
    fl->header.flags &= ~compression_flags[cmpr];
  }
  // ... and then select the compression method of the first block as compression type
  fl->header.flags |= compression_flags[file_compression];
  msg(log_info, "File compression: %d  flags:%u\n", file_compression, fl->header.flags);

  FILE *f = fopen(filename, "wb");
  if (!f) {
    msg(log_error, "Failed to open: %s\n", filename);
    goto failure;
  }
  size_t bytes_written = fwrite(&fl->header, 1, sizeof(fl->header), f);
  if (bytes_written != sizeof(fl->header)) {
    msg(log_error, "Failed to write file header\n");
    goto failure;
  }
  msg(log_debug, "Written file header\n");

  bytes_written = fwrite(&fl->stats, 1, sizeof(fl->stats), f);
  if (bytes_written != sizeof(fl->stats)) {
    msg(log_error, "Failed to write file stats\n");
    goto failure;
  }

  msg(log_debug, "Written file stats\n");

  for (int i = 0; i < fl->header.NumBlocks; ++i) {
    int result = write_block(f, fl->blocks[i]);
    if (result != 0) 
      goto failure;
  }

  fclose(f);
  return 0;
failure:
  return -1;
}


void handle_free_block(int blocknum, nf_block_p block) {
  free_block(&block);
}


nf_file_p new_file()
{
  return (nf_file_p)calloc(1, sizeof(nf_file_t));
}


void free_file(nf_file_p *file) {
  if (*file == NULL)
    return;
  nf_file_p fl = *file;
  *file = NULL;
  for_each_block(fl, &handle_free_block);
  free(fl);
}
