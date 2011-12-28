AM_CFLAGS=$(WARN_CFLAGS) $(WERROR_CFLAGS)
AM_CPPFLAGS=$(all_includes) -I$(top_builddir)/lib -I$(top_srcdir)/lib
AM_LDFLAGS=$(all_libraries)
LDADD = $(top_builddir)/lib/liba2catalog.a



bin_PROGRAMS=a2catalog a2nibblize

a2catalog_SOURCES=\
a2catalog.c\
a2catalog_opt.c\
a2catalog_opt.h\
a2const.h\
ctest.c\
ctest.h

a2nibblize_SOURCES=\
a2nibblize.c\
a2nibblize_opt.c\
a2nibblize_opt.h\
nibblize_4_4.c\
nibblize_4_4.h\
nibblize_5_3.c\
nibblize_5_3.h\
nibblize_5_3_alt.c\
nibblize_5_3_alt.h\
nibblize_5_3_common.c\
nibblize_5_3_common.h\
nibblize_6_2.c\
nibblize_6_2.h\
a2const.h\
ctest.c\
ctest.h



check_SCRIPTS=unit_test.sh
TESTS=$(check_SCRIPTS)

unit_test.sh: .FORCE
	echo "set -e" >$@
	echo "./a2catalog --test" >>$@
	echo "./a2nibblize --test" >>$@
	chmod +x $@



CLEANFILES=unit_test.sh

.FORCE:
