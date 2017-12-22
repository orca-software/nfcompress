#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "types.h"
#include "utils.h"
#include "compress.h"
#include "file.h"

const char usage[] = 
    "Usage: nfrecompress -c <none|lzo|bz2|lz4|lzma> [-l <0-9>] <nfdump files>\n"
    "  -c : compression method\n"
    "  -l : compression level (for bz2 and lzma)\n";

int main(int argc, char* argv[])
{
  compression_t compression;
  char opt = '\0';
  char* arg = NULL;
  int preset = -1;
  while ((opt = getopt(argc, argv, "hc:l:")) != -1) {
    switch (opt) {
      case 'c':
        arg = optarg;
        if (arg == NULL) {
          msg(log_error, "Expected argument to -c\n");
          return -1;
        }
        else if (strcmp(arg, "none") == 0) {
          compression = compressed_none;
        }
        else if(strcmp(arg, "lzo") == 0) {
          compression = compressed_lzo;
        }
        else if(strcmp(arg, "lz4") == 0) {
          compression = compressed_lz4;
        }
        else if(strcmp(arg, "bz2") == 0) {
          compression = compressed_bz2;
        }
        else if(strcmp(arg, "lzma") == 0) {
          compression = compressed_lzma;
        }
        else {
          msg(log_error, "Unexpected argument to -c: %s\n", optarg);
          return -1;
        }
        break;

      case 'h':
        printf(usage);
        return 0;

      case 'l':
        if (optarg == NULL) {
          msg(log_error, "Expected argument to -l\n");
          return -1;
        }
        if (strcmp(optarg, "0") == 0) {
          preset = 0;
        }
        else {
          preset = atoi(optarg);
          if (preset == 0) {
            msg(log_error, "Unexpected argument to -l: %s\n", optarg);
            return -1;
          }
        }
        break;

      default:
        msg(log_error, "Unexpected option: %c\n", arg);
        printf(usage);
        return -1;
    }
  }

  if (arg == NULL) {
    printf(usage);
    return -1;
  }

  int result = 0;
  for (int i = optind; i < argc; ++i) {
    char *filename = argv[i];
#ifndef _OPENMP
    // For the single core case it's faster to first read the file...
    nf_file_p fl = file_load(filename, NULL);
#else
    // , but for the multicore case: start decompressing while reading
    nf_file_p fl = file_load(filename, &decompressor);
#endif
    if (fl == NULL) {
      msg(log_error, "Failed to load file: %s\n", filename);
      return -1;
    }
#ifndef _OPENMP
    // ... and than decompress
    result = for_each_block(fl, &decompressor);
    if (result < 0) {
      msg(log_error, "Failed to decompress block in: %s\n", filename);
      return result;
    }
#endif
    switch(compression) {
      case compressed_none:
        break;
      case compressed_lzo:
        result = file_for_each_block(fl, &lzo_compressor);
        break;
      case compressed_lz4:
        result = file_for_each_block(fl, &lz4_compressor);
        break;
      case compressed_bz2:
        if (preset > 0)
          bz2_preset = preset;
        result = file_for_each_block(fl, &bz2_compressor);
        break;
      case compressed_lzma:
        if (preset >= 0)
          lzma_preset = preset;
        result = file_for_each_block(fl, &lzma_compressor);
        break;
      default:
        msg(log_error, "Unexpected compression method");
        result = -1;
    }
    if (result < 0) {
      msg(log_error, "Compression failed\n");
      result = -1;
    }
    else if (file_save_as(fl, filename) != 0) {
      msg(log_error, "Failed to save file: %s\n", filename);
      result = -1;
    }
    file_free(&fl);
  }
  msg(log_info, "Done\n");
  return result;
}
