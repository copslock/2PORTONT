# Generated automatically from Makefile.in by configure.
# Makefile for GNU termcap library.
# Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#### Start of system configuration section. ####

srcdir = .

CC = rsdk-linux-gcc
AR = ar
RANLIB = rsdk-linux-ranlib

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644

MAKEINFO = makeinfo

DEFS =  -DHAVE_STRING_H=1 -DHAVE_UNISTD_H=1  -DTERMCAP_FILE=\"$(termcapfile)\"

CFLAGS = -g

prefix = /usr/local
exec_prefix = ${prefix}

# Directory in which to install libtermcap.a.
libdir = $(exec_prefix)/lib

# Directory in which to install termcap.h.
includedir = $(prefix)/include

# Directory in which to optionally also install termcap.h,
# so compilers besides gcc can find it by default.
# If it is empty or not defined, termcap.h will only be installed in
# includedir. 
oldincludedir = /usr/include

# Directory in which to install the documentation info files.
infodir = $(prefix)/info

# File to which `install-data' should install the data file
# if --enable-install-termcap was given.
termcapfile = /etc/termcap

#### End of system configuration section. ####

SHELL = /bin/sh

SRCS = termcap.c tparam.c version.c
OBJS = termcap.o tparam.o version.o
HDRS = termcap.h
DISTFILES = $(SRCS) $(HDRS) ChangeLog COPYING README INSTALL NEWS \
termcap.src termcap.texi termcap.info* \
texinfo.tex Makefile.in configure configure.in mkinstalldirs install-sh

all:	libtermcap.a info

.c.o:
	$(CC) -c $(CPPFLAGS) $(DEFS) -I. -I$(srcdir) $(CFLAGS) $<

install: all installdirs 
	$(INSTALL_DATA) libtermcap.a $(libdir)/libtermcap.a
	-$(RANLIB) $(libdir)/libtermcap.a
	cd $(srcdir); $(INSTALL_DATA) termcap.h $(includedir)/termcap.h
	-cd $(srcdir); test -z "$(oldincludedir)" || \
	  $(INSTALL_DATA) termcap.h $(oldincludedir)/termcap.h
	cd $(srcdir); for f in termcap.info*; \
	do $(INSTALL_DATA) $$f $(infodir)/$$f; done

uninstall: 
	rm -f $(libdir)/libtermcap.a $(includedir)/termcap.h
	test -z "$(oldincludedir)" || rm -f $(oldincludedir)/termcap.h
	rm -f $(infodir)/termcap.info*

# These are separate targets to avoid trashing the user's existing
# termcap file unexpectedly.
install-data:
	$(INSTALL_DATA) ${srcdir}/termcap.src ${termcapfile}

uninstall-data:
	rm -f ${termcapfile}

installdirs:
	$(SHELL) ${srcdir}/mkinstalldirs $(bindir) $(libdir) \
	$(includedir) $(infodir)

Makefile: Makefile.in config.status
	$(SHELL) config.status
config.status: configure
	$(SHELL) config.status --recheck
configure: configure.in
	cd $(srcdir) && autoconf

libtermcap.a: $(OBJS)
	$(AR) rc $@ $(OBJS)
	-$(RANLIB) $@

info: termcap.info

termcap.info: termcap.texi
	$(MAKEINFO) $(srcdir)/termcap.texi --output=$@

TAGS:	$(SRCS)
	etags $(SRCS)

clean:
	rm -f *.a *.o core

mostlyclean: clean

distclean: clean
	rm -f Makefile config.status config.cache config.log

maintainer-clean: distclean
	@echo "This command is intended for maintainers to use;"
	@echo "rebuilding the deleted files requires makeinfo."
	rm -f TAGS *.info*

dist: $(DISTFILES)
	echo termcap-`sed -e '/version_string/!d' -e 's/[^0-9]*\([0-9a-z.]*\).*/\1/' -e q version.c` > .fname
	rm -rf `cat .fname`
	mkdir `cat .fname`
	ln $(DISTFILES) `cat .fname`
	tar chzf `cat .fname`.tar.gz `cat .fname`
	rm -rf `cat .fname` .fname
