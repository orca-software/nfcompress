AC_INIT([nfcompress], [1.0.1], [j.r.versteegh@orca-st.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 doc/Makefile
 test/Makefile
])
AC_CONFIG_FILES([test/test.sh], [chmod +x test/test.sh])
AC_CHECK_LIB(lzo2, lzo1x_1_compress, [], [AC_MSG_ERROR([Unable to find LZO library])])
AC_CHECK_LIB(bz2, BZ2_bzBuffToBuffCompress, [], [AC_MSG_ERROR([Unable to find BZ2 library])])
AC_CHECK_LIB(lz4, LZ4_compress_default)
AC_CHECK_LIB(lzma, lzma_easy_buffer_encode)
AC_OPENMP

AC_PATH_PROG([DOXYGEN], [doxygen], [])
AM_CONDITIONAL([HAVE_DOXYGEN], test -z $DOXYGEN)
AM_COND_IF([HAVE_DOXYGEN], AC_CONFIG_FILES([doc/Doxyfile]))
AC_OUTPUT
