AC_PREREQ([2.52])

AC_INIT(rtcom-accounts-plugins, 4.141)

LT_CURRENT=1
LT_REVISION=0
LT_AGE=0
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE


AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(ACCOUNTS, rtcom-accounts-widgets libhildonmime)
PKG_CHECK_MODULES(GLADE, libglade-2.0)

dnl Localization
GETTEXT_PACKAGE=osso-applet-accounts
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "${GETTEXT_PACKAGE}", [gettext package])

dnl Installation directories
AC_SUBST(librtcom_accountsdir)

pluginlibdir="`$PKG_CONFIG --variable=pluginlibdir libaccounts`"
AC_SUBST(pluginlibdir)

AC_ARG_ENABLE(cast-checks,  [  --disable-cast-checks   compile with GLIB cast checks disabled],[cchecks=${enableval}],cchecks=yes)
if test "x$cchecks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CAST_CHECKS"
fi

AC_ARG_ENABLE(asserts,      [  --disable-asserts       compile with GLIB assertions disabled],[asserts=${enableval}],asserts=yes)
if test "x$asserts" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_ASSERTS"
fi

AC_ARG_ENABLE(checks,       [  --disable-checks        compile with GLIB checks disabled],[checks=${enableval}],checks=yes)
if test "x$checks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CHECKS"
fi

AC_ARG_ENABLE(debug,     [  --enable-debug          enable printing of debug messages],[ddebug=${enableval}],ddebug=no)
if test "x$ddebug" != "xyes"; then
    CFLAGS="$CFLAGS -DG_DEBUG_DISABLE"
fi

AC_OUTPUT([
	Makefile
	src/Makefile
	data/Makefile
])
