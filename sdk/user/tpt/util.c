/*
 * $Header: /usr/local/dslrepos/uClinux-dist/user/tpt/util.c,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $
 */

#include <stdio.h>

#include "tpt.h"

int
get_mhz(void)
{
	FILE *f = fopen("/proc/cpuinfo", "r");
	if (f == 0)
	{
		perror("can't open /proc/cpuinfo\n");
		exit(1);
	}

	for ( ; ; )
	{
		int mhz;
		int ret;
		char buf[1000];

		if (fgets(buf, sizeof(buf), f) == NULL)
		{
			fprintf(stderr, "cannot locate cpu MHz in /proc/cpuinfo\n");
			exit(1);
		}

		ret = sscanf(buf, "cpu MHz         : %d", &mhz);

		if (ret == 1)
		{
			fclose(f);
			return mhz;
		}
	}
}


