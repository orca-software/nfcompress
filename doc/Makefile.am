#if HAVE_DOXYGEN

directory = $(top_srcdir)/doc/html

$(directory): Doxyfile
	$(DOXYGEN) Doxyfile

all-local: $(directory)
clean-local:
	rm -rf $(directory)

#endif
