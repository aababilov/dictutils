AC_INIT([dicutils], [0.1])
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CXX

AM_CONFIG_HEADER(config.h)

AC_CHECK_FUNCS([strtok_r])

if test "$GCC" = "yes"; then
    CPPFLAGS="$CPPFLAGS -Wcast-align -Wno-uninitialized -Wall -Wformat-security"
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
])
AC_OUTPUT
