// Use with newlib headers.
// Mixture of weblibc, mini-printf and ???

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

int errno;
int _write(int fd, const char *buf, int size);

int mini_vsnprintf(char *buffer, unsigned int buffer_len, const char *fmt, va_list va);
int mini_vpprintf(int (*puts)(char* s, int len, void* buf), void* buf, const char *fmt, va_list va);

static int __puts_uart(char *s, int len, void *buf)
{
	_write( 0, s, len );
}

int printf(const char* format, ...)
{
	va_list args;
	va_start( args, format );
	int ret_status = mini_vpprintf(__puts_uart, 0, format, args);
	va_end( args );
	return ret_status;
}


/* Some stuff from MUSL


----------------------------------------------------------------------
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------

*/

/*
 * mbstate_t is an opaque object to keep conversion state, during multibyte
 * stream conversions.  The content must not be referenced by user programs.
 */

#define CURRENT_UTF8 0
#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)
#define MB_CUR_MAX (CURRENT_UTF8 ? 4 : 1)

typedef void * mbstate_t;

size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict st)
{
	if (!s) return 1;
	if ((unsigned)wc < 0x80) {
		*s = wc;
		return 1;
	} else if (MB_CUR_MAX == 1) {
		if (!IS_CODEUNIT(wc)) {
			errno = 0x02; // EILSEQ
			return -1;
		}
		*s = wc;
		return 1;
	} else if ((unsigned)wc < 0x800) {
		*s++ = 0xc0 | (wc>>6);
		*s = 0x80 | (wc&0x3f);
		return 2;
	} else if ((unsigned)wc < 0xd800 || (unsigned)wc-0xe000 < 0x2000) {
		*s++ = 0xe0 | (wc>>12);
		*s++ = 0x80 | ((wc>>6)&0x3f);
		*s = 0x80 | (wc&0x3f);
		return 3;
	} else if ((unsigned)wc-0x10000 < 0x100000) {
		*s++ = 0xf0 | (wc>>18);
		*s++ = 0x80 | ((wc>>12)&0x3f);
		*s++ = 0x80 | ((wc>>6)&0x3f);
		*s = 0x80 | (wc&0x3f);
		return 4;
	}
	errno = 0x02;//EILSEQ;
	return -1;
}
int wctomb(char *s, wchar_t wc)
{
	if (!s) return 0;
	return wcrtomb(s, wc, 0);
}
size_t strlen(const char *s) { const char *a = s;for (; *s; s++);return s-a; }
size_t strnlen(const char *s, size_t n) { const char *p = memchr(s, 0, n); return p ? p-s : n;}
void *memset(void *dest, int c, size_t n) { unsigned char *s = dest; for (; n; n--, s++) *s = c; }
char *strcpy(char *d, const char *s) { for (; (*d=*s); s++, d++); }
char *strncpy(char *d, const char *s, size_t n) { for (; n && (*d=*s); n--, s++, d++); }
int strcmp(const char *l, const char *r)
{
	for (; *l==*r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}
int strncmp(const char *_l, const char *_r, size_t n)
{
	const unsigned char *l=(void *)_l, *r=(void *)_r;
	if (!n--) return 0;
	for (; *l && *r && n && *l == *r ; l++, r++, n--);
	return *l - *r;
}

static char *twobyte_strstr(const unsigned char *h, const unsigned char *n)
{
	uint16_t nw = n[0]<<8 | n[1], hw = h[0]<<8 | h[1];
	for (h++; *h && hw != nw; hw = hw<<8 | *++h);
	return *h ? (char *)h-1 : 0;
}

static char *threebyte_strstr(const unsigned char *h, const unsigned char *n)
{
	uint32_t nw = (uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8;
	uint32_t hw = (uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8;
	for (h+=2; *h && hw != nw; hw = (hw|*++h)<<8);
	return *h ? (char *)h-2 : 0;
}

static char *fourbyte_strstr(const unsigned char *h, const unsigned char *n)
{
	uint32_t nw = (uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8 | n[3];
	uint32_t hw = (uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8 | h[3];
	for (h+=3; *h && hw != nw; hw = hw<<8 | *++h);
	return *h ? (char *)h-3 : 0;
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

static char *twoway_strstr(const unsigned char *h, const unsigned char *n)
{
	const unsigned char *z;
	size_t l, ip, jp, k, p, ms, p0, mem, mem0;
	size_t byteset[32 / sizeof(size_t)] = { 0 };
	size_t shift[256];

	/* Computing length of needle and fill shift table */
	for (l=0; n[l] && h[l]; l++)
		BITOP(byteset, n[l], |=), shift[n[l]] = l+1;
	if (n[l]) return 0; /* hit the end of h */

	/* Compute maximal suffix */
	ip = -1; jp = 0; k = p = 1;
	while (jp+k<l) {
		if (n[ip+k] == n[jp+k]) {
			if (k == p) {
				jp += p;
				k = 1;
			} else k++;
		} else if (n[ip+k] > n[jp+k]) {
			jp += k;
			k = 1;
			p = jp - ip;
		} else {
			ip = jp++;
			k = p = 1;
		}
	}
	ms = ip;
	p0 = p;

	/* And with the opposite comparison */
	ip = -1; jp = 0; k = p = 1;
	while (jp+k<l) {
		if (n[ip+k] == n[jp+k]) {
			if (k == p) {
				jp += p;
				k = 1;
			} else k++;
		} else if (n[ip+k] < n[jp+k]) {
			jp += k;
			k = 1;
			p = jp - ip;
		} else {
			ip = jp++;
			k = p = 1;
		}
	}
	if (ip+1 > ms+1) ms = ip;
	else p = p0;

	/* Periodic needle? */
	if (memcmp(n, n+p, ms+1)) {
		mem0 = 0;
		p = MAX(ms, l-ms-1) + 1;
	} else mem0 = l-p;
	mem = 0;

	/* Initialize incremental end-of-haystack pointer */
	z = h;

	/* Search loop */
	for (;;) {
		/* Update incremental end-of-haystack pointer */
		if (z-h < l) {
			/* Fast estimate for MAX(l,63) */
			size_t grow = l | 63;
			const unsigned char *z2 = memchr(z, 0, grow);
			if (z2) {
				z = z2;
				if (z-h < l) return 0;
			} else z += grow;
		}

		/* Check last byte first; advance by shift on mismatch */
		if (BITOP(byteset, h[l-1], &)) {
			k = l-shift[h[l-1]];
			if (k) {
				if (k < mem) k = mem;
				h += k;
				mem = 0;
				continue;
			}
		} else {
			h += l;
			mem = 0;
			continue;
		}

		/* Compare right half */
		for (k=MAX(ms+1,mem); n[k] && n[k] == h[k]; k++);
		if (n[k]) {
			h += k-ms;
			mem = 0;
			continue;
		}
		/* Compare left half */
		for (k=ms+1; k>mem && n[k-1] == h[k-1]; k--);
		if (k <= mem) return (char *)h;
		h += p;
		mem = mem0;
	}
}

char *strstr(const char *h, const char *n)
{
	/* Return immediately on empty needle */
	if (!n[0]) return (char *)h;

	/* Use faster algorithms for short needles */
	h = strchr(h, *n);
	if (!h || !n[1]) return (char *)h;
	if (!h[1]) return 0;
	if (!n[2]) return twobyte_strstr((void *)h, (void *)n);
	if (!h[2]) return 0;
	if (!n[3]) return threebyte_strstr((void *)h, (void *)n);
	if (!h[3]) return 0;
	if (!n[4]) return fourbyte_strstr((void *)h, (void *)n);

	return twoway_strstr((void *)h, (void *)n);
}

char *strchr(const char *s, int c)
{
	c = (unsigned char)c;
	if (!c) return (char *)s + strlen(s);
	for (; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}


void *__memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return (void *)(s+n);
	return 0;
}

char *strrchr(const char *s, int c)
{
	return __memrchr(s, c, strlen(s) + 1);
}

void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	for (; n; n--) *d++ = *s++;
	return dest;
}

int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l=vl, *r=vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
}


void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d==s) return d;
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);

	if (d<s) {
		for (; n; n--) *d++ = *s++;
	} else {
		while (n) n--, d[n] = s[n];
	}

	return dest;
}
void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *s = src;
	c = (unsigned char)c;
	for (; n && *s != c; s++, n--);
	return n ? (void *)s : 0;
}

int puts(const char *s)
{
	int sl = strlen( s );
	_write(0, s, sl );
	_write(0, "\n", 1 );
	return sl + 1;
}

/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */

#define mini_strlen strlen

static int
mini_itoa(long value, unsigned int radix, int uppercase, int unsig,
	 char *buffer)
{
	char	*pbuffer = buffer;
	int	negative = 0;
	int	i, len;

	/* No support for unusual radixes. */
	if (radix > 16)
		return 0;

	if (value < 0 && !unsig) {
		negative = 1;
		value = -value;
	}

	/* This builds the string back to front ... */
	do {
		int digit = value % radix;
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		value /= radix;
	} while (value > 0);

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';

	/* ... now we reverse it (could do it recursively but will
	 * conserve the stack space) */
	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++) {
		char j = buffer[i];
		buffer[i] = buffer[len-i-1];
		buffer[len-i-1] = j;
	}

	return len;
}

static int
mini_pad(char* ptr, int len, char pad_char, int pad_to, char *buffer)
{
	int i;
	int overflow = 0;
	char * pbuffer = buffer;
	if(pad_to == 0) pad_to = len;
	if(len > pad_to) {
		len = pad_to;
		overflow = 1;
	}
	for(i = pad_to - len; i > 0; i --) {
		*(pbuffer++) = pad_char;
	}
	for(i = len; i > 0; i --) {
		*(pbuffer++) = *(ptr++);
	}
	len = pbuffer - buffer;
	if(overflow) {
		for (i = 0; i < 3 && pbuffer > buffer; i ++) {
			*(pbuffer-- - 1) = '*';
		}
	}
	return len;
}

struct mini_buff {
	char *buffer, *pbuffer;
	unsigned int buffer_len;
};

static int
_puts(char *s, int len, void *buf)
{
	if(!buf) return len;
	struct mini_buff *b = buf;
	char * p0 = b->buffer;
	int i;
	/* Copy to buffer */
	for (i = 0; i < len; i++) {
		if(b->pbuffer == b->buffer + b->buffer_len - 1) {
			break;
		}
		*(b->pbuffer ++) = s[i];
	}
	*(b->pbuffer) = 0;
	return b->pbuffer - p0;
}

#ifdef MINI_PRINTF_ENABLE_OBJECTS
static int (*mini_handler) (void* data, void* obj, int ch, int lhint, char** bf) = 0;
static void (*mini_handler_freeor)(void* data, void*) = 0;
static void * mini_handler_data = 0;

void mini_printf_set_handler(
	void* data,
	int (*handler)(void* data, void* obj, int ch, int len_hint, char** buf),
	void (*freeor)(void* data, void* buf))
{
	mini_handler = handler;
	mini_handler_freeor = freeor;
	mini_handler_data = data;
}
#endif

int
mini_vsnprintf(char *buffer, unsigned int buffer_len, const char *fmt, va_list va)
{
	struct mini_buff b;
	b.buffer = buffer;
	b.pbuffer = buffer;
	b.buffer_len = buffer_len;
	if(buffer_len == 0) buffer = (void*) 0;
	int n = mini_vpprintf(_puts, (buffer != (void*)0)?&b:(void*)0, fmt, va);
	if(buffer == (void*) 0) {
		return n;
	}
	return b.pbuffer - b.buffer;
}

int
mini_vpprintf(int (*puts)(char* s, int len, void* buf), void* buf, const char *fmt, va_list va)
{
	char bf[24];
	char bf2[24];
	char ch;
#ifdef MINI_PRINTF_ENABLE_OBJECTS
	void* obj;
#endif
	if(puts == (void*)0) {
		/* run puts in counting mode. */
		puts = _puts; buf = (void*)0;
	}
	int n = 0;
	while ((ch=*(fmt++))) {
		int len;
		if (ch!='%') {
			len = 1;
			len = puts(&ch, len, buf);
		} else {
			char pad_char = ' ';
			int pad_to = 0;
			char l = 0;
			char *ptr;

			ch=*(fmt++);

			/* Zero padding requested */
			if (ch == '0') pad_char = '0';
			while (ch >= '0' && ch <= '9') {
				pad_to = pad_to * 10 + (ch - '0');
				ch=*(fmt++);
			}
			if(pad_to > (signed int) sizeof(bf)) {
				pad_to = sizeof(bf);
			}
			if (ch == 'l') {
				l = 1;
				ch=*(fmt++);
			}

			switch (ch) {
				case 0:
					goto end;
				case 'u':
				case 'd':
					if(l) {
						len = mini_itoa(va_arg(va, unsigned long), 10, 0, (ch=='u'), bf2);
					} else {
						if(ch == 'u') {
							len = mini_itoa((unsigned long) va_arg(va, unsigned int), 10, 0, 1, bf2);
						} else {
							len = mini_itoa((long) va_arg(va, int), 10, 0, 0, bf2);
						}
					}
					len = mini_pad(bf2, len, pad_char, pad_to, bf);
					len = puts(bf, len, buf);
					break;

				case 'x':
				case 'X':
					if(l) {
						len = mini_itoa(va_arg(va, unsigned long), 16, (ch=='X'), 1, bf2);
					} else {
						len = mini_itoa((unsigned long) va_arg(va, unsigned int), 16, (ch=='X'), 1, bf2);
					}
					len = mini_pad(bf2, len, pad_char, pad_to, bf);
					len = puts(bf, len, buf);
					break;

				case 'c' :
					ch = (char)(va_arg(va, int));
					len = mini_pad(&ch, 1, pad_char, pad_to, bf);
					len = puts(bf, len, buf);
					break;

				case 's' :
					ptr = va_arg(va, char*);
					len = mini_strlen(ptr);
					if (pad_to > 0) {
						len = mini_pad(ptr, len, pad_char, pad_to, bf);
						len = puts(bf, len, buf);
					} else {
						len = puts(ptr, len, buf);
					}
					break;
#ifdef MINI_PRINTF_ENABLE_OBJECTS
				case 'O' :  /* Object by content (e.g. str) */
				case 'R' :  /* Object by representation (e.g. repr)*/
					obj = va_arg(va, void*);
					len = mini_handler(mini_handler_data, obj, ch, pad_to, &ptr);
					if (pad_to > 0) {
						len = mini_pad(ptr, len, pad_char, pad_to, bf);
						len = puts(bf, len, buf);
					} else {
						len = puts(ptr, len, buf);
					}
					mini_handler_freeor(mini_handler_data, ptr);
					break;
#endif
				default:
					len = 1;
					len = puts(&ch, len, buf);
					break;
			}
		}
		n = n + len;
	}
end:
	return n;
}


int
mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...)
{
	int ret;
	va_list va;
	va_start(va, fmt);
	ret = mini_vsnprintf(buffer, buffer_len, fmt, va);
	va_end(va);

	return ret;
}

int
mini_pprintf(int (*puts)(char*s, int len, void* buf), void* buf, const char *fmt, ...)
{
	int ret;
	va_list va;
	va_start(va, fmt);
	ret = mini_vpprintf(puts, buf, fmt, va);
	va_end(va);

	return ret;
}

