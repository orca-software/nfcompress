#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "compress.h"
#include "file.h"

int main(int argc, char* argv[])
{
  if (argc < 2) {
    msg(log_error, "Usage: nfdecompress <nfdump file(s)>\n");
    return -1;
  }

  for (int i = 1; i < argc; ++i) {
    char *filename = argv[i];
#ifdef _OPENMP
    nf_file_t* fl = load(filename, &decompressor);
#else
    // When not using OpenMP, it's faster to first read the whole file...
    nf_file_t* fl = load(filename, NULL);
#endif
    if (fl == NULL) {
      msg(log_error, "Failed to load file: %s\n", filename);
      return -1;
    }
#ifndef _OPENMP
    // ... and only then start decompressing the blocks in it.
    for_each_block(fl, &decompressor);
    if (blocks_status(fl) < 0) {
      msg(log_error, "One or more blocks have an invalid status\n");
      return -1;
    }
#endif
    for (int j = 0; j < fl->header.NumBlocks; ++j) {
      fwrite(fl->blocks[j]->data, 1, fl->blocks[j]->header.size, stdout);
    }
  }
  msg(log_debug, "Done\n");
  return 0;
}
