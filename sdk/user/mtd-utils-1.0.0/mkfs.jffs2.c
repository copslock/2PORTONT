/* vi: set sw=4 ts=4: */
/*
 * Build a JFFS2 image in a file, from a given directory tree.
 *
 * Copyright 2001, 2002 Red Hat, Inc.
 *           2001 David A. Schleef <ds@lineo.com>
 *           2002 Axis Communications AB
 *           2001, 2002 Erik Andersen <andersen@codepoet.org>
 *           2004 University of Szeged, Hungary
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Cross-endian support added by David Schleef <ds@schleef.org>.
 *
 * Major architectural rewrite by Erik Andersen <andersen@codepoet.org>
 * to allow support for making hard links (though hard links support is
 * not yet implemented), and for munging file permissions and ownership
 * on the fly using --faketime, --squash, --devtable.   And I plugged a
 * few memory leaks, adjusted the error handling and fixed some little
 * nits here and there.
 *
 * I also added a sample device table file.  See device_table.txt
 *  -Erik, September 2001
 *
 * Cleanmarkers support added by Axis Communications AB
 *
 * Rewritten again.  Cleanly separated host and target filsystem
 * activities (mainly so I can reuse all the host handling stuff as I
 * rewrite other mkfs utils).  Added a verbose option to list types
 * and attributes as files are added to the file system.  Major cleanup
 * and scrubbing of the code so it can be read, understood, and
 * modified by mere mortals.
 *
 *  -Erik, November 2002
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <libgen.h>
#include <ctype.h>
#include <time.h>
#include <getopt.h>
#include <byteswap.h>
#define crc32 __complete_crap
#include <zlib.h>
#undef crc32
#include "crc32.h"

/* Do not use the wierd XPG version of basename */
#undef basename

//#define DMALLOC
//#define mkfs_debug_msg    error_msg
#define mkfs_debug_msg(a...)	{ }
#define min(x,y) ({ typeof((x)) _x = (x); typeof((y)) _y = (y); (_x>_y)?_y:_x; })

#define PAD(x) (((x)+3)&~3)

struct filesystem_entry {
	char *name;					/* Name of this directory (think basename) */
	char *path;					/* Path of this directory (think dirname) */
	char *fullname;				/* Full name of this directory (i.e. path+name) */
	char *hostname;				/* Full path to this file on the host filesystem */
	struct stat sb;				/* Stores directory permissions and whatnot */
	char *link;					/* Target a symlink points to. */
	struct filesystem_entry *parent;	/* Parent directory */
	struct filesystem_entry *prev;	/* Only relevant to non-directories */
	struct filesystem_entry *next;	/* Only relevant to non-directories */
	struct filesystem_entry *files;	/* Only relevant to directories */
};


static int out_fd = -1;
static int in_fd = -1;
static char default_rootdir[] = ".";
static char *rootdir = default_rootdir;
static int verbose = 0;
static int squash_uids = 0;
static int squash_perms = 0;
static int fake_times = 0;
int target_endian = __BYTE_ORDER;
static const char *const app_name = "mkfs.jffs2";
static const char *const memory_exhausted = "memory exhausted";

static void verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	fprintf(stderr, "%s: ", app_name);
	vfprintf(stderr, s, p);
}
static void error_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

static void error_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}

static void vperror_msg(const char *s, va_list p)
{
	int err = errno;

	if (s == 0)
		s = "";
	verror_msg(s, p);
	if (*s)
		s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

static void perror_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
}

static void perror_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
	exit(EXIT_FAILURE);
}

#ifndef DMALLOC
extern void *xmalloc(size_t size)
{
	void *ptr = malloc(size);

	if (ptr == NULL && size != 0)
		error_msg_and_die(memory_exhausted);
	return ptr;
}

extern void *xcalloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);

	if (ptr == NULL && nmemb != 0 && size != 0)
		error_msg_and_die(memory_exhausted);
	return ptr;
}

extern void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		error_msg_and_die(memory_exhausted);
	return ptr;
}

extern char *xstrdup(const char *s)
{
	char *t;

	if (s == NULL)
		return NULL;
	t = strdup(s);
	if (t == NULL)
		error_msg_and_die(memory_exhausted);
	return t;
}
#endif

extern char *xreadlink(const char *path)
{
	static const int GROWBY = 80; /* how large we will grow strings by */

	char *buf = NULL;
	int bufsize = 0, readsize = 0;

	do {
		buf = xrealloc(buf, bufsize += GROWBY);
		readsize = readlink(path, buf, bufsize); /* 1st try */
		if (readsize == -1) {
		    perror_msg("%s:%s", app_name, path);
		    return NULL;
		}
	}
	while (bufsize < readsize + 1);

	buf[readsize] = '\0';

	return buf;
}
static FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(path, mode)) == NULL)
		perror_msg_and_die("%s", path);
	return fp;
}

static struct filesystem_entry *find_filesystem_entry(
		struct filesystem_entry *dir, char *fullname, uint32_t type)
{
	struct filesystem_entry *e = dir;

	if (S_ISDIR(dir->sb.st_mode)) {
		e = dir->files;
	}
	while (e) {
		/* Only bother to do the expensive strcmp on matching file types */
		if (type == (e->sb.st_mode & S_IFMT)) {
			if (S_ISDIR(e->sb.st_mode)) {
				int len = strlen(e->fullname);

				/* Check if we are a parent of the correct path */
				if (strncmp(e->fullname, fullname, len) == 0) {
					/* Is this an _exact_ match? */
					if (strcmp(fullname, e->fullname) == 0) {
						return (e);
					}
					/* Looks like we found a parent of the correct path */
					if (fullname[len] == '/') {
						if (e->files) {
							return (find_filesystem_entry (e, fullname, type));
						} else {
							return NULL;
						}
					}
				}
			} else {
				if (strcmp(fullname, e->fullname) == 0) {
					return (e);
				}
			}
		}
		e = e->next;
	}
	return (NULL);
}

static struct filesystem_entry *add_host_filesystem_entry(
		char *name, char *path, unsigned long uid, unsigned long gid,
		unsigned long mode, dev_t rdev, struct filesystem_entry *parent)
{
	int status;
	char *tmp;
	struct stat sb;
	time_t timestamp = time(NULL);
	struct filesystem_entry *entry;

	memset(&sb, 0, sizeof(struct stat));
	status = lstat(path, &sb);

	if (status >= 0) {
		/* It is ok for some types of files to not exit on disk (such as
		 * device nodes), but if they _do_ exist the specified mode had
		 * better match the actual file or strange things will happen.... */
		if ((mode & S_IFMT) != (sb.st_mode & S_IFMT)) {
			error_msg_and_die ("%s: file type does not match specified type!", path);
		}
		timestamp = sb.st_mtime;
	} else {
		/* If this is a regular file, it _must_ exist on disk */
		if ((mode & S_IFMT) == S_IFREG) {
			error_msg_and_die("%s: does not exist!", path);
		}
	}

	/* Squash all permissions so files are owned by root, all
	 * timestamps are _right now_, and file permissions
	 * have group and other write removed */
	if (squash_uids) {
		uid = gid = 0;
	}
	if (squash_perms) {
		if (!S_ISLNK(mode)) {
			mode &= ~(S_IWGRP | S_IWOTH);
			mode &= ~(S_ISUID | S_ISGID);
		}
	}
	if (fake_times) {
		timestamp = 0;
	}

	entry = xcalloc(1, sizeof(struct filesystem_entry));

	entry->hostname = xstrdup(path);
	entry->fullname = xstrdup(name);
	tmp = xstrdup(name);
	entry->name = xstrdup(basename(tmp));
	free(tmp);
	tmp = xstrdup(name);
	entry->path = xstrdup(dirname(tmp));
	free(tmp);

	entry->sb.st_uid = uid;
	entry->sb.st_gid = gid;
	entry->sb.st_mode = mode;
	entry->sb.st_rdev = rdev;
	entry->sb.st_atime = entry->sb.st_ctime =
		entry->sb.st_mtime = timestamp;
	if (S_ISREG(mode)) {
		entry->sb.st_size = sb.st_size;
	}
	if (S_ISLNK(mode)) {
		entry->link = xreadlink(path);
		entry->sb.st_size = strlen(entry->link);
	}

	/* This happens only for root */
	if (!parent)
		return (entry);

	/* Hook the file into the parent directory */
	entry->parent = parent;
	if (!parent->files) {
		parent->files = entry;
	} else {
		struct filesystem_entry *prev;
		for (prev = parent->files; prev->next; prev = prev->next);
		prev->next = entry;
		entry->prev = prev;
	}

	return (entry);
}

static struct filesystem_entry *recursive_add_host_directory(
		struct filesystem_entry *parent, char *targetpath, char *hostpath)
{
	int i, n;
	struct stat sb;
	char *hpath, *tpath;
	struct dirent *dp, **namelist;
	struct filesystem_entry *entry;


	if (lstat(hostpath, &sb)) {
		perror_msg_and_die("%s", hostpath);
	}

	entry = add_host_filesystem_entry(targetpath, hostpath,
			sb.st_uid, sb.st_gid, sb.st_mode, 0, parent);

	n = scandir(hostpath, &namelist, 0, alphasort);
	if (n < 0) {
		perror_msg_and_die("opening directory %s", hostpath);
	}

	for (i=0; i<n; i++)
	{
		dp = namelist[i];
		if (dp->d_name[0] == '.' && (dp->d_name[1] == 0 ||
		   (dp->d_name[1] == '.' &&  dp->d_name[2] == 0)))
		{
			free(dp);
			continue;
		}

		asprintf(&hpath, "%s/%s", hostpath, dp->d_name);
		if (lstat(hpath, &sb)) {
			perror_msg_and_die("%s", hpath);
		}
		if (strcmp(targetpath, "/") == 0) {
			asprintf(&tpath, "%s%s", targetpath, dp->d_name);
		} else {
			asprintf(&tpath, "%s/%s", targetpath, dp->d_name);
		}

		switch (sb.st_mode & S_IFMT) {
			case S_IFDIR:
				recursive_add_host_directory(entry, tpath, hpath);
				break;

			case S_IFREG:
			case S_IFSOCK:
			case S_IFIFO:
			case S_IFLNK:
			case S_IFCHR:
			case S_IFBLK:
				add_host_filesystem_entry(tpath, hpath, sb.st_uid,
						sb.st_gid, sb.st_mode, sb.st_rdev, entry);
				break;

			default:
				error_msg("Unknown file type %o for %s", sb.st_mode, hpath);
				break;
		}
		free(dp);
		free(hpath);
		free(tpath);
	}
	free(namelist);
	return (entry);
}

/* the GNU C library has a wonderful scanf("%as", string) which will
 allocate the string with the right size, good to avoid buffer overruns.
 the following macros use it if available or use a hacky workaround...
 */

#ifdef __GNUC__
#define SCANF_PREFIX "a"
#define SCANF_STRING(s) (&s)
#define GETCWD_SIZE 0
#else
#define SCANF_PREFIX "511"
#define SCANF_STRING(s) (s = malloc(512))
#define GETCWD_SIZE -1
inline int snprintf(char *str, size_t n, const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsprintf(str, fmt, ap);
	va_end(ap);
	return ret;
}
#endif

/*  device table entries take the form of:
    <path>	<type> <mode>	<uid>	<gid>	<major>	<minor>	<start>	<inc>	<count>
    /dev/mem     c    640       0       0         1       1       0     0         -

    type can be one of:
	f	A regular file
	d	Directory
	c	Character special device file
	b	Block special device file
	p	Fifo (named pipe)

    I don't bother with symlinks (permissions are irrelevant), hard
    links (special cases of regular files), or sockets (why bother).

    Regular files must exist in the target root directory.  If a char,
    block, fifo, or directory does not exist, it will be created.
*/
static int interpret_table_entry(struct filesystem_entry *root, char *line)
{
	char *hostpath;
	char type, *name = NULL, *tmp, *dir;
	unsigned long mode = 0755, uid = 0, gid = 0, major = 0, minor = 0;
	unsigned long start = 0, increment = 1, count = 0;
	struct filesystem_entry *parent, *entry;

	if (sscanf (line, "%" SCANF_PREFIX "s %c %lo %lu %lu %lu %lu %lu %lu %lu",
		 SCANF_STRING(name), &type, &mode, &uid, &gid, &major, &minor,
		 &start, &increment, &count) < 0)
	{
		return 1;
	}

	if (!strcmp(name, "/")) {
		error_msg_and_die("Device table entries require absolute paths");
	}

	asprintf(&hostpath, "%s%s", rootdir, name);

	/* Check if this file already exists... */
	switch (type) {
		case 'd':
			mode |= S_IFDIR;
			break;
		case 'f':
			mode |= S_IFREG;
			break;
		case 'p':
			mode |= S_IFIFO;
			break;
		case 'c':
			mode |= S_IFCHR;
			break;
		case 'b':
			mode |= S_IFBLK;
			break;
		default:
			error_msg_and_die("Unsupported file type");
	}
	entry = find_filesystem_entry(root, name, mode);
	if (entry) {
		/* Ok, we just need to fixup the existing entry
		 * and we will be all done... */
		entry->sb.st_uid = uid;
		entry->sb.st_gid = gid;
		entry->sb.st_mode = mode;
		if (major && minor) {
			entry->sb.st_rdev = makedev(major, minor);
		}
	} else {
		/* If parent is NULL (happens with device table entries),
		 * try and find our parent now) */
		tmp = strdup(name);
		dir = dirname(tmp);
		parent = find_filesystem_entry(root, dir, S_IFDIR);
		free(tmp);
		if (parent == NULL) {
			error_msg ("skipping device_table entry '%s': no parent directory!", name);
			free(name);
			free(hostpath);
			return 1;
		}

		switch (type) {
			case 'd':
				add_host_filesystem_entry(name, hostpath, uid, gid, mode, 0, parent);
				break;
			case 'f':
				add_host_filesystem_entry(name, hostpath, uid, gid, mode, 0, parent);
				break;
			case 'p':
				add_host_filesystem_entry(name, hostpath, uid, gid, mode, 0, parent);
				break;
			case 'c':
			case 'b':
				if (count > 0) {
					dev_t rdev;
					unsigned long i;
					char *dname, *hpath;

					for (i = start; i < count; i++) {
						asprintf(&dname, "%s%lu", name, i);
						asprintf(&hpath, "%s/%s%lu", rootdir, name, i);
						rdev = makedev(major, minor + (i * increment - start));
						add_host_filesystem_entry(dname, hpath, uid, gid,
								mode, rdev, parent);
						free(dname);
						free(hpath);
					}
				} else {
					dev_t rdev = makedev(major, minor);
					add_host_filesystem_entry(name, hostpath, uid, gid,
							mode, rdev, parent);
				}
				break;
			default:
				error_msg_and_die("Unsupported file type");
		}
	}
	free(name);
	free(hostpath);
	return 0;
}

static int parse_device_table(struct filesystem_entry *root, FILE * file)
{
	char *line;
	int status = 0;
	size_t length = 0;

	/* Turn off squash, since we must ensure that values
	 * entered via the device table are not squashed */
	squash_uids = 0;
	squash_perms = 0;

	/* Looks ok so far.  The general plan now is to read in one
	 * line at a time, check for leading comment delimiters ('#'),
	 * then try and parse the line as a device table.  If we fail
	 * to parse things, try and help the poor fool to fix their
	 * device table with a useful error msg... */
	line = NULL;
	while (getline(&line, &length, file) != -1) {
		/* First trim off any whitespace */
		int len = strlen(line);

		/* trim trailing whitespace */
		while (len > 0 && isspace(line[len - 1]))
			line[--len] = '\0';
		/* trim leading whitespace */
		memmove(line, &line[strspn(line, " \n\r\t\v")], len);

		/* How long are we after trimming? */
		len = strlen(line);

		/* If this is NOT a comment line, try to interpret it */
		if (len && *line != '#') {
			if (interpret_table_entry(root, line))
				status = 1;
		}

		free(line);
		line = NULL;
	}
	fclose(file);

	return status;
}

static void cleanup(struct filesystem_entry *dir)
{
	struct filesystem_entry *e, *prev;

	e = dir->files;
	while (e) {
		if (e->name)
			free(e->name);
		if (e->path)
			free(e->path);
		if (e->fullname)
			free(e->fullname);
		e->next = NULL;
		e->name = NULL;
		e->path = NULL;
		e->fullname = NULL;
		e->prev = NULL;
		prev = e;
		if (S_ISDIR(e->sb.st_mode)) {
			cleanup(e);
		}
		e = e->next;
		free(prev);
	}
}

/* Here is where we do the actual creation of the file system */
#include "mtd/jffs2-user.h"

#define JFFS2_MAX_FILE_SIZE 0xFFFFFFFF
#ifndef JFFS2_MAX_SYMLINK_LEN
#define JFFS2_MAX_SYMLINK_LEN 254
#endif

static uint32_t ino = 0;
static uint8_t *file_buffer = NULL;		/* file buffer contains the actual erase block*/
static int out_ofs = 0;
static int erase_block_size = 65536;
static int pad_fs_size = 0;
static int add_cleanmarkers = 1;
static struct jffs2_unknown_node cleanmarker;
static int cleanmarker_size = sizeof(cleanmarker);
static unsigned char ffbuf[16] =
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff
};

/* We default to 4096, per x86.  When building a fs for
 * 64-bit arches and whatnot, use the --pagesize=SIZE option */
int page_size = 4096;

#include "compr.h"

static void full_write(int fd, const void *buf, int len)
{
	int ret;

	while (len > 0) {
		ret = write(fd, buf, len);

		if (ret < 0)
			perror_msg_and_die("write");

		if (ret == 0)
			perror_msg_and_die("write returned zero");

		len -= ret;
		buf += ret;
		out_ofs += ret;
	}
}

static void padblock(void)
{
	while (out_ofs % erase_block_size) {
		full_write(out_fd, ffbuf, min(sizeof(ffbuf),
					erase_block_size - (out_ofs % erase_block_size)));
	}
}

static void pad(int req)
{
	while (req) {
		if (req > sizeof(ffbuf)) {
			full_write(out_fd, ffbuf, sizeof(ffbuf));
			req -= sizeof(ffbuf);
		} else {
			full_write(out_fd, ffbuf, req);
			req = 0;
		}
	}
}

static inline void padword(void)
{
	if (out_ofs % 4) {
		full_write(out_fd, ffbuf, 4 - (out_ofs % 4));
	}
}

static inline void pad_block_if_less_than(int req)
{
	if (add_cleanmarkers) {
		if ((out_ofs % erase_block_size) == 0) {
			full_write(out_fd, &cleanmarker, sizeof(cleanmarker));
			pad(cleanmarker_size - sizeof(cleanmarker));
			padword();
		}
	}
	if ((out_ofs % erase_block_size) + req > erase_block_size) {
		padblock();
	}
	if (add_cleanmarkers) {
		if ((out_ofs % erase_block_size) == 0) {
			full_write(out_fd, &cleanmarker, sizeof(cleanmarker));
			pad(cleanmarker_size - sizeof(cleanmarker));
			padword();
		}
	}
}

static void write_dirent(struct filesystem_entry *e)
{
	char *name = e->name;
	struct jffs2_raw_dirent rd;
	struct stat *statbuf = &(e->sb);
	static uint32_t version = 0;

	memset(&rd, 0, sizeof(rd));

	rd.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	rd.nodetype = cpu_to_je16(JFFS2_NODETYPE_DIRENT);
	rd.totlen = cpu_to_je32(sizeof(rd) + strlen(name));
	rd.hdr_crc = cpu_to_je32(crc32(0, &rd,
				sizeof(struct jffs2_unknown_node) - 4));
	rd.pino = cpu_to_je32((e->parent) ? e->parent->sb.st_ino : 1);
	rd.version = cpu_to_je32(version++);
	rd.ino = cpu_to_je32(statbuf->st_ino);
	rd.mctime = cpu_to_je32(statbuf->st_mtime);
	rd.nsize = strlen(name);
	rd.type = IFTODT(statbuf->st_mode);
	//rd.unused[0] = 0;
	//rd.unused[1] = 0;
	rd.node_crc = cpu_to_je32(crc32(0, &rd, sizeof(rd) - 8));
	rd.name_crc = cpu_to_je32(crc32(0, name, strlen(name)));

	pad_block_if_less_than(sizeof(rd) + rd.nsize);
	full_write(out_fd, &rd, sizeof(rd));
	full_write(out_fd, name, rd.nsize);
	padword();
}

static void write_regular_file(struct filesystem_entry *e)
{
	int fd, len;
	uint32_t ver;
	unsigned int offset;
	unsigned char *buf, *cbuf, *wbuf;
	struct jffs2_raw_inode ri;
	struct stat *statbuf;


	statbuf = &(e->sb);
	if (statbuf->st_size >= JFFS2_MAX_FILE_SIZE) {
		error_msg("Skipping file \"%s\" too large.", e->path);
		return;
	}
	fd = open(e->hostname, O_RDONLY);
	if (fd == -1) {
		perror_msg_and_die("%s: open file", e->hostname);
	}

	statbuf->st_ino = ++ino;
	mkfs_debug_msg("writing file '%s'  ino=%lu  parent_ino=%lu",
			e->name, (unsigned long) statbuf->st_ino,
			(unsigned long) e->parent->sb.st_ino);
	write_dirent(e);

	buf = xmalloc(page_size);
	cbuf = NULL;

	ver = 0;
	offset = 0;

	memset(&ri, 0, sizeof(ri));
	ri.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);

	ri.ino = cpu_to_je32(statbuf->st_ino);
	ri.mode = cpu_to_jemode(statbuf->st_mode);
	ri.uid = cpu_to_je16(statbuf->st_uid);
	ri.gid = cpu_to_je16(statbuf->st_gid);
	ri.atime = cpu_to_je32(statbuf->st_atime);
	ri.ctime = cpu_to_je32(statbuf->st_ctime);
	ri.mtime = cpu_to_je32(statbuf->st_mtime);
	ri.isize = cpu_to_je32(statbuf->st_size);

	while ((len = read(fd, buf, page_size))) {
		unsigned char *tbuf = buf;

		if (len < 0) {
			perror_msg_and_die("read");
		}

		while (len) {
			uint32_t dsize, space;
                        uint16_t compression;

			pad_block_if_less_than(sizeof(ri) + JFFS2_MIN_DATA_LEN);

			dsize = len;
			space =
				erase_block_size - (out_ofs % erase_block_size) -
				sizeof(ri);
			if (space > dsize)
				space = dsize;

			compression = jffs2_compress(tbuf, &cbuf, &dsize, &space);
                        ri.compr = compression & 0xff;
                        ri.usercompr = (compression >> 8) & 0xff;
			if (ri.compr) {
				wbuf = cbuf;
			} else {
				wbuf = tbuf;
				dsize = space;
			}

			ri.totlen = cpu_to_je32(sizeof(ri) + space);
			ri.hdr_crc = cpu_to_je32(crc32(0,
						&ri, sizeof(struct jffs2_unknown_node) - 4));

			ri.version = cpu_to_je32(++ver);
			ri.offset = cpu_to_je32(offset);
			ri.csize = cpu_to_je32(space);
			ri.dsize = cpu_to_je32(dsize);
			ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri) - 8));
			ri.data_crc = cpu_to_je32(crc32(0, wbuf, space));

			full_write(out_fd, &ri, sizeof(ri));
			full_write(out_fd, wbuf, space);
			padword();

            if (tbuf!= cbuf) {
				free(cbuf);
				cbuf = NULL;
			}

			tbuf += dsize;
			len -= dsize;
			offset += dsize;

		}
	}
	if (!je32_to_cpu(ri.version)) {
		/* Was empty file */
		pad_block_if_less_than(sizeof(ri));

		ri.version = cpu_to_je32(++ver);
		ri.totlen = cpu_to_je32(sizeof(ri));
		ri.hdr_crc = cpu_to_je32(crc32(0,
					&ri, sizeof(struct jffs2_unknown_node) - 4));
		ri.csize = cpu_to_je32(0);
		ri.dsize = cpu_to_je32(0);
		ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri) - 8));

		full_write(out_fd, &ri, sizeof(ri));
		padword();
	}
	free(buf);
	close(fd);
}

static void write_symlink(struct filesystem_entry *e)
{
	int len;
	struct stat *statbuf;
	struct jffs2_raw_inode ri;

	statbuf = &(e->sb);
	statbuf->st_ino = ++ino;
	mkfs_debug_msg("writing symlink '%s'  ino=%lu  parent_ino=%lu",
			e->name, (unsigned long) statbuf->st_ino,
			(unsigned long) e->parent->sb.st_ino);
	write_dirent(e);

	len = strlen(e->link);
	if (len > JFFS2_MAX_SYMLINK_LEN) {
		error_msg("symlink too large. Truncated to %d chars.",
				JFFS2_MAX_SYMLINK_LEN);
		len = JFFS2_MAX_SYMLINK_LEN;
	}

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_je32(sizeof(ri) + len);
	ri.hdr_crc = cpu_to_je32(crc32(0,
				&ri, sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_je32(statbuf->st_ino);
	ri.mode = cpu_to_jemode(statbuf->st_mode);
	ri.uid = cpu_to_je16(statbuf->st_uid);
	ri.gid = cpu_to_je16(statbuf->st_gid);
	ri.atime = cpu_to_je32(statbuf->st_atime);
	ri.ctime = cpu_to_je32(statbuf->st_ctime);
	ri.mtime = cpu_to_je32(statbuf->st_mtime);
	ri.isize = cpu_to_je32(statbuf->st_size);
	ri.version = cpu_to_je32(1);
	ri.csize = cpu_to_je32(len);
	ri.dsize = cpu_to_je32(len);
	ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_je32(crc32(0, e->link, len));

	pad_block_if_less_than(sizeof(ri) + len);
	full_write(out_fd, &ri, sizeof(ri));
	full_write(out_fd, e->link, len);
	padword();
}

static void write_pipe(struct filesystem_entry *e)
{
	struct stat *statbuf;
	struct jffs2_raw_inode ri;

	statbuf = &(e->sb);
	statbuf->st_ino = ++ino;
	if (S_ISDIR(statbuf->st_mode)) {
		mkfs_debug_msg("writing dir '%s'  ino=%lu  parent_ino=%lu",
				e->name, (unsigned long) statbuf->st_ino,
				(unsigned long) (e->parent) ? e->parent->sb.  st_ino : 1);
	}
	write_dirent(e);

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_je32(sizeof(ri));
	ri.hdr_crc = cpu_to_je32(crc32(0,
				&ri, sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_je32(statbuf->st_ino);
	ri.mode = cpu_to_jemode(statbuf->st_mode);
	ri.uid = cpu_to_je16(statbuf->st_uid);
	ri.gid = cpu_to_je16(statbuf->st_gid);
	ri.atime = cpu_to_je32(statbuf->st_atime);
	ri.ctime = cpu_to_je32(statbuf->st_ctime);
	ri.mtime = cpu_to_je32(statbuf->st_mtime);
	ri.isize = cpu_to_je32(0);
	ri.version = cpu_to_je32(1);
	ri.csize = cpu_to_je32(0);
	ri.dsize = cpu_to_je32(0);
	ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_je32(0);

	pad_block_if_less_than(sizeof(ri));
	full_write(out_fd, &ri, sizeof(ri));
	padword();
}

static void write_special_file(struct filesystem_entry *e)
{
	jint16_t kdev;
	struct stat *statbuf;
	struct jffs2_raw_inode ri;

	statbuf = &(e->sb);
	statbuf->st_ino = ++ino;
	write_dirent(e);

	kdev = cpu_to_je16((major(statbuf->st_rdev) << 8) +
			minor(statbuf->st_rdev));

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_je32(sizeof(ri) + sizeof(kdev));
	ri.hdr_crc = cpu_to_je32(crc32(0,
				&ri, sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_je32(statbuf->st_ino);
	ri.mode = cpu_to_jemode(statbuf->st_mode);
	ri.uid = cpu_to_je16(statbuf->st_uid);
	ri.gid = cpu_to_je16(statbuf->st_gid);
	ri.atime = cpu_to_je32(statbuf->st_atime);
	ri.ctime = cpu_to_je32(statbuf->st_ctime);
	ri.mtime = cpu_to_je32(statbuf->st_mtime);
	ri.isize = cpu_to_je32(statbuf->st_size);
	ri.version = cpu_to_je32(1);
	ri.csize = cpu_to_je32(sizeof(kdev));
	ri.dsize = cpu_to_je32(sizeof(kdev));
	ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_je32(crc32(0, &kdev, sizeof(kdev)));

	pad_block_if_less_than(sizeof(ri) + sizeof(kdev));
	full_write(out_fd, &ri, sizeof(ri));
	full_write(out_fd, &kdev, sizeof(kdev));
	padword();
}

static void recursive_populate_directory(struct filesystem_entry *dir)
{
	struct filesystem_entry *e;

	if (verbose) {
		printf("%s\n", dir->fullname);
	}
	e = dir->files;
	while (e) {

		switch (e->sb.st_mode & S_IFMT) {
			case S_IFDIR:
				if (verbose) {
					printf("\td %04o %9lu %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, e->sb.st_size,
							(int) (e->sb.st_uid), (int) (e->sb.st_gid),
							e->name);
				}
				write_pipe(e);
				break;
			case S_IFSOCK:
				if (verbose) {
					printf("\ts %04o %9lu %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, e->sb.st_size,
							(int) e->sb.st_uid, (int) e->sb.st_gid, e->name);
				}
				write_pipe(e);
				break;
			case S_IFIFO:
				if (verbose) {
					printf("\tp %04o %9lu %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, e->sb.st_size,
							(int) e->sb.st_uid, (int) e->sb.st_gid, e->name);
				}
				write_pipe(e);
				break;
			case S_IFCHR:
				if (verbose) {
					printf("\tc %04o %4d,%4d %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, major(e->sb.st_rdev),
							minor(e->sb.st_rdev), (int) e->sb.st_uid,
							(int) e->sb.st_gid, e->name);
				}
				write_special_file(e);
				break;
			case S_IFBLK:
				if (verbose) {
					printf("\tb %04o %4d,%4d %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, major(e->sb.st_rdev),
							minor(e->sb.st_rdev), (int) e->sb.st_uid,
							(int) e->sb.st_gid, e->name);
				}
				write_special_file(e);
				break;
			case S_IFLNK:
				if (verbose) {
					printf("\tl %04o %9lu %5d:%-3d %s -> %s\n",
							e->sb.st_mode & ~S_IFMT, e->sb.st_size,
							(int) e->sb.st_uid, (int) e->sb.st_gid, e->name,
							e->link);
				}
				write_symlink(e);
				break;
			case S_IFREG:
				if (verbose) {
					printf("\tf %04o %9lu %5d:%-3d %s\n",
							e->sb.st_mode & ~S_IFMT, e->sb.st_size,
							(int) e->sb.st_uid, (int) e->sb.st_gid, e->name);
				}
				write_regular_file(e);
				break;
			default:
				error_msg("Unknown mode %o for %s", e->sb.st_mode,
						e->fullname);
				break;
		}
		e = e->next;
	}

	e = dir->files;
	while (e) {
		if (S_ISDIR(e->sb.st_mode)) {
			if (e->files) {
				recursive_populate_directory(e);
			} else if (verbose) {
				printf("%s\n", e->fullname);
			}
		}
		e = e->next;
	}
}

static void create_target_filesystem(struct filesystem_entry *root)
{
	cleanmarker.magic    = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16(JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen   = cpu_to_je32(cleanmarker_size);
	cleanmarker.hdr_crc  = cpu_to_je32(crc32(0, &cleanmarker, sizeof(struct jffs2_unknown_node)-4));

	if (ino == 0)
		ino = 1;

	root->sb.st_ino = 1;
	recursive_populate_directory(root);

	if (pad_fs_size == -1) {
		padblock();
	} else {
		if (pad_fs_size && add_cleanmarkers){
			padblock();
			while (out_ofs < pad_fs_size) {
				full_write(out_fd, &cleanmarker, sizeof(cleanmarker));
				pad(cleanmarker_size - sizeof(cleanmarker));
				padblock();
			}
		} else {
			while (out_ofs < pad_fs_size) {
				full_write(out_fd, ffbuf, min(sizeof(ffbuf), pad_fs_size - out_ofs));
			}

		}
	}
}

static struct option long_options[] = {
	{"pad", 2, NULL, 'p'},
	{"root", 1, NULL, 'r'},
	{"pagesize", 1, NULL, 's'},
	{"eraseblock", 1, NULL, 'e'},
	{"output", 1, NULL, 'o'},
	{"help", 0, NULL, 'h'},
	{"verbose", 0, NULL, 'v'},
	{"version", 0, NULL, 'V'},
	{"big-endian", 0, NULL, 'b'},
	{"little-endian", 0, NULL, 'l'},
	{"no-cleanmarkers", 0, NULL, 'n'},
	{"cleanmarker", 1, NULL, 'c'},
	{"squash", 0, NULL, 'q'},
	{"squash-uids", 0, NULL, 'U'},
	{"squash-perms", 0, NULL, 'P'},
	{"faketime", 0, NULL, 'f'},
	{"devtable", 1, NULL, 'D'},
	{"compression-mode", 1, NULL, 'm'},
	{"disable-compressor", 1, NULL, 'x'},
	{"test-compression", 0, NULL, 't'},
	{"compressor-priority", 1, NULL, 'y'},
	{"incremental", 1, NULL, 'i'},
	{NULL, 0, NULL, 0}
};

static char *helptext =
	"Usage: mkfs.jffs2 [OPTIONS]\n"
	"Make a JFFS2 file system image from an existing directory tree\n\n"
	"Options:\n"
	"  -p, --pad[=SIZE]        Pad output to SIZE bytes with 0xFF. If SIZE is\n"
	"                          not specified, the output is padded to the end of\n"
	"                          the final erase block\n"
	"  -r, -d, --root=DIR      Build file system from directory DIR (default: cwd)\n"
	"  -s, --pagesize=SIZE     Use page size (max data node size) SIZE (default: 4KiB)\n"
	"  -e, --eraseblock=SIZE   Use erase block size SIZE (default: 64KiB)\n"
	"  -c, --cleanmarker=SIZE  Size of cleanmarker (default 12)\n"
	"  -m, --compr-mode=MODE   Select compression mode (default: priortiry)\n"
        "  -x, --disable-compressor=COMPRESSOR_NAME\n"
        "                          Disable a compressor\n"
        "  -X, --enable-compressor=COMPRESSOR_NAME\n"
        "                          Enable a compressor\n"
        "  -y, --compressor-priority=PRIORITY:COMPRESSOR_NAME\n"
        "                          Set the priority of a compressor\n"
        "  -L, --list-compressors  Show the list of the avaiable compressors\n"
        "  -t, --test-compression  Call decompress and compare with the original (for test)\n"
	"  -n, --no-cleanmarkers   Don't add a cleanmarker to every eraseblock\n"
	"  -o, --output=FILE       Output to FILE (default: stdout)\n"
	"  -l, --little-endian     Create a little-endian filesystem\n"
	"  -b, --big-endian        Create a big-endian filesystem\n"
	"  -D, --devtable=FILE     Use the named FILE as a device table file\n"
	"  -f, --faketime          Change all file times to '0' for regression testing\n"
	"  -q, --squash            Squash permissions and owners making all files be owned by root\n"
	"  -U, --squash-uids       Squash owners making all files be owned by root\n"
	"  -P, --squash-perms      Squash permissions on all files\n"
	"  -h, --help              Display this help text\n"
	"  -v, --verbose           Verbose operation\n"
	"  -V, --version           Display version information\n"
	"  -i, --incremental=FILE  Parse FILE and generate appendage output for it\n\n";

static char *revtext = "$Revision: 1.1 $";

int load_next_block() {

	int ret;
	ret = read(in_fd, file_buffer, erase_block_size);

	if(verbose)
		printf("Load next block : %d bytes read\n",ret);

	return ret;
}

void process_buffer(int inp_size) {
	uint8_t		*p = file_buffer;
	union jffs2_node_union 	*node;
	uint16_t	type;
	int		bitchbitmask = 0;
	int		obsolete;

	char	name[256];

	while ( p < (file_buffer + inp_size)) {

		node = (union jffs2_node_union *) p;

		/* Skip empty space */
		if (je16_to_cpu (node->u.magic) == 0xFFFF && je16_to_cpu (node->u.nodetype) == 0xFFFF) {
			p += 4;
			continue;
		}

		if (je16_to_cpu (node->u.magic) != JFFS2_MAGIC_BITMASK)	{
			if (!bitchbitmask++)
    			    printf ("Wrong bitmask  at  0x%08x, 0x%04x\n", p - file_buffer, je16_to_cpu (node->u.magic));
			p += 4;
			continue;
		}

		bitchbitmask = 0;

		type = je16_to_cpu(node->u.nodetype);
		if ((type & JFFS2_NODE_ACCURATE) != JFFS2_NODE_ACCURATE) {
			obsolete = 1;
			type |= JFFS2_NODE_ACCURATE;
		} else
			obsolete = 0;

		node->u.nodetype = cpu_to_je16(type);

		switch(je16_to_cpu(node->u.nodetype)) {

			case JFFS2_NODETYPE_INODE:
				if(verbose)
					printf ("%8s Inode      node at 0x%08x, totlen 0x%08x, #ino  %5d, version %5d, isize %8d, csize %8d, dsize %8d, offset %8d\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->i.totlen), je32_to_cpu (node->i.ino),
						je32_to_cpu ( node->i.version), je32_to_cpu (node->i.isize),
						je32_to_cpu (node->i.csize), je32_to_cpu (node->i.dsize), je32_to_cpu (node->i.offset));

				if ( je32_to_cpu (node->i.ino) > ino )
					ino = je32_to_cpu (node->i.ino);

				p += PAD(je32_to_cpu (node->i.totlen));
				break;

			case JFFS2_NODETYPE_DIRENT:
				memcpy (name, node->d.name, node->d.nsize);
				name [node->d.nsize] = 0x0;

				if(verbose)
					printf ("%8s Dirent     node at 0x%08x, totlen 0x%08x, #pino %5d, version %5d, #ino  %8d, nsize %8d, name %s\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->d.totlen), je32_to_cpu (node->d.pino),
						je32_to_cpu ( node->d.version), je32_to_cpu (node->d.ino),
						node->d.nsize, name);

				p += PAD(je32_to_cpu (node->d.totlen));
				break;

			case JFFS2_NODETYPE_CLEANMARKER:
				if (verbose) {
					printf ("%8s Cleanmarker     at 0x%08x, totlen 0x%08x\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}

				p += PAD(je32_to_cpu (node->u.totlen));
				break;

			case JFFS2_NODETYPE_PADDING:
				if (verbose) {
					printf ("%8s Padding    node at 0x%08x, totlen 0x%08x\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}

				p += PAD(je32_to_cpu (node->u.totlen));
				break;

			case 0xffff:
				p += 4;
				break;

			default:
				if (verbose) {
					printf ("%8s Unknown    node at 0x%08x, totlen 0x%08x\n",
						obsolete ? "Obsolete" : "",
						p - file_buffer, je32_to_cpu (node->u.totlen));
				}

				p += PAD(je32_to_cpu (node->u.totlen));
		}
	}
}

void parse_image(){
	int ret;

	file_buffer = malloc(erase_block_size);

	if (!file_buffer) {
		perror("out of memory");
		close (in_fd);
		close (out_fd);
		exit(1);
	}

	while ((ret = load_next_block())) {
		process_buffer(ret);
	}

	if (file_buffer)
		free(file_buffer);

	close(in_fd);
}

int main(int argc, char **argv)
{
	int c, opt;
	char *cwd;
	struct stat sb;
	FILE *devtable = NULL;
	struct filesystem_entry *root;
        char *compr_name = NULL;
        int compr_prior  = -1;

        jffs2_compressors_init();

	while ((opt = getopt_long(argc, argv,
					"D:d:r:s:o:qUPfh?vVe:lbp::nc:m:x:X:Lty:i:", long_options, &c)) >= 0)
	{
		switch (opt) {
			case 'D':
				devtable = xfopen(optarg, "r");
				if (fstat(fileno(devtable), &sb) < 0)
					perror_msg_and_die(optarg);
				if (sb.st_size < 10)
					error_msg_and_die("%s: not a proper device table file", optarg);
				break;

			case 'r':
			case 'd':	/* for compatibility with mkfs.jffs, genext2fs, etc... */
				if (rootdir != default_rootdir) {
					error_msg_and_die("root directory specified more than once");
				}
				rootdir = xstrdup(optarg);
				break;

			case 's':
				page_size = strtol(optarg, NULL, 0);
				break;

			case 'o':
				if (out_fd != -1) {
					error_msg_and_die("output filename specified more than once");
				}
				out_fd = open(optarg, O_CREAT | O_TRUNC | O_RDWR, 0644);
				if (out_fd == -1) {
					perror_msg_and_die("open output file");
				}
				break;

			case 'q':
				squash_uids = 1;
				squash_perms = 1;
				break;

			case 'U':
				squash_uids = 1;
				break;

			case 'P':
				squash_perms = 1;
				break;

			case 'f':
				fake_times = 1;
				break;

			case 'h':
			case '?':
				error_msg_and_die(helptext);

			case 'v':
				verbose = 1;
				break;

			case 'V':
				error_msg_and_die("revision %.*s\n",
						(int) strlen(revtext) - 13, revtext + 11);

			case 'e': {
				char *next;
				unsigned units = 0;
				erase_block_size = strtol(optarg, &next, 0);
				if (!erase_block_size)
					error_msg_and_die("Unrecognisable erase size\n");

				if (*next) {
					if (!strcmp(next, "KiB")) {
						units = 1024;
					} else if (!strcmp(next, "MiB")) {
						units = 1024 * 1024;
					} else {
						error_msg_and_die("Unknown units in erasesize\n");
					}
				} else {
					if (erase_block_size < 0x1000)
						units = 1024;
					else
						units = 1;
				}
				erase_block_size *= units;

				/* If it's less than 8KiB, they're not allowed */
				if (erase_block_size < 0x2000) {
					fprintf(stderr, "Erase size 0x%x too small. Increasing to 8KiB minimum\n",
						erase_block_size);
					erase_block_size = 0x2000;
				}
				break;
			}

			case 'l':
				target_endian = __LITTLE_ENDIAN;
				break;

			case 'b':
				target_endian = __BIG_ENDIAN;
				break;

			case 'p':
				if (optarg)
					pad_fs_size = strtol(optarg, NULL, 0);
				else
					pad_fs_size = -1;
				break;
			case 'n':
				add_cleanmarkers = 0;
				break;
			case 'c':
				cleanmarker_size = strtol(optarg, NULL, 0);
				if (cleanmarker_size < sizeof(cleanmarker)) {
					error_msg_and_die("cleanmarker size must be >= 12");
				}
				if (cleanmarker_size >= erase_block_size) {
					error_msg_and_die("cleanmarker size must be < eraseblock size");
				}
				break;
                        case 'm':
                                if (jffs2_set_compression_mode_name(optarg)) {
					error_msg_and_die("Unknown compression mode %s", optarg);
				}
                                break;
                        case 'x':
                                if (jffs2_disable_compressor_name(optarg)) {
                                        error_msg_and_die("Unknown compressor name %s",optarg);
                                }
                                break;
                        case 'X':
                                if (jffs2_enable_compressor_name(optarg)) {
                                        error_msg_and_die("Unknown compressor name %s",optarg);
                                }
                                break;
                        case 'L':
                                error_msg_and_die("\n%s",jffs2_list_compressors());
                                break;
                        case 't':
                                jffs2_compression_check_set(1);
                                break;
                        case 'y':
                                compr_name = malloc(strlen(optarg));
                                sscanf(optarg,"%d:%s",&compr_prior,compr_name);
                                if ((compr_prior>=0)&&(compr_name)) {
                                        if (jffs2_set_compressor_priority(compr_name, compr_prior))
	                                        exit(EXIT_FAILURE);
                                }
                                else {
                                        error_msg_and_die("Cannot parse %s",optarg);
                                }
                                free(compr_name);
                                break;
			case 'i':
				if (in_fd != -1) {
					error_msg_and_die("(incremental) filename specified more than once");
				}
				in_fd = open(optarg, O_RDONLY);
				if (in_fd == -1) {
					perror_msg_and_die("cannot open (incremental) file");
				}
				break;
		}
	}
	if (out_fd == -1) {
		if (isatty(1)) {
			error_msg_and_die(helptext);
		}
		out_fd = 1;
	}
	if (lstat(rootdir, &sb)) {
		perror_msg_and_die("%s", rootdir);
	}
	if (chdir(rootdir))
		perror_msg_and_die("%s", rootdir);

	if (!(cwd = getcwd(0, GETCWD_SIZE)))
		perror_msg_and_die("getcwd failed");

	if(in_fd != -1)
		parse_image();

	root = recursive_add_host_directory(NULL, "/", cwd);

	if (devtable)
		parse_device_table(root, devtable);

	create_target_filesystem(root);

	cleanup(root);

	if (rootdir != default_rootdir)
		free(rootdir);

	close(out_fd);

        if (verbose) {
                char *s = jffs2_stats();
                fprintf(stderr,"\n\n%s",s);
                free(s);
        }
        if ((verbose)||(jffs2_compression_check_get()&&(jffs2_compression_check_errorcnt_get()))) {
                fprintf(stderr,"Compression errors: %d\n",jffs2_compression_check_errorcnt_get());
        }

        jffs2_compressors_exit();

	return 0;
}
