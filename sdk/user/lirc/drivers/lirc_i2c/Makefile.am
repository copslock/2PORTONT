## $Id: Makefile.am,v 1.1.1.1 2003/08/18 05:41:29 kaohj Exp $

## Process this file with automake to produce Makefile.in

EXTRA_DIST = 

## this is so that Automake includes the C compiling definitions, and
## includes the source files in the distribution.
EXTRA_PROGRAMS = automake_dummy
automake_dummy_SOURCES = lirc_i2c.c

## there is no *just* object file support in automake.  This is close enough
module_DATA = lirc_i2c.o

# where the kernel sources are located
KERNEL_LOCATION=@kerneldir@

# some magic for using linux kernel settings
# when compiling module(s)
CC           = @kernelcc@
MI_OBJS      = $(module_DATA)
EXTRA_CFLAGS = $(DEFS)

$(module_DATA): ${automake_dummy_SOURCES} $(top_srcdir)/config.h ../lirc.h
  ifndef MAKING_MODULES
	DIR=`pwd`; (cd $(KERNEL_LOCATION); make SUBDIRS=$$DIR obj-m=$(module_DATA) modules)
  endif
# the spaces above are necessary because automake otherwise will complain
# and even worse, it will delete the line

install-exec-local: mkdev
uninstall-local: rmdev

mkdev:
	test -c @devdir@/lirc || @mknod@ @devdir@/lirc c @lirc_major@ 0

rmdev:
	test -c @devdir@/lirc && $(RM) @devdir@/lirc

install-data-local:
	-@depmod@ -a

CLEANFILES = $(module_DATA) *~ \#*\#

include $(KERNEL_LOCATION)/Rules.make
