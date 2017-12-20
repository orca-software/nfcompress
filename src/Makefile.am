HDRS = types.h utils.h compress.h file.h block.h
SRCS = utils.c compress.c file.c block.c

AM_CFLAGS = $(OPENMP_CFLAGS)

bin_PROGRAMS = nfdecompress nfrecompress nffileinfo

nfdecompress_SOURCES = nfdecompress.c $(SRCS) $(HDRS)

nfrecompress_SOURCES = nfrecompress.c $(SRCS) $(HDRS)

nffileinfo_SOURCES = nffileinfo.c $(SRCS) $(HDRS)
