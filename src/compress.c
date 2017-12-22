/**
 * \file compress.c
 * \brief Block (de)compression functions
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

#include <stdlib.h>
#include <string.h>

#include <lzo/lzo1x.h>

#include "config.h"

#ifdef HAVE_LIBBZ2
#include <bzlib.h>
#else
#define BZ_OK 0
#define BZ_OUTBUFF_FULL (-8)
#endif

#ifdef HAVE_LIBLZ4
#include <lz4.h>
#endif

#ifdef HAVE_LIBLZMA
#include <lzma.h>
#else
#define LZMA_OK 0
#define LZMA_BUF_ERROR 10
#endif

#include "types.h"
#include "utils.h"
#include "compress.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

int bz2_preset = DEFAULT_BZ2_PRESET;
int lzma_preset = DEFAULT_LZMA_PRESET;



int compress_none(const char* source, const size_t source_len, char* target, size_t* target_len) {
  size_t len = min(source_len, *target_len);
  memcpy(target, source, len);
  *target_len = len;
  return 0;
}

int compress_lzo(const char* source, const size_t source_len, char* target, size_t* target_len) {
  // Instantiate workmem. Not using a static buffer to avoid issues with threading
  void* wrkmem = malloc(LZO1X_1_MEM_COMPRESS);
  if (wrkmem == NULL) {
    msg(log_error, "Failed to allocate lzo work memory\n");
    return -1;
  }
  int result = lzo1x_1_compress((lzo_bytep)source, source_len, (lzo_bytep)target, target_len, wrkmem);
  free(wrkmem);
  return result;
}


int compress_bz2(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBBZ2
  return BZ2_bzBuffToBuffCompress(
      target,
      (unsigned int*)target_len,
      (char*)source,
      source_len,
      bz2_preset,  // * 100k: block size
      0,           // verbosity: be quiet
      30);         // workFactor, threshold for fallback to alt algo: defaults to 30
#else
  msg(log_error, "BZ2 support is not compiled in.\n");
  return -1;
#endif
}


int compress_lz4(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBLZ4
  int result = LZ4_compress_default(source, target, source_len, *target_len);
  if (result >= 0) {
    *target_len = result;
    result = 0;
  }
  return result;
#else
  msg(log_error, "LZ4 support is not compiled in.\n");
  return -1;
#endif
}


int compress_lzma(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBLZMA
  size_t target_pos = 0;
  int result = lzma_easy_buffer_encode(
      lzma_preset,       // preset: high values are expensive and don't bring much
      LZMA_CHECK_CRC64,  // data corruption check
      NULL,              // allocator. NULL means (malloc, free)
      (uint8_t*)source,
      source_len,
      (uint8_t*)target,
      &target_pos,
      *target_len);
  *target_len = target_pos;
  return result;
#else
  msg(log_error, "LZMA support is not compiled in.\n");
  return -1;
#endif
}


size_t compress_max_size_none(const size_t uncompressed_size) {
  return uncompressed_size;
}


size_t compress_max_size_lzo(const size_t uncompressed_size) {
  return uncompressed_size + uncompressed_size / 16 + 64 + 3;
}


size_t compress_max_size_bz2(const size_t uncompressed_size) {
  return 101 * uncompressed_size / 100 + 600;
}


size_t compress_max_size_lz4(const size_t uncompressed_size) {
#ifdef HAVE_LIBLZ4
  return LZ4_compressBound(uncompressed_size);
#else
  return uncompressed_size;
#endif
}


size_t compress_max_size_lzma(const size_t uncompressed_size) {
#ifdef HAVE_LIBLZMA
  return lzma_stream_buffer_bound(uncompressed_size);
#else
  return uncompressed_size;
#endif
}


int decompress_none(const char* source, const size_t source_len, char* target, size_t* target_len) {
  size_t len = min(source_len, *target_len);
  memcpy(target, source, len);
  *target_len = len;
  return 0;
}


int decompress_lzo(const char* source, const size_t source_len, char* target, size_t* target_len) {
  return lzo1x_decompress_safe(
      (lzo_bytep)source,
      source_len,
      (lzo_bytep)target,
      target_len,
      NULL);
}


int decompress_bz2(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBBZ2
  return BZ2_bzBuffToBuffDecompress(
      target,
      (unsigned int*)target_len,
      (char*)source,
      source_len,
      0,  // verbosity: be quiet
      0); // "small" for small memories, we don't expect to have that
#else
  msg(log_error, "BZ2 support is not compiled in.\n");
  return -1;
#endif
}


int decompress_lz4(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBLZ4
  int result = LZ4_decompress_safe(
      source,
      target,
      source_len,
      *target_len);
  if (result < 0)
    return -1;
  else
    *target_len = result;
  return 0;
#else
  msg(log_error, "LZ4 support is not compiled in.\n");
  return -1;
#endif
}


int decompress_lzma(const char* source, const size_t source_len, char* target, size_t* target_len) {
#ifdef HAVE_LIBLZMA
  uint64_t mem_limit = 0x04000000;
  size_t source_pos = 0, target_pos = 0;
  int result = lzma_stream_buffer_decode(
      &mem_limit,  // Max memory to be used
      0,           // Flags
      NULL,        // allocator. NULL means (malloc, free)
      (unsigned char*)source,
      &source_pos,
      source_len,
      (unsigned char*)target,
      &target_pos,
      *target_len);
  if (result == LZMA_OK) {
    *target_len = target_pos;
  }
  return result;
#else
  msg(log_error, "LZMA support is not compiled in.\n");
  return -1;
#endif
}


size_t decompress_suggested_size_none(const size_t compressed_size) {
  return compressed_size;
}


size_t decompress_suggested_size_lzo(const size_t compressed_size) {
  return 4 * compressed_size;
}


size_t decompress_suggested_size_bz2(const size_t compressed_size) {
  return 8 * compressed_size;
}


size_t decompress_suggested_size_lz4(const size_t compressed_size) {
  return 4 * compressed_size;
}


size_t decompress_suggested_size_lzma(const size_t compressed_size) {
  return 8 * compressed_size;
}


const transform_funs_t compress_funs_list[] = {
  {&compress_none, &compress_max_size_none, "None", 0},
  {&compress_lzo, &compress_max_size_lzo, "LZO", LZO_E_OK},
  {&compress_bz2, &compress_max_size_bz2, "BZ2", BZ_OK},
  {&compress_lz4, &compress_max_size_lz4, "LZ4", 0},
  {&compress_lzma, &compress_max_size_lzma, "LZMA", LZMA_OK}
};

static const transform_funs_t decompress_funs_list[] = {
  {&decompress_none, &decompress_suggested_size_none, "None", 0, -1},
  {&decompress_lzo, &decompress_suggested_size_lzo, "LZO", LZO_E_OK, LZO_E_OUTPUT_OVERRUN},
  {&decompress_bz2, &decompress_suggested_size_bz2, "BZ2", BZ_OK, BZ_OUTBUFF_FULL},
  {&decompress_lz4, &decompress_suggested_size_lz4, "LZ4", 0, -1},
  {&decompress_lzma, &decompress_suggested_size_lzma, "LZMA", LZMA_OK, LZMA_BUF_ERROR}
};

int compress(nf_block_t* block, compression_t compression) {
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }

  // Expected decompressed block
  if (block->compression != compressed_none) {
    msg(log_error, "Block is already compressed\n");
    return -1;
  }

  if (compression < compressed_none || compression >= compressed_term) {
    msg(log_error, "Unknown compression method: %d\n", compression);
    return -1;
  }

  if (compression == compressed_none) {
    // Nothing to be done
    return 0;
  }

  if (block->header.id == CATALOG_BLOCK) {
    // Catalog blocks should not be compressed
    return 0;
  }
  size_t size = block->header.size;
  size_t buffer_size = compress_funs_list[compression].size(size);
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate compression memory\n");
    goto failure;
  }
  int result = compress_funs_list[compression].transform(
      block->data,
      size,
      buffer,
      &buffer_size);
  if (result != compress_funs_list[compression].ok_result) {
    msg(log_error, "%s compression error: %d\n", compress_funs_list[compression].name, result);
    goto failure;
  }
  // Shrink buffer to compressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink compression buffer\n");
    goto failure;
  }
  free(block->data);
  block->header.size = buffer_size;
  block->compressed_size = buffer_size;
  block->compression = compression;
  block->data = buffer;
  return 0;
failure:
  free(buffer);
  return -1;
}


int decompress(nf_block_t* block) {
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }

  compression_t compression = block->compression;
  if (compression == compressed_none) {
    // The block is already decompressed
    return 0;
  }

  size_t size = block->header.size;
  size_t buffer_size = decompress_funs_list[compression].size(size);
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression buffer\n");
    return -1;
  }
  int result = 0;
  while ((result = decompress_funs_list[compression].transform(
      block->data,
      size,
      buffer,
      &buffer_size)) != decompress_funs_list[compression].ok_result) {
    if (result == decompress_funs_list[compression].buffer_error
        && buffer_size < 64 * size) {
      // Double size of decompression buffer if it was too small
      buffer_size *= 2;
      char* new_buffer = (char*)realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        msg(log_error, "Failed to grow decompression buffer\n");
        goto failure;
      }
      buffer = new_buffer;
    }
    else {
      msg(log_error, "%s decompression error: %d\n", decompress_funs_list[compression].name, result);
      goto failure;
    }
  }
  // Shrink buffer to decompressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink decompression buffer\n");
    goto failure;
  }
  // Free the original compressed data and update the block with
  // the new decompressed data
  free(block->data);
  block->header.size = buffer_size;
  block->uncompressed_size = buffer_size;
  block->compression = compressed_none;
  block->data = buffer;
  return 0;
failure:
  free(buffer);
  return -1;

}

void decompressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "Decompressing block: %d\n", blocknum);
  int result = decompress(block);
  block->status = result;
}


void lzo_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZO compressing block: %d\n", blocknum);
  block->status = compress(block, compressed_lzo);
}


void bz2_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "BZ2 compressing block: %d\n", blocknum);
  block->status = compress(block, compressed_bz2);
}


void lz4_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZ4 compressing block: %d\n", blocknum);
  block->status = compress(block, compressed_lz4);
}


void lzma_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZMA compressing block: %d\n", blocknum);
  block->status = compress(block, compressed_lzma);
}
