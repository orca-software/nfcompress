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

#include "types.h"
#include "utils.h"

void read_block(FILE *f, nf_block_t* block)
{
  size_t bytes_read = fread(&block->header, 1, sizeof(block->header), f);
  if (bytes_read != sizeof(block->header)) {
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
  return;
failure:
  free(block->data);
  block->data = NULL;
  block->header.size = 0;
  block->status = -1;
}


int write_block(FILE *f, nf_block_t* block)
{
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


void for_each_block(nf_file_t* fl, block_handler_p handle_block)
{
  #pragma omp parallel for
  for (int i = 0; i < fl->header.NumBlocks; ++i) {
    handle_block(i, &fl->blocks[i]);
  }
}


nf_file_t* load(const char* filename, block_handler_p handle_block)
{
  nf_file_t* fl = (nf_file_t*)malloc(sizeof(nf_file_t));
  if (fl == NULL) {
    msg(log_error, "Failed to allocate file buffer\n");
    return NULL;
  }

  msg(log_info, "Reading %s\n", filename);

  FILE *f = fopen(filename, "rb");
  if (!f) {
    msg(log_error, "Failed to open: %s\n", filename);
    goto cleanup_result;
  }

  size_t bytes_read = fread(&fl->header, 1, sizeof(fl->header), f);
  if (bytes_read != sizeof(fl->header)) {
    msg(log_error, "Failed to read file header\n");
    goto cleanup_file;
  }
  msg(log_debug, "Read file header\n");

  bytes_read = fread(&fl->stats, 1, sizeof(fl->stats), f);
  if (bytes_read != sizeof(fl->stats)) {
    msg(log_error, "Failed to read file stats\n");
    goto cleanup_file;
  }

  msg(log_debug, "Read file stats\n");

  size_t blocks_size = fl->header.NumBlocks * sizeof(nf_block_t);
  nf_file_t* new_fl = (nf_file_t*)realloc(fl, sizeof(nf_file_t) + blocks_size);
  if (new_fl == NULL) {
    msg(log_error, "Failed to re-allocate file bufferi\n");
    goto cleanup_file;
  }
  fl = new_fl;
  memset(&fl->blocks, 0, blocks_size);

  compression_t file_compression =
      fl->header.flags & FLAG_LZO_COMPRESSED ? compressed_lzo : 
      fl->header.flags & FLAG_BZ2_COMPRESSED ? compressed_bz2 :
      fl->header.flags & FLAG_LZ4_COMPRESSED ? compressed_lz4 :
      fl->header.flags & FLAG_LZMA_COMPRESSED ? compressed_lzma :
        compressed_none;
  msg(log_info, "File compression: %d  flags:%u\n", file_compression, fl->header.flags);

  #pragma omp parallel
  #pragma omp master
  for (int i = 0; i < fl->header.NumBlocks; ++i) {
    read_block(f, &fl->blocks[i]);
    fl->blocks[i].compression = file_compression;
    if (handle_block != NULL) {
      #pragma omp task
      handle_block(i, &fl->blocks[i]);
    }
  }

  fclose(f);
  return fl;
cleanup_file:
  fclose(f);
cleanup_result:
  free(fl);
  return NULL;
}


int save(const char* filename, nf_file_t* fl)
{
  msg(log_info, "Writing %s\n", filename);

  if (fl->header.NumBlocks == 0) {
    msg(log_error, "Not saving empty file");
    return -1;
  }

  compression_t file_compression = fl->blocks[0].compression;
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
    int result = write_block(f, &fl->blocks[i]);
    if (result != 0) 
      goto failure;
  }

  fclose(f);
  return 0;
failure:
  return -1;
}

void free_block(int blocknum, nf_block_t* block) {
  free(block->data);
  block->data = NULL;
  block->header.size = 0;
}

void free_file(nf_file_t* fl) {
  for_each_block(fl, &free_block);
  free(fl);
}
