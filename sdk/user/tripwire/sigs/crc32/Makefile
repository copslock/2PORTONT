# $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $
#
# Makefile for crc32 wrapper
#
# Gene Kim
# Purdue University
# October 18, 1992
#

all:	crc32.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o .pure

crc32.o: ../../include/byteorder.h
