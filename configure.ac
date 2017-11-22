AC_INIT([nfcompress], [1.0], [j.r.versteegh@orca-st.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_CHECK_LIB(lzo2, lzo1x_1_compress)
AC_CHECK_LIB(bz2, BZ2_bzBuffToBuffCompress)
AC_CHECK_LIB(lz4, LZ4_compress_default)
AC_CHECK_LIB(lzma, lzma_easy_buffer_encode)
AC_OPENMP
AC_OUTPUT
