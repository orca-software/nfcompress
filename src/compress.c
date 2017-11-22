#include <stdlib.h>

#include <lzo/lzo1x.h>
#include <bzlib.h>
#include <lz4.h>
#include <lzma.h>

#include "types.h"
#include "utils.h"
#include "compress.h"

int bz2_preset = BZ2_PRESET;
int lzma_preset = LZMA_PRESET;

int compress_lzo(nf_block_t* block)
{
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
  // Instantiate compression buffer and workmem
  void* wrkmem = malloc(LZO1X_1_MEM_COMPRESS);
  size_t size = block->header.size;
  size_t buffer_size = (size + size / 16 + 64 + 3);
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL || wrkmem == NULL) {
    msg(log_error, "Failed to allocate compression memory\n");
    goto failure;
  }
  int result = lzo1x_1_compress(
      (lzo_bytep)block->data,
      size,
      (lzo_bytep)buffer,
      &buffer_size,
      wrkmem);
  if (result != LZO_E_OK) {
    msg(log_error, "Failure with LZO compression\n");
    goto failure;
  }
  // Shrink buffer to compressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink compression buffer");
    goto failure;
  }
  free(block->data);
  block->header.size = buffer_size;
  block->compression = compressed_lzo;
  block->data = buffer;

  free(wrkmem);
  return 0;
failure:
  free(buffer);
  free(wrkmem);
  return -1;
}

int decompress_lzo(nf_block_t* block)
{
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }
  // Instantiate decompression buffer
  size_t buffer_size = 4 * block->header.size;
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression buffer\n");
    return -1;
  }
  int result = 0;
  while ((result = lzo1x_decompress_safe(
      (lzo_bytep)block->data,
      block->header.size,
      (lzo_bytep)buffer,
      &buffer_size, NULL)) != LZO_E_OK) {
    if (result == LZO_E_OUTPUT_OVERRUN) {
      // Double size of decompression buffer if it was too small
      buffer_size *= 2;
      char* new_buffer = (char*)realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        msg(log_error, "Failed to grow decompression buffer");
        goto failure;
      }
      buffer = new_buffer;
    }
    else {
      msg(log_error, "LZO decompression error: %d\n", result);
      goto failure;
    }
  }
  // Shrink buffer to decompressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink decompression buffer");
    goto failure;
  }
  // Free the original compressed data and update the block with
  // the new decompressed data
  free(block->data);
  block->header.size = buffer_size;
  block->compression = compressed_none;
  block->data = buffer;
  return 0;
failure:
  free(buffer);
  return -1;
}

int compress_bz2(nf_block_t* block)
{
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
  // Instantiate compression buffer and workmem
  size_t size = block->header.size;
  unsigned buffer_size = 101 * size / 100 + 600;
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate compression memory\n");
    goto failure;
  }
  int result = BZ2_bzBuffToBuffCompress(
      buffer,
      &buffer_size,
      block->data,
      block->header.size,
      bz2_preset,  // * 100k: block size
      0,           // verbosity: be quiet
      30           // workFactor, threshold for fallback to alt algo: defaults to 30
      );
  if (result != BZ_OK) {
    msg(log_error, "Failure with BZ2 compression\n");
    goto failure;
  }
  // Shrink buffer to compressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink compression buffer");
    goto failure;
  }
  free(block->data);
  block->header.size = buffer_size;
  block->compression = compressed_bz2;
  block->data = buffer;

  return 0;
failure:
  free(buffer);
  return -1;
}

int decompress_bz2(nf_block_t* block)
{
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }
  // Instantiate decompression buffer
  unsigned buffer_size = 8 * block->header.size;
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression buffer\n");
    return -1;
  }
  int result = 0;
  while ((result = BZ2_bzBuffToBuffDecompress(
      buffer,
      &buffer_size,
      block->data,
      block->header.size,
      0, // verbosity: be quiet
      0  // "small" for small memories, we don't expect to have that
      )) != BZ_OK) {
    if (result == BZ_OUTBUFF_FULL) {
      // Double size of decompression buffer if it was too small
      buffer_size *= 2;
      char* new_buffer = (char*)realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        msg(log_error, "Failed to grow decompression buffer");
        goto failure;
      }
      buffer = new_buffer;
    }
    else {
      msg(log_error, "BZ2 decompression error: %d\n", result);
      goto failure;
    }
  }
  // Shrink buffer to decompressed size
  buffer = (char*)realloc(buffer, buffer_size);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink decompression buffer");
    goto failure;
  }
  // Free the original compressed data and update the block with
  // the new decompressed data
  free(block->data);
  block->header.size = buffer_size;
  block->compression = compressed_none;
  block->data = buffer;
  return 0;
failure:
  free(buffer);
  return -1;
}

int compress_lz4(nf_block_t* block)
{
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
  int size = (int)block->header.size;
  int buffer_size = LZ4_compressBound(size);
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression memory\n");
    goto failure;
  }
  int result = LZ4_compress_default(block->data, buffer, size, buffer_size);
  if (result < 0) {
    msg(log_error, "Failure with LZ4 compression\n");
    goto failure;
  }
  // Shrink buffer to compressed size
  buffer = (char*)realloc(buffer, result);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink compression buffer");
    goto failure;
  }
  free(block->data);
  block->header.size = result;
  block->compression = compressed_lz4;
  block->data = buffer;
  msg(log_debug, "Compressed block LZ4. Size %d\n", result);
  return 0;
failure:
  free(buffer);
  return -1;
}


int decompress_lz4(nf_block_t* block)
{
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }
  // Instantiate decompression buffer
  int size = block->header.size;
  size_t buffer_size = 4 * size;
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression buffer\n");
    return -1;
  }
  int result = 0;
  while ((result = LZ4_decompress_safe(
      block->data,
      buffer,
      size,
      buffer_size)) < 0) {
    if (buffer_size < 64 * size) {
      // Double size of decompression buffer if it was too small
      buffer_size *= 2;
      char* new_buffer = (char*)realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        msg(log_error, "Failed to grow decompression buffer");
        goto failure;
      }
      buffer = new_buffer;
    }
    else {
      msg(log_error, "LZ4 decompression error: %d\n", result);
      return -1;
    }
  }
  // Shrink buffer to decompressed size
  buffer = (char*)realloc(buffer, result);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL)  {
    msg(log_error, "Failed to shrink decompression buffer");
    return -1;
  }
  // Free the original compressed data and update the block with
  // the new decompressed data
  free(block->data);
  block->header.size = result;
  block->compression = compressed_none;
  block->data = buffer;
  msg(log_debug, "Decompressed block LZ4. Size %d\n", result);
  return 0;
failure:
  free(buffer);
  return -1;
}

int compress_lzma(nf_block_t* block)
{
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
  size_t size = (int)block->header.size;
  size_t buffer_pos = 0;
  size_t buffer_size = lzma_stream_buffer_bound(size);
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression memory\n");
    goto failure;
  }
  int result;
  result = lzma_easy_buffer_encode(
      lzma_preset,       // preset: high values are expensive and don't bring much
      LZMA_CHECK_CRC64,  // data corruption check
      NULL,              // allocator. NULL means (malloc, free)
      (uint8_t*)block->data, 
      size,
      (uint8_t*)buffer, 
      &buffer_pos,
      buffer_size);
  if (result != LZMA_OK) {
    msg(log_error, "Failure with LZMA compression\n");
    goto failure;
  }
  // Shrink buffer to compressed size
  buffer = (char*)realloc(buffer, buffer_pos);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL) {
    msg(log_error, "Failed to shrink compression buffer to %d bytes", buffer_pos);
    goto failure;
  }
  free(block->data);
  block->header.size = buffer_pos;
  block->compression = compressed_lzma;
  block->data = buffer;
  msg(log_debug, "Compressed block LZMA. Size %d\n", result);
  return 0;
failure:
  free(buffer);
  return -1;
}

int decompress_lzma(nf_block_t* block)
{
  // Expected the block to have data
  if (block->data == NULL) {
    msg(log_error, "Block has no data\n");
    return -1;
  }
  // Instantiate decompression buffer
  size_t pos = 0, 
         size = block->header.size;
  size_t buffer_pos = 0, 
         buffer_size = 8 * size;
  char* buffer = (char*)malloc(buffer_size);
  if (buffer == NULL) {
    msg(log_error, "Failed to allocate decompression buffer\n");
    return -1;
  }
  int result = 0;
  uint64_t mem_limit = 0x04000000;
  while ((result = lzma_stream_buffer_decode(
      &mem_limit,  // Max memory to be used
      0,           // Flags
      NULL,        // allocator. NULL means (malloc, free)
      (unsigned char*)block->data,
      &pos,
      size,
      (unsigned char*)buffer,
      &buffer_pos,
      buffer_size)) != LZMA_OK) {
    if ((result == LZMA_BUF_ERROR) && (buffer_size < 64 * size)) {
      // Double size of decompression buffer if it was too small
      buffer_size *= 2;
      char* new_buffer = (char*)realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        msg(log_error, "Failed to grow decompression buffer");
        goto failure;
      }
      buffer = new_buffer;
    }
    else {
      msg(log_error, "LZMA decompression error: %d\n", result);
      goto failure;
    }
  }
  // Shrink buffer to decompressed size
  buffer = (char*)realloc(buffer, buffer_pos);
  // .. shouldn't really fail, but just in case
  if (buffer == NULL)  {
    msg(log_error, "Failed to shrink decompression buffer to %d", buffer_pos);
    return -1;
  }
  // Free the original compressed data and update the block with
  // the new decompressed data
  free(block->data);
  block->header.size = buffer_pos;
  block->compression = compressed_none;
  block->data = buffer;
  msg(log_debug, "Decompressed block LZMA. Size %d\n", result);
  return 0;
failure:
  free(buffer);
  return -1;
}

void decompressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "Decompressing block: %d\n", blocknum);
  int result = 0;
  switch (block->compression) {
    case compressed_none:
      break;
    case compressed_lzo:
      result = decompress_lzo(block);
      break;
    case compressed_bz2:
      result = decompress_bz2(block);
      break;
    case compressed_lz4:
      result = decompress_lz4(block);
      break;
    case compressed_lzma:
      result = decompress_lzma(block);
      break;
    default:
      // Unknown compression
      msg(log_error, "Unknown compression\n");
  }
  block->status = result;
}

void lzo_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZO compressing block: %d\n", blocknum);
  block->status = compress_lzo(block);
}


void bz2_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "BZ2 compressing block: %d\n", blocknum);
  block->status = compress_bz2(block);
}


void lz4_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZ4 compressing block: %d\n", blocknum);
  block->status = compress_lz4(block);
}


void lzma_compressor(const int blocknum, nf_block_t* block)
{
  msg(log_debug, "LZMA compressing block: %d\n", blocknum);
  block->status = compress_lzma(block);
}
