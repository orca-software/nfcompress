AC_INIT([nftools], [1.0.3], [j.r.versteegh@orca-st.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

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
AC_CONFIG_FILES([test/unittests.sh], [chmod +x test/unittests.sh])

AC_CHECK_LIB(
  lzo2,
  lzo1x_1_compress,
  [],
  [AC_MSG_ERROR([Unable to find LZO library.])]
)
AC_CHECK_LIB(
  bz2,
  BZ2_bzBuffToBuffCompress,
  [],
  [AC_MSG_WARN([BZ2 library not found. BZ2 (de)compression will not be available.])]
)
AC_CHECK_LIB(
  lz4,
  LZ4_compress_default,
  [],
  [AC_MSG_WARN([LZ4 library not found. LZ4 (de)compression will not be available.])]
)
AC_CHECK_LIB(
  lzma,
  lzma_easy_buffer_encode,
  [],
  [AC_MSG_WARN([LZMA library not found. LZMA (de)compression will not be available.])]
)

AC_OPENMP

AC_PATH_PROG([DOXYGEN], [doxygen], [])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], AC_CONFIG_FILES([doc/Doxyfile]))

# For unit tests
AC_PROG_CXX
AM_CONDITIONAL([HAVE_CXX], [test -n "$CXX"])
m4_include([m4/cppunit.m4])
AM_PATH_CPPUNIT([1.0.0], [], [AC_MSG_NOTICE([CppUnit not found. Unit tests will not be build.])])
AM_CONDITIONAL([HAVE_CPPUNIT], [test "x$CPPUNIT_CONFIG" != "xno"])

AC_OUTPUT
