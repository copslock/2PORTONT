/* vi: set sw=4 ts=4: */
/* printf - format and print data

   Copyright 1999 Dave Cinege
   Portions copyright (C) 1990-1996 Free Software Foundation, Inc.

   Licensed under GPL v2 or later, see file LICENSE in this tarball for details.
*/

/* Usage: printf format [argument...]

   A front end to the printf function that lets it be used from the shell.

   Backslash escapes:

   \" = double quote
   \\ = backslash
   \a = alert (bell)
   \b = backspace
   \c = produce no further output
   \f = form feed
   \n = new line
   \r = carriage return
   \t = horizontal tab
   \v = vertical tab
   \0ooo = octal number (ooo is 0 to 3 digits)
   \xhhh = hexadecimal number (hhh is 1 to 3 digits)

   Additional directive:

   %b = print an argument string, interpreting backslash escapes

   The 'format' argument is re-used as many times as necessary
   to convert all of the given arguments.

   David MacKenzie <djm@gnu.ai.mit.edu>
*/

//   19990508 Busy Boxed! Dave Cinege

#include "libbb.h"

/* A note on bad input: neither bash 3.2 nor coreutils 6.10 stop on it.
 * They report it:
 *  bash: printf: XXX: invalid number
 *  printf: XXX: expected a numeric value
 *  bash: printf: 123XXX: invalid number
 *  printf: 123XXX: value not completely converted
 * but then they use 0 (or partially converted numeric prefix) as a value
 * and continue. They exit with 1 in this case.
 * Both accept insane field width/precision (e.g. %9999999999.9999999999d).
 * Both print error message and assume 0 if %*.*f width/precision is "bad"
 *  (but negative numbers are not "bad").
 * Both accept negative numbers for %u specifier.
 *
 * We try to be compatible. We are not compatible here:
 * - we do not accept -NUM for %u
 * - exit code is 0 even if "invalid number" was seen (FIXME)
 * See "if (errno)" checks in the code below.
 */

typedef void FAST_FUNC (*converter)(const char *arg, void *result);

static int multiconvert(const char *arg, void *result, converter convert)
{
	if (*arg == '"' || *arg == '\'') {
		arg = utoa((unsigned char)arg[1]);
	}
	errno = 0;
	convert(arg, result);
	if (errno) {
		bb_error_msg("%s: invalid number", arg);
		return 1;
	}
	return 0;
}

static void FAST_FUNC conv_strtoul(const char *arg, void *result)
{
	*(unsigned long*)result = bb_strtoul(arg, NULL, 0);

	// For negative value
	if (errno) {
		*(unsigned long*)result = bb_strtoll(arg, NULL, 0);
	}
}
static void FAST_FUNC conv_strtol(const char *arg, void *result)
{
	*(long*)result = bb_strtol(arg, NULL, 0);
}
static void FAST_FUNC conv_strtod(const char *arg, void *result)
{
	char *end;
	/* Well, this one allows leading whitespace... so what? */
	/* What I like much less is that "-" accepted too! :( */
	*(double*)result = strtod(arg, &end);
	if (end[0]) {
		errno = ERANGE;
		*(double*)result = 0;
	}
}

/* Callers should check errno to detect errors */
static unsigned long my_xstrtoul(const char *arg)
{
	unsigned long result;
	if (multiconvert(arg, &result, conv_strtoul))
		result = 0;
	return result;
}
static long my_xstrtol(const char *arg)
{
	long result;
	if (multiconvert(arg, &result, conv_strtol))
		result = 0;
	return result;
}
static double my_xstrtod(const char *arg)
{
	double result;
	multiconvert(arg, &result, conv_strtod);
	return result;
}

static void print_esc_string(char *str)
{
	while (*str) {
		if (*str == '\\') {
			str++;
			bb_putchar(bb_process_escape_sequence((const char **)&str));
		} else {
			bb_putchar(*str);
			str++;
		}
	}
}

static void print_direc(char *format, unsigned fmt_length,
		int field_width, int precision,
		const char *argument)
{
	long lv;
	double dv;
	char saved;
	char *have_prec, *have_width;

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	if (have_width - 1 == have_prec)
		have_width = NULL;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	switch (format[fmt_length - 1]) {
	case 'c':
		printf(format, *argument);
		break;
	case 'd':
	case 'i':
		lv = my_xstrtol(argument);
 print_long:
		/* if (errno) return; - see comment at the top */
		if (!have_width) {
			if (!have_prec)
				printf(format, lv);
			else
				printf(format, precision, lv);
		} else {
			if (!have_prec)
				printf(format, field_width, lv);
			else
				printf(format, field_width, precision, lv);
		}
		break;
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		lv = my_xstrtoul(argument);
		/* cheat: unsigned long and long have same width, so... */
		goto print_long;
	case 's':
		/* Are char* and long the same? (true for most arches) */
		if (sizeof(argument) == sizeof(lv)) {
			lv = (long)(ptrdiff_t)argument;
			goto print_long;
		} else { /* Hope compiler will optimize it out */
			if (!have_width) {
				if (!have_prec)
					printf(format, argument);
				else
					printf(format, precision, argument);
			} else {
				if (!have_prec)
					printf(format, field_width, argument);
				else
					printf(format, field_width, precision, argument);
			}
			break;
		}
	case 'f':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
		dv = my_xstrtod(argument);
		/* if (errno) return; */
		if (!have_width) {
			if (!have_prec)
				printf(format, dv);
			else
				printf(format, precision, dv);
		} else {
			if (!have_prec)
				printf(format, field_width, dv);
			else
				printf(format, field_width, precision, dv);
		}
		break;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
static int get_width_prec(const char *str)
{
	int v = bb_strtoi(str, NULL, 10);
	if (errno) {
		bb_error_msg("%s: invalid number", str);
		v = 0;
	}
	return v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
   Return advanced ARGV.  */
static char **print_formatted(char *f, char **argv)
{
	char *direc_start;      /* Start of % directive.  */
	unsigned direc_length;  /* Length of % directive.  */
	int field_width;        /* Arg to first '*' */
	int precision;          /* Arg to second '*' */
	char **saved_argv = argv;

	for (; *f; ++f) {
		switch (*f) {
		case '%':
			direc_start = f++;
			direc_length = 1;
			field_width = precision = 0;
			if (*f == '%') {
				bb_putchar('%');
				break;
			}
			if (*f == 'b') {
				if (*argv) {
					print_esc_string(*argv);
					++argv;
				}
				break;
			}
			if (strchr("-+ #", *f)) {
				++f;
				++direc_length;
			}
			if (*f == '*') {
				++f;
				++direc_length;
				if (*argv)
					field_width = get_width_prec(*argv++);
			} else {
				while (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			if (*f == '.') {
				++f;
				++direc_length;
				if (*f == '*') {
					++f;
					++direc_length;
					if (*argv)
						precision = get_width_prec(*argv++);
				} else {
					while (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}
			/* Remove size modifiers - "%Ld" would try to printf
			 * long long, we pass long, and it spews garbage */
			if ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z') {
				overlapping_strcpy(f, f + 1);
			}
//FIXME: actually, the same happens with bare "%d":
//it printfs an int, but we pass long!
//What saves us is that on most arches stack slot
//is pointer-sized -> long-sized -> ints are promoted to longs
// for variadic functions -> printf("%d", int_v) is in reality
// indistinqushable from printf("%d", long_v) ->
// since printf("%d", int_v) works, printf("%d", long_v) has to work.
//But "clean" solution would be to add "l" to d,i,o,x,X.
//Probably makes sense to go all the way to "ll" then.
//Coreutils support long long-sized arguments.

			/* needed - try "printf %" without it */
			if (!strchr("diouxXfeEgGcs", *f)) {
				bb_error_msg("%s: invalid format", direc_start);
				/* causes main() to exit with error */
				return saved_argv - 1;
			}
			++direc_length;
			if (*argv) {
				print_direc(direc_start, direc_length, field_width,
							precision, *argv);
				++argv;
			} else {
				print_direc(direc_start, direc_length, field_width,
							precision, "");
			}
			/* if (errno) return saved_argv - 1; */
			break;
		case '\\':
			if (*++f == 'c') {
				return saved_argv; /* causes main() to exit */
			}
			bb_putchar(bb_process_escape_sequence((const char **)&f));
			f--;
			break;
		default:
			bb_putchar(*f);
		}
	}

	return argv;
}

int printf_main(int argc UNUSED_PARAM, char **argv)
{
	char *format;
	char **argv2;

	/* We must check that stdout is not closed.
	 * The reason for this is highly non-obvious.
	 * printf_main is used from shell.
	 * Shell must correctly handle 'printf "%s" foo'
	 * if stdout is closed. With stdio, output gets shoveled into
	 * stdout buffer, and even fflush cannot clear it out. It seems that
	 * even if libc receives EBADF on write attempts, it feels determined
	 * to output data no matter what. So it will try later,
	 * and possibly will clobber future output. Not good. */
// TODO: check fcntl() & O_ACCMODE == O_WRONLY or O_RDWR?
	if (fcntl(1, F_GETFL) == -1)
		return 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	/* bash builtin errors out on "printf '-%s-\n' foo",
	 * coreutils-6.9 works. Both work with "printf -- '-%s-\n' foo".
	 * We will mimic coreutils. */
	if (argv[1] && argv[1][0] == '-' && argv[1][1] == '-' && !argv[1][2])
		argv++;
	if (!argv[1]) {
		if (ENABLE_ASH_BUILTIN_PRINTF
		 && applet_name[0] != 'p'
		) {
			bb_error_msg("usage: printf FORMAT [ARGUMENT...]");
			return 2; /* bash compat */
		}
		bb_show_usage();
	}

	format = argv[1];
	argv2 = argv + 2;

	do {
		argv = argv2;
		argv2 = print_formatted(format, argv);
	} while (argv2 > argv && *argv2);

	/* coreutils compat (bash doesn't do this):
	if (*argv)
		fprintf(stderr, "excess args ignored");
	*/

	return (argv2 < argv); /* if true, print_formatted errored out */
}
