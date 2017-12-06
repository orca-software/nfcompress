#if HAVE_DOXYGEN

directory = $(top_srcdir)/doc

html_doc: Doxyfile
	$(DOXYGEN) Doxyfile

all-local: html_doc

clean-local:
	rm -rf html
	rm -rf man
	rm -rf xml

#endif
