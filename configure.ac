#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([a2catalog],[0.1])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/a2catalog.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror])

# Checks for programs.
AC_PROG_CC
gl_EARLY



#All compiler-warning stuff came from coreutils
#which is
# Copyright (C) 1991, 1993-2010 Free Software Foundation, Inc.
# under GPL v3
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
                  [turn on lots of GCC warnings (for developers)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=no]
)

if test "$gl_gcc_warnings" = yes; then

gl_WARN_ADD([-Werror], [WERROR_CFLAGS])
AC_SUBST([WERROR_CFLAGS])

# Set up the list of the pointless, undesired warnings.
nw=
nw="$nw -Wsystem-headers"       # Don't let system headers trigger warnings
nw="$nw -Wundef"                # All compiler preprocessors support #if UNDEF
nw="$nw -Wtraditional"          # All compilers nowadays support ANSI C
nw="$nw -Wtraditional-conversion" # We always use prototypes
nw="$nw -Wconversion"           # These warnings usually don't point to mistakes.
nw="$nw -Wpadded"               # padded structs are OK
nw="$nw -Wsuggest-attribute=pure" # don't want it to suggest non-standard things
nw="$nw -Wsuggest-attribute=const" # don't want it to suggest non-standard things
nw="$nw -Wsuggest-attribute=noreturn" # don't want it to suggest non-standard things
# Enable all GCC warnings not in this list.
gl_MANYWARN_ALL_GCC([warnings])
gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
for w in $warnings
do
  gl_WARN_ADD([$w])
done
gl_WARN_ADD([-fdiagnostics-show-option])
AC_SUBST([WARN_CFLAGS])
fi
#end of coreutil stuff


# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([OS.h fcntl.h inttypes.h libintl.h limits.h locale.h netdb.h stddef.h stdint.h stdlib.h string.h sys/param.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Checks for library functions.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpagesize memset strchr strtol])
gl_INIT

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 lib/Makefile
                 src/Makefile
                 tests/Makefile])
AC_OUTPUT
