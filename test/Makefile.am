check_SCRIPTS = test.sh unittests.sh
check_DATA = nfcapd.test1 nfcapd.test2
TESTS = $(check_SCRIPTS)
EXTRA_DIST = $(check_DATA)
DEPDIR = .testdeps

if HAVE_CXX
if HAVE_CPPUNIT

AM_CXXFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src

check_PROGRAMS = unittests

unittests_SOURCES = unittests.cpp ../src/file.c ../src/compress.c ../src/utils.c ../src/block.c ../src/record.c
unittests_CXXFLAGS = $(CPPUNIT_FLAGS) $(OPENMP_CFLAGS) $(AM_CXXFLAGS)
unittests_LDADD = $(CPPUNIT_LIBS)

endif
endif


.PHONY: test
test: check

clean-local:
	rm -f test.temp*
