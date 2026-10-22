/*
 * Gophernicus
 *
 * Copyright (c) 2009-2018 Kim Holviala <kimholviala@fastmail.com>
 * Copyright (c) 2019 Gophernicus Developers <gophernicus@gophernicus.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "gophernicus.h"
#include <ctype.h>


/*
 * Repeat a character num times and zero-terminate
 */
void strrepeat(char *dest, char c, size_t num)
{
	memset(dest, c, num);
	dest[num] = '\0';
}


/*
 * Replace characters in-place
 */
void strreplace(char *str, char from, char to)
{
	while (*str) {
		if (*str == from) *str = to;
		str++;
	}
}


/*
 * Cut string to width, return resulting width (UTF-8 aware)
 */
size_t strcut(char *str, size_t width)
{
	char *cursor = str;
	size_t count = 0;

	while (*cursor && count < width) {
		unsigned char c = (unsigned char)*cursor;
		size_t bytes = 1;
		size_t i;

		if ((c & 0xf8) == 0xf0) bytes = 4;
		else if ((c & 0xf0) == 0xe0) bytes = 3;
		else if ((c & 0xe0) == 0xc0) bytes = 2;

		/* An incomplete sequence must not advance beyond the NUL byte. */
		for (i = 1; i < bytes; i++) {
			unsigned char next = (unsigned char)cursor[i];
			if (next == '\0' || (next & 0xc0) != 0x80) {
				bytes = 1;
				break;
			}
		}

		cursor += bytes;
		count++;
	}

	if (count == width) *cursor = '\0';
	return count;
}


/*
 * Match key and return value (key: value)
 */
char *strkey(char *header, char *key)
{
	char *c;
	size_t len;

	if ((len = strlen(key)) == 0) return NULL;

	if (strncasecmp(header, key, len) == MATCH) {
		c = header + len;
		do { c++; } while (*c == ' ' || *c == '\t');

		if (*c != ':') return NULL;

		do { c++; } while (*c == ' ' || *c == '\t');
		return c;
	}

	return NULL;
}


/*
 * Return last character of a string
 */
char strlast(char *str)
{
	int len;

	if ((len = (int)strlen(str) - 1) >= 0) return str[len];
	else return 0;
}


/*
 * Remove CRLF from a string
 */
void chomp(char *str)
{
	char *c;

	if ((c = strrchr(str, '\n'))) *c = '\0';
	if ((c = strrchr(str, '\r'))) *c = '\0';
}


/*
 * Return charset name
 */
char *strcharset(int charset)
{
	if (charset == AUTO) return "auto";
	if (charset == US_ASCII) return "US-ASCII";
	if (charset == ISO_8859_1) return "ISO-8859-1";
	if (charset == UTF_8) return "UTF-8";

	return "(unknown)";
}


/*
 * Convert a string between UTF-8, ISO-8859-1 and US-ASCII
 */
void strniconv(int charset, char *out, char *in, size_t outsize)
{
	char ascii[] = ASCII;
	unsigned long c;
	size_t len;
	int i;

	/* Loop through the input string */
	len = strlen(in);
	while (--outsize && len > 0) {

		/* Get one input char */
		c = (unsigned char) *in++;
		len--;

		/* 7-bit chars are the same in all three charsets */
		if (c < 0x80) {
			*out++ = (unsigned char) c;
			continue;
		}

		/* Assume ISO-8859-1 which requires 0 extra bytes */
		i = 0;

		/* UTF-8? (We'll actually check the next char here, not current) */
		if ((*in & 0xc0) == 0x80) {

			/* Four-byte UTF-8? */
			if ((c & 0xf8) == 0xf0 && len >= 3) { c &= 0x07; i = 3; }

			/* Three-byte UTF-8? */
			else if ((c & 0xf0) == 0xe0 && len >= 2) { c &= 0x0f; i = 2; }

			/* Two-byte UTF-8? */
			else if ((c & 0xe0) == 0xc0 && len >= 1) { c &= 0x1f; i = 1; }

			/* Parse rest of the UTF-8 bytes */
			while (i--) {
				c <<= 6;
				c |= *in++ & 0x3f;
				len--;
			}
		}

		/*
		 * At this point we've got one 32bit UTF character in c and
		 * we're ready to convert it to the specified output charset
		 */

		/* Handle UTF-8 */
		if (charset == UTF_8) {
			i = 0;

			/* Two-byte encoding? */
			if (c < 0x800 && outsize > 2) { *out++ = (c >> 6) | 0xc0; i = 1; }

			/* Three-byte encoding? */
			else if (c < 0x10000 && outsize > 3) { *out++ = (c >> 12) | 0xe0; i = 2; }

			/* Four-byte encoding? */
			else if (c < 0x110000 && outsize > 4) { *out++ = (c >> 18) | 0xf0; i = 3; }

			/* Encode rest of the UTF-8 bytes */
			while (i--) {
				*out++ = ((c >> (i * 6)) & 0x3f) | 0x80;
				outsize--;
			}
			continue;
		}

		/* Handle ISO-8859-1 */
		if (charset == ISO_8859_1) {

			if (c >= 0xa0 && c <= 0xff)
				*out++ = (unsigned char) c;
			else
				*out++ = UNKNOWN;
			continue;
		}

		/* Handle all other charsets as 7-bit US-ASCII */
		if (c >= 0x80 && c <= 0xff)
			*out++ = ascii[c - 0x80];
		else
			*out++ = UNKNOWN;
	}

	/* Zero-terminate output */
	*out = '\0';
}


/*
 * Encode string with #OCT encoding
 */
void strnencode(char *out, const char *in, size_t outsize)
{
	unsigned char c;

	if (outsize == 0) return;

	/* Account for all four bytes of each #OCT escape, not just the input. */
	while (*in && outsize > 1) {
		c = (unsigned char)*in++;
		if (c < '+' || c > '~') {
			if (outsize < 5) break;
			snprintf(out, outsize, "#%.3o", c);
			out += 4;
			outsize -= 4;
		} else {
			*out++ = (char)c;
			outsize--;
		}
	}

	*out = '\0';
}


/*
 * Decode both %HEX and #OCT encodings
 */
void strndecode(char *out, char *in, size_t outsize)
{
	unsigned char c;
	unsigned int i;

	if (outsize == 0) return;

	/* A malformed or NUL escape remains literal. */
	while (--outsize) {
		if (!(c = (unsigned char)*in++)) break;

		if (c == '%' && strlen(in) >= 2 &&
			isxdigit((unsigned char)in[0]) &&
			isxdigit((unsigned char)in[1])) {
			if (sscanf(in, "%2x", &i) == 1 && i != 0) {
				*out++ = (char)i;
				in += 2;
				continue;
			}
		}

		if (c == '#' && strlen(in) >= 3 &&
			in[0] >= '0' && in[0] <= '7' &&
			in[1] >= '0' && in[1] <= '7' &&
			in[2] >= '0' && in[2] <= '7') {
			if (sscanf(in, "%3o", &i) == 1 && i != 0 && i <= 255) {
				*out++ = (char)i;
				in += 3;
				continue;
			}
		}

		*out++ = (char)c;
	}

	*out = '\0';
}


/*
 * Format number to human-readable filesize with unit
 */
void strfsize(char *out, off_t size, size_t outsize)
{
	static char *unit[] = { UNITS };
	int u;
	float s;

	/* Start with kilobytes */
	s = ((float) size) / 1024;
	u = 0;

	/* Loop through the units until the size is small enough */
	while (s >= 1000 && unit[(u + 1)]) {
		s = s / 1024;
		u++;
	}

	/* Format size */
	snprintf(out, outsize, "%7.1f %s", s, unit[u]);
}


#ifndef HAVE_STRLCPY
/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <string.h>

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

#endif
