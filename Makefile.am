EXTRA_DIST = autogen.sh contrib

lib_LTLIBRARIES = libledger.la libamounts.la

libamounts_la_CPPFLAGS = -I$(srcdir)/src -I$(srcdir)/lib
libamounts_la_SOURCES  = \
	src/utils.cc       \
	src/times.cc       \
	src/mask.cc        \
	src/binary.cc      \
			   \
	src/amount.cc      \
	src/commodity.cc   \
	src/balance.cc     \
	src/value.cc       \
			   \
	src/token.cc       \
	src/parser.cc      \
	src/op.cc          \
	src/expr.cc        \
	src/scope.cc       \
	src/format.cc      \
	src/option.cc

if HAVE_EXPAT
libamounts_la_CPPFLAGS += -DHAVE_EXPAT=1
else
if HAVE_XMLPARSE
libamounts_la_CPPFLAGS += -DHAVE_XMLPARSE=1
endif
endif
if HAVE_LIBOFX
libamounts_la_CPPFLAGS += -DHAVE_LIBOFX=1
endif
if DEBUG
libamounts_la_CPPFLAGS += -DDEBUG_MODE
endif

libledger_la_CPPFLAGS = $(libamounts_la_CPPFLAGS)
libledger_la_LDFLAGS  = -release 3.0.0
libledger_la_SOURCES  =    \
	src/journal.cc     \
	src/item.cc        \
	src/entry.cc       \
	src/xact.cc        \
	src/account.cc     \
	src/iterators.cc   \
	src/compare.cc     \
			   \
	src/textual.cc     \
	src/cache.cc       \
	src/emacs.cc       \
	src/qif.cc	   \
	src/xml.cc	   \
	src/csv.cc         \
			   \
	src/session.cc     \
	src/report.cc      \
	src/filters.cc     \
	src/output.cc      \
	src/help.cc        \
			   \
	src/derive.cc      \
	src/reconcile.cc   \
	src/quotes.cc

if HAVE_EXPAT
libledger_la_SOURCES += src/gnucash.cc
else
if HAVE_XMLPARSE
libledger_la_SOURCES += src/gnucash.cc
endif
endif
if HAVE_LIBOFX
libledger_la_SOURCES += src/ofx.cc
endif

pkginclude_HEADERS =	  \
	acconf.h	  \
	src/system.hh	  \
	src/utils.h	  \
	src/flags.h	  \
	src/hooks.h	  \
	src/pushvar.h	  \
	src/error.h	  \
	src/times.h	  \
	src/mask.h	  \
	src/binary.h	  \
			  \
	src/amount.h	  \
	src/commodity.h	  \
	src/balance.h	  \
	src/balpair.h	  \
	src/value.h	  \
			  \
	src/token.h	  \
	src/parser.h	  \
	src/op.h	  \
	src/expr.h	  \
	src/scope.h	  \
	src/predicate.h	  \
	src/format.h	  \
	src/option.h	  \
			  \
	src/journal.h	  \
	src/item.h	  \
	src/entry.h	  \
	src/xact.h	  \
	src/account.h	  \
	src/iterators.h	  \
	src/compare.h	  \
			  \
	src/textual.h	  \
	src/cache.h	  \
	src/emacs.h	  \
	src/qif.h	  \
	src/xml.h	  \
	src/csv.h	  \
	src/gnucash.h	  \
	src/ofx.h	  \
			  \
	src/session.h	  \
	src/report.h	  \
	src/handler.h	  \
	src/filters.h	  \
	src/output.h	  \
	src/help.h	  \
			  \
	src/derive.h	  \
	src/reconcile.h	  \
	src/quotes.h	  \
			  \
	src/ledger.h	  \
	lib/fdstream.h    \
			  \
	python/pyledger.h \
	python/pyinterp.h

CLEANFILES =

if USE_PCH
nodist_libledger_la_SOURCES = src/system.hh.gch

BUILT_SOURCES = src/system.hh.gch
CLEANFILES   += src/system.hh.gch

$(srcdir)/src/system.hh.gch: $(srcdir)/src/system.hh $(top_builddir)/acconf.h
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	    $(libledger_la_CPPFLAGS) \
	    $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	    -g -o $@ $(srcdir)/src/system.hh
endif

######################################################################

bin_PROGRAMS	= ledger

ledger_CPPFLAGS	= $(libledger_la_CPPFLAGS)
ledger_SOURCES	= src/main.cc
ledger_LDADD	= $(LIBOBJS) libamounts.la libledger.la
ledger_LDFLAGS	= 

info_TEXINFOS	= doc/ledger.texi

dist_lisp_LISP	= lisp/ledger.el lisp/timeclock.el
ELCFILES	=
DISTCLEANFILES	= ledger.elc timeclock.elc
CLEANFILES     += TAGS

all_sources     = $(libamounts_la_SOURCES) \
		  $(libledger_la_SOURCES) \
		  $(libpyledger_la_SOURCES) \
		  python/pyledger.cc \
		  $(pkginclude_HEADERS)

TAGS: $(all_sources)
	@etags $(all_sources)
	@echo TAGS rebuilt.

######################################################################

if HAVE_BOOST_PYTHON

lib_LTLIBRARIES += libpyledger.la

libpyledger_la_SOURCES =       \
	python/py_amount.cc    \
	python/py_commodity.cc \
	python/py_times.cc     \
	python/py_utils.cc     \
	python/pyfstream.h     \
	python/pyinterp.cc     \
	python/pyutils.h       \
	python/setup.py

libpyledger_la_CPPFLAGS = $(libledger_la_CPPFLAGS) -I$(srcdir)/python

pyexec_PROGRAMS = ledger.so

clean-local:
	rm -fr build

ledger_so_SOURCES      = $(libamounts_la_SOURCES) $(libledger_la_SOURCES) \
			 $(libpyledger_la_SOURCES) python/pyledger.cc
ledger_so_DEPENDENCIES = $(lib_LTLIBRARIES)

PYLIBS  = ledger amounts pyledger gmp \
	  boost_system$(BOOST_SUFFIX)  \
	  boost_date_time$(BOOST_SUFFIX)  \
          boost_filesystem$(BOOST_SUFFIX) \
          boost_regex$(BOOST_SUFFIX)	  \
          boost_python$(BOOST_SUFFIX)

if HAVE_LIBOFX
PYLIBS += ofx
endif

ledger.so: $(ledger_so_SOURCES) $(ledger_so_DEPENDENCIES)
	BUILD_DIR=`cd $(top_builddir); pwd`; \
	(cd $(srcdir); \
	 CFLAGS="$(CPPFLAGS) -I$(srcdir) $(libpyledger_la_CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)" \
	 LDFLAGS="$(LDFLAGS) -L$$BUILD_DIR -L$$BUILD_DIR/.libs" \
	 ARCHFLAGS="$(ARCHFLAGS)" PYLIBS="$(PYLIBS)" \
	     $(PYTHON) python/setup.py build \
		--build-lib=$$BUILD_DIR/ --build-temp=$$BUILD_DIR/build -f)

endif

######################################################################

TESTS =

if HAVE_PYTHON
TESTS += RegressionTests
endif

if HAVE_CPPUNIT
TESTS += UnitTests
endif

if HAVE_BOOST_PYTHON
TESTS += PyUnitTests
endif

check_PROGRAMS = $(TESTS)

UnitTests_SOURCES =		 \
	test/UnitTests.cc	 \
	test/UnitTests.h	 \
	test/unit/t_utils.cc	 \
	test/unit/t_utils.h	 \
	test/unit/t_times.cc	 \
	test/unit/t_times.h	 \
	test/unit/t_commodity.cc \
	test/unit/t_commodity.h	 \
	test/unit/t_amount.cc	 \
	test/unit/t_amount.h	 \
	test/unit/t_balance.cc	 \
	test/unit/t_balance.h	 \
	test/unit/t_expr.cc	 \
	test/unit/t_expr.h

UnitTests_CPPFLAGS = -I$(srcdir)/test $(libledger_la_CPPFLAGS)
UnitTests_LDFLAGS  = $(LIBADD_DL)
UnitTests_LDADD    = $(lib_LTLIBRARIES) -lcppunit

EXTRA_DIST += test/python

PyUnitTests_SOURCES = test/__init__.py test/PyUnitTests.py test/UnitTests.py

ESC_python=`echo "$(PYTHON)" | sed 's/\//\\\\\//g'`
ESC_srcdir=`echo "$(srcdir)" | sed 's/\//\\\\\//g'`
ESC_builddir=`echo "$(top_builddir)" | sed 's/\//\\\\\//g'`
ESC_distdir=`echo "$(distdir)" | sed 's/\//\\\\\//g'`

# jww (2007-05-10): This rule will not be triggered on systems that
# define an EXEEXT.
PyUnitTests: $(srcdir)/test/PyUnitTests.py
	cat $(srcdir)/test/PyUnitTests.py \
	    | sed "s/%python%/$(ESC_python)/" \
	    | sed "s/%srcdir%/$(ESC_srcdir)/g" \
	    | sed "s/%builddir%/$(ESC_builddir)/g" > $@
	chmod 755 $@

RegressionTests_SOURCES = test/regress.py

EXTRA_DIST += test/regress

RegressionTests: $(srcdir)/test/regress.py
	echo "python $(srcdir)/test/regress.py $(top_builddir)/ledger$(EXEEXT) $(srcdir)/test/regress" > $@
	chmod 755 $@

if HAVE_VALGRIND
VALGRIND = valgrind
else
VALGRIND =
endif

fullcheck: check
	MallocGuardEdges=1	 \
	MallocScribble=1	 \
	MallocPreScribble=1	 \
	MallocCheckHeapStart=100 \
	MallocCheckHeapEach=100	 \
	DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib \
	$(VALGRIND) $(top_builddir)/UnitTests$(EXEEXT) --verify

######################################################################

EXTRA_DIST += doc/LICENSE doc/NEWS doc/README
EXTRA_DIST += doc/Doxyfile doc/ledger.pdf doc/refman.pdf

DISTCLEANFILES += doc/ledger.info doc/ledger.pdf \
		  Doxyfile.gen doc/Doxyfile.bak doc/refman.pdf

dist-hook:
	find $(distdir) -name .DS_Store -delete
	find $(distdir) -name .localized -delete
	find $(distdir)/doc -name .dirstamp -delete
	rm -fr $(distdir)/doc/latex \
	       $(distdir)/doc/Doxyfile.bak \
	       $(distdir)/doc/Doxyfile.gen
	cp -pR $(srcdir)/doc/html $(distdir)/doc
	chmod u+w $(distdir)/README
	cp -p $(srcdir)/doc/README $(distdir)/README

distclean-local:
	rm -fr doc/html doc/latex
	rm -f doc/refman.pdf

$(top_builddir)/Doxyfile.gen: $(srcdir)/doc/Doxyfile
	cat $(srcdir)/doc/Doxyfile \
	    | sed "s/%srcdir%/$(ESC_srcdir)/g" \
	    | sed "s/%builddir%/$(ESC_builddir)/g" > $@

$(top_builddir)/doc/html/index.html: $(top_builddir)/Doxyfile.gen
	doxygen $(top_builddir)/Doxyfile.gen

# The intention with the following rules is that all of the Doxygen
# documentation (both HTML and PDF) is built locally before distcheck is
# run, since it's quite possible that the user will not have a complete
# TeX + Doxygen + dot environment on their own system.

$(top_builddir)/doc/refman.pdf: $(top_builddir)/doc/html/index.html
	(cd $(top_builddir)/doc/latex && make)
	cp $(top_builddir)/doc/latex/refman.pdf $@

docs: pdf $(top_builddir)/doc/refman.pdf

libs:
	@echo Building dependency libs and installing in /usr/local/stow ...
	git submodule update --init
	(cd lib; make)

report: all
	-rm -fr build
	lcov -d $(shell pwd) --zerocounters
	-mkdir doc/report
	lcov -c -i -d $(shell pwd) -o doc/report/ledger_base.info
	make fullcheck
	lcov -c -d $(shell pwd) --checksum -o doc/report/ledger_test.info
	lcov -a doc/report/ledger_base.info \
	     -a doc/report/ledger_test.info -o doc/report/ledger_total.info
	lcov --extract doc/report/ledger_total.info '*src/ledger/*' \
	     -o doc/report/ledger_cov.info
	genhtml -o doc/report doc/report/ledger_cov.info
	@echo Coverage reported generated\; now open doc/report/index.html

######################################################################

copy-sources:
	-mkdir /tmp/ledger
	sudo rsync -av --delete \
	    --exclude='/lib/*/' \
	    --exclude=/.libs/ \
	    --exclude=/.deps/ \
	    ./ /tmp/ledger/
	(cd /tmp/ledger; sudo git clean -x -d -f)

release: copy-sources
	(cd /tmp/ledger; ./acprep --release --build -j3)

release-distcheck: copy-sources
	(cd /tmp/ledger; ./acprep --release --build -j3 distcheck)

# Makefile.am ends here
