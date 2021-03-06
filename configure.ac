# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

m4_include([version.m4])

AC_INIT([ledger],[VERSION_NUMBER],[johnw@newartisans.com])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADER([acconf.h])
AM_INIT_AUTOMAKE([dist-bzip2 foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_CHECK_PROG([VALGRIND_TEST], [valgrind], [true], [false])
AM_CONDITIONAL(HAVE_VALGRIND, test x$VALGRIND_TEST = xtrue)

# Checks for emacs lisp path
AM_PATH_LISPDIR

# Check for options
AC_ARG_ENABLE(debug,
  [  --enable-debug          Turn on debugging],
  [case "${enableval}" in
    yes) debug=true ;;
    no)  debug=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
  esac],[debug=false])

AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

AC_ARG_ENABLE(pch,
  [  --enable-pch            Use GCC 4.x pre-compiled headers],
  [case "${enableval}" in
    yes) pch=true ;;
    no)  pch=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-pch) ;;
  esac],[pch=false])

AM_CONDITIONAL(USE_PCH, test x$pch = xtrue)

AC_ARG_WITH(boost-suffix,
  [  --with-boost-suffix=X   Append X to the Boost library names],
  [BOOST_SUFFIX="${withval}"],
  [BOOST_SUFFIX=""])

AC_SUBST([BOOST_SUFFIX], $BOOST_SUFFIX)

# check if UNIX pipes are available
AC_CACHE_CHECK(
  [if pipes can be used],
  [pipes_avail_cv_],
  [AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
	[[#include <sys/types.h>
	  #include <sys/wait.h>
	  #include <unistd.h>
	  #include <stdlib.h>
	  #include <string.h>
	  #include <stdio.h>]],
	[[int status, pfd[2];
	  status = pipe(pfd);
	  status = fork();
	  if (status < 0) {
	    ;
	  } else if (status == 0) {
	    char *arg0;

	    status = dup2(pfd[0], STDIN_FILENO);

	    close(pfd[1]);
	    close(pfd[0]);

	    execlp("", arg0, (char *)0);
	    perror("execl");
	    exit(1);
	  } else {
	    close(pfd[0]);
	  }]])],
     [pipes_avail_cv_=true],
     [pipes_avail_cv_=false])
   AC_LANG_POP])

if [test x$pipes_avail_cv_ = xtrue ]; then
  AC_DEFINE([HAVE_UNIX_PIPES], [1], [Whether UNIX pipes are available])
fi

# check for gmp
AC_CACHE_CHECK(
  [if libgmp is available],
  [libgmp_avail_cv_],
  [libgmp_save_libs=$LIBS
   LIBS="-lgmp $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <gmp.h>]], [[mpz_t bar;
      mpz_init(bar);
      mpz_clear(bar);]])],[libgmp_avail_cv_=true],[libgmp_avail_cv_=false])
   AC_LANG_POP
   LIBS=$libgmp_save_libs])

if [test x$libgmp_avail_cv_ = xtrue ]; then
  LIBS="-lgmp $LIBS"
else
  AC_MSG_FAILURE("Could not find gmp library (set CPPFLAGS and LDFLAGS?)")
fi

# check for expat or xmlparse
AC_CACHE_CHECK(
  [if libexpat is available],
  [libexpat_avail_cv_],
  [libexpat_save_libs=$LIBS
   LIBS="-lexpat $LIBS"
   AC_LANG_PUSH(C++)
   AC_TRY_LINK(
     [#include <stdio.h>
	extern "C" {
	#include <expat.h>           // expat XML parser
	}],
     [XML_Parser parser = XML_ParserCreate(NULL);
	return parser != NULL;],
     [libexpat_avail_cv_=true],
     [libexpat_avail_cv_=false])
   AC_LANG_POP
   LIBS=$libexpat_save_libs])

if [test x$libexpat_avail_cv_ = xtrue ]; then
  AM_CONDITIONAL(HAVE_EXPAT, true)
  LIBS="-lexpat $LIBS"
else
  AM_CONDITIONAL(HAVE_EXPAT, false)
fi

if [test x$libexpat_avail_cv_ = xfalse ]; then
  AC_CACHE_CHECK(
    [if libxmlparse is available],
    [libxmlparse_avail_cv_],
    [libxmlparse_save_libs=$LIBS
     LIBS="-lxmlparse -lxmltok $LIBS"
     AC_LANG_PUSH(C++)
     AC_TRY_LINK(
	 [#include <stdio.h>
	extern "C" {
	#include <xmlparse.h>           // expat XML parser
	}],
	 [XML_Parser parser = XML_ParserCreate(NULL);
	return parser != NULL;],
	 [libxmlparse_avail_cv_=true],
	 [libxmlparse_avail_cv_=false])
     AC_LANG_POP
     LIBS=$libxmlparse_save_libs])

  if [test x$libxmlparse_avail_cv_ = xtrue ]; then
    AM_CONDITIONAL(HAVE_XMLPARSE, true)
    LIBS="-lxmlparse -lxmltok $LIBS"
  else
    AM_CONDITIONAL(HAVE_XMLPARSE, false)
  fi
else
  AM_CONDITIONAL(HAVE_XMLPARSE, false)
fi

# check for boost_regex
AC_CACHE_CHECK(
  [if boost_regex is available],
  [boost_regex_avail_cv_],
  [boost_regex_save_libs=$LIBS
   LIBS="-lboost_regex$BOOST_SUFFIX $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
       [[#include <boost/regex.hpp>]],
       [[boost::regex foo_regexp("Hello, world!");]])],
     [boost_regex_avail_cv_=true],
     [boost_regex_avail_cv_=false])
   AC_LANG_POP
   LIBS=$boost_regex_save_libs])

if [test x$boost_regex_avail_cv_ = xtrue ]; then
  LIBS="-lboost_regex$BOOST_SUFFIX $LIBS"
else
  AC_MSG_FAILURE("Could not find boost_regex library (set CPPFLAGS and LDFLAGS?)")
fi

# check for boost_date_time
AC_CACHE_CHECK(
  [if boost_date_time is available],
  [boost_date_time_cpplib_avail_cv_],
  [boost_date_time_save_libs=$LIBS
   LIBS="-lboost_date_time$BOOST_SUFFIX $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
	[[#include <boost/date_time/posix_time/posix_time.hpp>
	  #include <boost/date_time/gregorian/gregorian.hpp>
	  #include <boost/date_time/local_time_adjustor.hpp>
	  #include <boost/date_time/time_duration.hpp>

	  using namespace boost::posix_time;
	  using namespace boost::date_time;

	  #include <ctime>

	  inline ptime time_to_system_local(const ptime& when) {
	    struct std::tm tm_gmt = to_tm(when);
	    return from_time_t(mktime(&tm_gmt));
	  }]],
	[[ptime t10 = ptime(boost::gregorian::from_string("2007-01-15"),
			    ptime::time_duration_type());

	  ptime t12 = time_to_system_local(t10);

	  return t10 != t12;]])],
     [boost_date_time_cpplib_avail_cv_=true],
     [boost_date_time_cpplib_avail_cv_=false])
   AC_LANG_POP
   LIBS=$boost_date_time_save_libs])

if [test x$boost_date_time_cpplib_avail_cv_ = xtrue ]; then
  LIBS="-lboost_date_time$BOOST_SUFFIX $LIBS"
else
  AC_MSG_FAILURE("Could not find boost_date_time library (set CPPFLAGS and LDFLAGS?)")
fi

# check for boost_filesystem
AC_CACHE_CHECK(
  [if boost_filesystem is available],
  [boost_filesystem_cpplib_avail_cv_],
  [boost_filesystem_save_libs=$LIBS
   LIBS="-lboost_filesystem$BOOST_SUFFIX -lboost_system$BOOST_SUFFIX $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
	[[#include <boost/filesystem/path.hpp>]],
	[[boost::filesystem::path this_path("Hello");]])],
     [boost_filesystem_cpplib_avail_cv_=true],
     [boost_filesystem_cpplib_avail_cv_=false])
   AC_LANG_POP
   LIBS=$boost_filesystem_save_libs])

if [test x$boost_filesystem_cpplib_avail_cv_ = xtrue ]; then
  LIBS="-lboost_filesystem$BOOST_SUFFIX -lboost_system$BOOST_SUFFIX $LIBS"
else
  AC_MSG_FAILURE("Could not find boost_filesystem library (set CPPFLAGS and LDFLAGS?)")
fi

# check for libofx
AC_CACHE_CHECK(
  [if libofx is available],
  [libofx_avail_cv_],
  [libofx_save_libs=$LIBS
   LIBS="-lofx $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
	  [[#include <libofx.h>]],
	  [[LibofxContextPtr libofx_context = libofx_get_new_context();]])],
     [libofx_avail_cv_=true],
     [libofx_avail_cv_=false])
   AC_LANG_POP
   LIBS=$libofx_save_libs])

if [test x$libofx_avail_cv_ = xtrue ]; then
  AM_CONDITIONAL(HAVE_LIBOFX, true)
  LIBS="-lofx $LIBS"
else
  AM_CONDITIONAL(HAVE_LIBOFX, false)
fi

# check for Python
AM_PATH_PYTHON(2.4,, :)
if [test "$PYTHON" != :]; then
  AM_CONDITIONAL(HAVE_PYTHON, true)
  AC_CACHE_CHECK(
    [if boost_python is available],
    [boost_python_cpplib_avail_cv_],
    [boost_python_save_libs=$LIBS
     LIBS="-lboost_python$BOOST_SUFFIX -lpython$PYTHON_VERSION $LIBS"
     AC_LANG_PUSH(C++)
     AC_LINK_IFELSE(
	 [AC_LANG_PROGRAM(
	    [[#include <boost/python.hpp>
	      using namespace boost::python;
	      class foo {};
	      BOOST_PYTHON_MODULE(samp) {
		class_< foo > ("foo") ;
	      }]],
	    [[return 0]])],
	 [boost_python_cpplib_avail_cv_=true],
	 [boost_python_cpplib_avail_cv_=false])
     AC_LANG_POP
     LIBS=$boost_python_save_libs])

  if [test x$boost_python_cpplib_avail_cv_ = xtrue ]; then
    AM_CONDITIONAL(HAVE_BOOST_PYTHON, true)
    LIBS="-lboost_python$BOOST_SUFFIX -lpython$PYTHON_VERSION $LIBS"
  else
    AM_CONDITIONAL(HAVE_BOOST_PYTHON, false)
  fi
else
  AM_CONDITIONAL(HAVE_PYTHON, false)
  AM_CONDITIONAL(HAVE_BOOST_PYTHON, false)
fi

# check for CppUnit
AC_CACHE_CHECK(
  [if cppunit is available],
  [cppunit_avail_cv_],
  [cppunit_save_libs=$LIBS
   LIBS="-lcppunit $LIBS"
   AC_LANG_PUSH(C++)
   AC_LINK_IFELSE(
     [AC_LANG_PROGRAM(
	[[#include <cppunit/CompilerOutputter.h>
	  #include <cppunit/TestResult.h>
	  #include <cppunit/TestResultCollector.h>
	  #include <cppunit/TestRunner.h>
	  #include <cppunit/TextTestProgressListener.h>
	  #include <cppunit/BriefTestProgressListener.h>
	  #include <cppunit/XmlOutputter.h>
	  #include <cppunit/extensions/TestFactoryRegistry.h>]],
	[[CPPUNIT_NS::TestResult controller;
	  CPPUNIT_NS::TestResultCollector result;]])],
     [cppunit_avail_cv_=true],
     [cppunit_avail_cv_=false])
   AC_LANG_POP
   LIBS=$cppunit_save_libs])

if [test x$cppunit_avail_cv_ = xtrue ]; then
  AM_CONDITIONAL(HAVE_CPPUNIT, true)
else
  AM_CONDITIONAL(HAVE_CPPUNIT, false)
fi

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STAT
AC_CHECK_HEADERS([langinfo.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_MKTIME
#AC_FUNC_STAT
#AC_FUNC_STRFTIME
AC_CHECK_FUNCS([access realpath getpwuid getpwnam nl_langinfo])

# Pepare the Makefiles
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
