#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "compress.h"
#include "file.h"

void sep(const char c)
{
  const int ll = 42;
  char line[ll];
  for (int i = 0; i < ll; ++i)
    line[i] = c;
  line[ll - 1] = '\0';
  printf("%s\n", line);
}


int main(int argc, char* argv[])
{
  if (argc < 2) {
    msg(log_error, "Usage: nffileinfo <nfdump file(s)>\n");
    return -1;
  }

  size_t total_size = 0;
  int total_flows = 0;
  sep('=');
  printf("Number of files : %d\n", argc - 1);
  sep('=');

  for (int i = 1; i < argc; ++i) {
    char *filename = argv[i];
    sep('=');
    printf("File name        : %s\n", filename);
#ifdef _OPENMP
    nf_file_t* fl = file_load(filename, &decompressor);
#else
    // When not using OpenMP, it's faster to first read the whole file...
    nf_file_t* fl = file_load(filename, NULL);
#endif
    if (fl == NULL) {
      msg(log_error, "Failed to load file: %s\n", filename);
      return -1;
    }
    printf("File size        : %lu\n", fl->size);
    total_size += fl->size;
    printf("Number of blocks : %d\n", fl->header.NumBlocks);
    sep('=');
#ifndef _OPENMP
    // ... and only then start decompressing the blocks in it.
    for_each_block(fl, &decompressor);
    if (blocks_status(fl) < 0) {
      msg(log_error, "One or more blocks have an invalid status\n");
      return -1;
    }
#endif
    for (int i = 0; i < fl->header.NumBlocks; ++i) {
      printf("Block no          : %d\n", i);
      nf_block_p block = fl->blocks[i];
      printf("Block id          : %d\n", (int)block->header.id);
      printf("Number of records : %u\n", block->header.NumRecords);
      total_flows += block->header.NumRecords;
      printf("Compression       : %s\n", compress_funs_list[block->file_compression].name);
      printf("Uncompressed size : %lu\n", block->uncompressed_size);
      printf("Compressed size   : %lu\n", block->compressed_size);
      sep('-');
    }
  }
  
  sep('=');
  printf("Total number of records : %d\n", total_flows);
  printf("Total size              : %lu\n", total_size);
  sep('=');
  msg(log_debug, "Done\n");
  return 0;
}
