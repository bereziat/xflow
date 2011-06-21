AC_PREREQ(2.59)

AC_INIT(configure.ac)
dnl Init some automake stuff. Evaluates @PACKAGE@ and @VERSION@ macros
AM_INIT_AUTOMAKE(xflow,2.3.1)
dnl ...
AM_CONFIG_HEADER(config.h)
dnl A macro makes autotools happy with CVS
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Check for an Ansi C compiler ...
AM_PROG_CC_STDC
dnl ... and Posix. Is it necessary ?
AC_ISC_POSIX

# Checks for libraries.

dnl Gtk lib
pkg_modules="gtk+-2.0 >= 2.4.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])

dnl edges est juste utilis� pour un exemple de client
dnl xflow, -> mettre un truc dans config et c'est tout.
dnl AC_CHECK_LIB([edges], [main])

dnl Check INRIMAGE
AC_PATH_INRIMAGE
AC_CHECK_HEADER(inrimage/image.h,,exit)
AC_CHECK_LIB(inrimage,inr_init,,exit)

# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [cos])

dnl ajouter la d�tection de GLADE (pour devel)
AC_CHECK_PROG(GLADE, glade-2, yes)
AM_CONDITIONAL(GLADE, test x$GLADE = xyes)

dnl Check Doxygen
AC_CHECK_PROG(DOXYGEN,doxygen,yes,no)
AC_CHECK_PROG(FIG2DEV,fig2dev,yes,no)
AM_CONDITIONAL(BUILD_DOX,test x$DOXYGEN = xyes -a x$FIG2DEV = xyes)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor sqrt strcasecmp strdup strrchr strstr])

dnl r�pertoires provisoirement retir�s
dnl scripts/Makefile
dnl doc/Makefile

AC_OUTPUT([
Makefile
src/inrimage/Makefile
src/lib/Makefile
src/xflow/Makefile
src/utils/Makefile
scripts/Makefile
src/Makefile
src/img/Makefile
src/examples/c/Makefile
])

if test x$FIG2DEV = xno ; then
   cat <<EOF
WARNING: fig2dev is missing. fig2dev is used by xflow to export
         xflow images to various format such as postscript or bipmap and
         to build icons and documentation.
         fig2dev is a part of xfig/transfig software. 
EOF
fi