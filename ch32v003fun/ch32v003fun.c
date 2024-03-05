// Mixture of embedlibc, and ch32v00x_startup.c



// Use with newlib headers.
// Mixture of weblibc, mini-printf and ???

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>
#include <ch32v003fun.h>

int errno;

int mini_vsnprintf(char *buffer, unsigned int buffer_len, const char *fmt, va_list va);
int mini_vpprintf(int (*puts)(char* s, int len, void* buf), void* buf, const char *fmt, va_list va);

static int __puts_uart(char *s, int len, void *buf)
{
	_write( 0, s, len );
	return len;
}

int printf(const char* format, ...)
{
	va_list args;
	va_start( args, format );
	int ret_status = mini_vpprintf(__puts_uart, 0, format, args);
	va_end( args );
	return ret_status;
}

int vprintf(const char* format, va_list args)
{
	return mini_vpprintf(__puts_uart, 0, format, args);
}

int snprintf(char * buffer, unsigned int buffer_len, const char* format, ...)
{
	va_list args;
	va_start( args, format );
	int ret = mini_vsnprintf( buffer, buffer_len, format, args );
	va_end( args );
	return ret;
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

#ifdef UNICODE
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
#endif
size_t strlen(const char *s) { const char *a = s;for (; *s; s++);return s-a; }
size_t strnlen(const char *s, size_t n) { const char *p = memchr(s, 0, n); return p ? p-s : n;}
void *memset(void *dest, int c, size_t n) { unsigned char *s = dest; for (; n; n--, s++) *s = c; return dest; }
char *strcpy(char *d, const char *s) { for (; (*d=*s); s++, d++); return d; }
char *strncpy(char *d, const char *s, size_t n) { for (; n && (*d=*s); n--, s++, d++); return d; }
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


/*
	C version of CH32V003 Startup .s file from WCH
	This file is public domain where possible or the following where not:
	Copyright 2023 Charles Lohr, under the MIT-x11 or NewBSD licenses, you choose.
*/

#ifdef CPLUSPLUS
// Method to call the C++ constructors
void __libc_init_array(void);
#endif

int main() __attribute__((used));
void SystemInit( void ) __attribute__((used));

extern uint32_t * _sbss;
extern uint32_t * _ebss;
extern uint32_t * _data_lma;
extern uint32_t * _data_vma;
extern uint32_t * _edata;


// If you don't override a specific handler, it will just spin forever.
void DefaultIRQHandler( void )
{
	// Infinite Loop
	asm volatile( "1: j 1b" );
}

// This makes it so that all of the interrupt handlers just alias to
// DefaultIRQHandler unless they are individually overridden.

#if defined(FUNCONF_USE_CLK_SEC) && FUNCONF_USE_CLK_SEC
/**
 * @brief 	Non Maskabke Interrupt handler
 * 			Invoked when the Clock Security system
 * 			detects the failure of the HSE oscilator.
 * 			The sys clock is switched to HSI.
 * 			Clears the CSSF flag in RCC->INTR
 */
void NMI_RCC_CSS_IRQHandler( void )
{
	RCC->INTR |= RCC_CSSC;	// clear the clock security int flag
}

void NMI_Handler( void ) 				 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("NMI_RCC_CSS_IRQHandler"))) __attribute__((used));
#else
void NMI_Handler( void )                 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif
void HardFault_Handler( void )           __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#if defined(CH32V20x) || defined(CH32V30x)
void Ecall_M_Mode_Handler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void Ecall_U_Mode_Handler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void Break_Point_Handler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V20x) || defined(CH32V30x)
void SysTick_Handler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SW_Handler( void )                  __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void WWDG_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void PVD_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#if defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void TAMPER_IRQHandler( void )			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void RTC_IRQHandler( void )				 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void FLASH_IRQHandler( void )            __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void RCC_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#ifdef CH32V003
void EXTI7_0_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void AWU_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#elif defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void EXTI0_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI1_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI2_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI3_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI4_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif
void DMA1_Channel1_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel2_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel3_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel4_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel5_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel6_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel7_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#ifdef CH32V003
void ADC1_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#elif defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void ADC1_2_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif
#if defined(CH32V20x) || defined(CH32V30x)
void USB_HP_CAN1_TX_IRQHandler( void ) 	 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USB_LP_CAN1_RX0_IRQHandler( void )  __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void CAN1_RX1_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void CAN1_SCE_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V20x) || defined(CH32V30x)
#if defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void EXTI9_5_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void TIM1_BRK_IRQHandler( void )         __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_UP_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_TRG_COM_IRQHandler( void )     __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_CC_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM2_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#if defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void TIM3_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM4_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void I2C1_EV_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void I2C1_ER_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#ifdef CH32V003
void USART1_IRQHandler( void )           __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SPI1_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#elif defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)
void I2C2_EV_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void I2C2_ER_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SPI1_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SPI2_IRQHandler( void )			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USART1_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USART2_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USART3_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI15_10_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void RTCAlarm_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif
#if defined(CH32V10x) || defined(CH32V20x)
void USBWakeUp_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USBHD_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif // defined(CH32V10x) || defined(CH32V20x)
#if defined(CH32V20x)
void USBHDWakeUp_IRQHandler( void ) 	 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void ETH_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void ETHWakeUp_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM5_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void OSC32KCal_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void OSCWakeUp_IRQHandler( void ) 		 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART4_IRQHandler( void ) 			 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel8_IRQHandler( void ) 	 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#elif defined(CH32V30x)
void TIM8_BRK_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM8_UP_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM8_TRG_COM_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM8_CC_IRQHandler( void )			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void RNG_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void FSMC_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SDIO_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM5_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SPI3_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART4_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART5_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM6_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM7_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel1_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel2_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel3_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel4_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel5_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void OTG_FS_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART6_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART7_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void UART8_IRQHandler( void ) 			__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM9_BRK_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM9_UP_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM9_TRG_COM_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM9_CC_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM10_BRK_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM10_UP_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM10_TRG_COM_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM10_CC_IRQHandler( void ) 		__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel6_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel7_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel8_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel9_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel10_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA2_Channel11_IRQHandler( void ) 	__attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
#endif

#ifdef CH32V003

void InterruptVector()         __attribute__((naked)) __attribute((section(".init"))) __attribute((weak,alias("InterruptVectorDefault")));
void InterruptVectorDefault()  __attribute__((naked)) __attribute((section(".init")));

void InterruptVectorDefault()
{
	asm volatile( "\n\
	.align  2\n\
	.option   push;\n\
	.option   norvc;\n\
	j handle_reset\n" );
#if !defined(FUNCONF_TINYVECTOR) || !FUNCONF_TINYVECTOR
	asm volatile( "\n\
	.word   0\n\
	.word   NMI_Handler               /* NMI Handler */                    \n\
	.word   HardFault_Handler         /* Hard Fault Handler */             \n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   SysTick_Handler           /* SysTick Handler */                \n\
	.word   0\n\
	.word   SW_Handler                /* SW Handler */                     \n\
	.word   0\n\
	/* External Interrupts */                                              \n\
	.word   WWDG_IRQHandler           /* Window Watchdog */                \n\
	.word   PVD_IRQHandler            /* PVD through EXTI Line detect */   \n\
	.word   FLASH_IRQHandler          /* Flash */                          \n\
	.word   RCC_IRQHandler            /* RCC */                            \n\
	.word   EXTI7_0_IRQHandler        /* EXTI Line 7..0 */                 \n\
	.word   AWU_IRQHandler            /* AWU */                            \n\
	.word   DMA1_Channel1_IRQHandler  /* DMA1 Channel 1 */                 \n\
	.word   DMA1_Channel2_IRQHandler  /* DMA1 Channel 2 */                 \n\
	.word   DMA1_Channel3_IRQHandler  /* DMA1 Channel 3 */                 \n\
	.word   DMA1_Channel4_IRQHandler  /* DMA1 Channel 4 */                 \n\
	.word   DMA1_Channel5_IRQHandler  /* DMA1 Channel 5 */                 \n\
	.word   DMA1_Channel6_IRQHandler  /* DMA1 Channel 6 */                 \n\
	.word   DMA1_Channel7_IRQHandler  /* DMA1 Channel 7 */                 \n\
	.word   ADC1_IRQHandler           /* ADC1 */                           \n\
	.word   I2C1_EV_IRQHandler        /* I2C1 Event */                     \n\
	.word   I2C1_ER_IRQHandler        /* I2C1 Error */                     \n\
	.word   USART1_IRQHandler         /* USART1 */                         \n\
	.word   SPI1_IRQHandler           /* SPI1 */                           \n\
	.word   TIM1_BRK_IRQHandler       /* TIM1 Break */                     \n\
	.word   TIM1_UP_IRQHandler        /* TIM1 Update */                    \n\
	.word   TIM1_TRG_COM_IRQHandler   /* TIM1 Trigger and Commutation */   \n\
	.word   TIM1_CC_IRQHandler        /* TIM1 Capture Compare */           \n\
	.word   TIM2_IRQHandler           /* TIM2 */                           \n\
");
#endif
	asm volatile( ".option   pop;\n");
}

void handle_reset()
{
	asm volatile( "\n\
.option push\n\
.option norelax\n\
	la gp, __global_pointer$\n\
.option pop\n\
	la sp, _eusrstack\n"
#if __GNUC__ > 10
".option arch, +zicsr\n"
#endif
	// Setup the interrupt vector, processor status and INTSYSCR.
"	li a0, 0x80\n\
	csrw mstatus, a0\n\
	li a3, 0x3\n\
	la a0, InterruptVector\n\
	or a0, a0, a3\n\
	csrw mtvec, a0\n" 
	: : : "a0", "a3", "memory");

	// Careful: Use registers to prevent overwriting of self-data.
	// This clears out BSS.
asm volatile(
"	la a0, _sbss\n\
	la a1, _ebss\n\
	li a2, 0\n\
	bge a0, a1, 2f\n\
1:	sw a2, 0(a0)\n\
	addi a0, a0, 4\n\
	blt a0, a1, 1b\n\
2:"
	// This loads DATA from FLASH to RAM.
"	la a0, _data_lma\n\
	la a1, _data_vma\n\
	la a2, _edata\n\
1:	beq a1, a2, 2f\n\
	lw a3, 0(a0)\n\
	sw a3, 0(a1)\n\
	addi a0, a0, 4\n\
	addi a1, a1, 4\n\
	bne a1, a2, 1b\n\
2:\n"
#ifdef CPLUSPLUS
	// Call __libc_init_array function
"	call %0 \n\t"
: : "i" (__libc_init_array) 
: "a0", "a1", "a2", "a3", "a4", "a5", "t0", "t1", "t2", "memory"
#else
: : : "a0", "a1", "a2", "a3", "memory"
#endif
);


#if defined( FUNCONF_SYSTICK_USE_HCLK ) && FUNCONF_SYSTICK_USE_HCLK
	SysTick->CTLR = 5;
#else
	SysTick->CTLR = 1;
#endif

	// set mepc to be main as the root app.
asm volatile(
"	csrw mepc, %[main]\n"
"	mret\n" : : [main]"r"(main) );
}

#elif defined(CH32V10x) || defined(CH32V20x) || defined(CH32V30x)

void Init() 				   __attribute((section(".init"))) __attribute((used));
void InterruptVector()         __attribute__((naked)) __attribute((section(".vector"))) __attribute((weak,alias("InterruptVectorDefault")));
void InterruptVectorDefault()  __attribute__((naked)) __attribute((section(".vector")));

void handle_reset( void ) __attribute__((section(".text.handle_reset")));

void Init()
{
	asm volatile( "\n\
	.align	1 \n\
_start: \n\
	j handle_reset \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00000013 \n\
	.word 0x00100073 \n" );
}

void InterruptVectorDefault()
{
	asm volatile( "\n\
	.align	1 \n\
	.option norvc; \n\
	.word   Init \n"
#if !defined(FUNCONF_TINYVECTOR) || !FUNCONF_TINYVECTOR
"	.word   0 \n\
	.word   NMI_Handler                /* NMI */ \n\
	.word   HardFault_Handler          /* Hard Fault */ \n\
	.word   0 \n"
#if !defined(CH32V10x)
"	.word   Ecall_M_Mode_Handler       /* Ecall M Mode */ \n\
	.word   0 \n\
	.word   0 \n\
	.word   Ecall_U_Mode_Handler       /* Ecall U Mode */ \n\
	.word   Break_Point_Handler        /* Break Point */ \n\
	.word   0 \n\
	.word   0 \n"
#else
"	.word   0 \n\
	.word   0 \n\
	.word   0 \n\
	.word   0 \n\
	.word   0 \n\
	.word   0 \n\
	.word   0 \n"
#endif
"	.word   SysTick_Handler            /* SysTick */ \n\
	.word   0 \n\
	.word   SW_Handler                 /* SW */ \n\
	.word   0 \n\
	/* External Interrupts */ \n\
	.word   WWDG_IRQHandler            /* Window Watchdog */ \n\
	.word   PVD_IRQHandler             /* PVD through EXTI Line detect */ \n\
	.word   TAMPER_IRQHandler          /* TAMPER */ \n\
	.word   RTC_IRQHandler             /* RTC */ \n\
	.word   FLASH_IRQHandler           /* Flash */ \n\
	.word   RCC_IRQHandler             /* RCC */ \n\
	.word   EXTI0_IRQHandler           /* EXTI Line 0 */ \n\
	.word   EXTI1_IRQHandler           /* EXTI Line 1 */ \n\
	.word   EXTI2_IRQHandler           /* EXTI Line 2 */ \n\
	.word   EXTI3_IRQHandler           /* EXTI Line 3 */ \n\
	.word   EXTI4_IRQHandler           /* EXTI Line 4 */ \n\
	.word   DMA1_Channel1_IRQHandler   /* DMA1 Channel 1 */ \n\
	.word   DMA1_Channel2_IRQHandler   /* DMA1 Channel 2 */ \n\
	.word   DMA1_Channel3_IRQHandler   /* DMA1 Channel 3 */ \n\
	.word   DMA1_Channel4_IRQHandler   /* DMA1 Channel 4 */ \n\
	.word   DMA1_Channel5_IRQHandler   /* DMA1 Channel 5 */ \n\
	.word   DMA1_Channel6_IRQHandler   /* DMA1 Channel 6 */ \n\
	.word   DMA1_Channel7_IRQHandler   /* DMA1 Channel 7 */ \n\
	.word   ADC1_2_IRQHandler          /* ADC1_2 */ \n"
#if !defined(CH32V10x)
"	.word   USB_HP_CAN1_TX_IRQHandler  /* USB HP and CAN1 TX */ \n\
	.word   USB_LP_CAN1_RX0_IRQHandler /* USB LP and CAN1RX0 */ \n\
	.word   CAN1_RX1_IRQHandler        /* CAN1 RX1 */ \n\
	.word   CAN1_SCE_IRQHandler        /* CAN1 SCE */ \n"
#else
"	.word   0 \n\
	.word   0 \n\
	.word   0 \n\
	.word   0 \n"
#endif
"	.word   EXTI9_5_IRQHandler         /* EXTI Line 9..5 */ \n\
	.word   TIM1_BRK_IRQHandler        /* TIM1 Break */ \n\
	.word   TIM1_UP_IRQHandler         /* TIM1 Update */ \n\
	.word   TIM1_TRG_COM_IRQHandler    /* TIM1 Trigger and Commutation */ \n\
	.word   TIM1_CC_IRQHandler         /* TIM1 Capture Compare */ \n\
	.word   TIM2_IRQHandler            /* TIM2 */ \n\
	.word   TIM3_IRQHandler            /* TIM3 */ \n\
	.word   TIM4_IRQHandler            /* TIM4 */ \n\
	.word   I2C1_EV_IRQHandler         /* I2C1 Event */ \n\
	.word   I2C1_ER_IRQHandler         /* I2C1 Error */ \n\
	.word   I2C2_EV_IRQHandler         /* I2C2 Event */ \n\
	.word   I2C2_ER_IRQHandler         /* I2C2 Error */ \n\
	.word   SPI1_IRQHandler            /* SPI1 */ \n\
	.word   SPI2_IRQHandler            /* SPI2 */ \n\
	.word   USART1_IRQHandler          /* USART1 */ \n\
	.word   USART2_IRQHandler          /* USART2 */ \n\
	.word   USART3_IRQHandler          /* USART3 */ \n\
	.word   EXTI15_10_IRQHandler       /* EXTI Line 15..10 */ \n\
	.word   RTCAlarm_IRQHandler        /* RTC Alarm through EXTI Line */ \n"
#if defined(CH32V10x) || defined(CH32V20x)
"	.word   USBWakeUp_IRQHandler       /* USB Wake up from suspend */ \n\
	.word   USBHD_IRQHandler           /* USBHD Break */ \n"
#endif
#if defined(CH32V20x)
"	.word   USBHDWakeUp_IRQHandler     /* USBHD Wake up from suspend */ \n\
	.word   ETH_IRQHandler             /* ETH global */ \n\
	.word   ETHWakeUp_IRQHandler       /* ETH Wake up */ \n\
	.word   0                          /* BLE BB */ \n\
	.word   0                          /* BLE LLE */ \n\
	.word   TIM5_IRQHandler            /* TIM5 */ \n\
	.word   UART4_IRQHandler           /* UART4 */ \n\
	.word   DMA1_Channel8_IRQHandler   /* DMA1 Channel8 */ \n\
	.word   OSC32KCal_IRQHandler       /* OSC32KCal */ \n\
	.word   OSCWakeUp_IRQHandler       /* OSC Wake Up */ \n"
#elif defined(CH32V30x)
"	.word   0 \n\
    .word   TIM8_BRK_IRQHandler        /* TIM8 Break */ \n\
    .word   TIM8_UP_IRQHandler         /* TIM8 Update */ \n\
    .word   TIM8_TRG_COM_IRQHandler    /* TIM8 Trigger and Commutation */ \n\
    .word   TIM8_CC_IRQHandler         /* TIM8 Capture Compare */ \n\
    .word   RNG_IRQHandler             /* RNG */ \n\
    .word   FSMC_IRQHandler            /* FSMC */ \n\
    .word   SDIO_IRQHandler            /* SDIO */ \n\
    .word   TIM5_IRQHandler            /* TIM5 */ \n\
    .word   SPI3_IRQHandler            /* SPI3 */ \n\
    .word   UART4_IRQHandler           /* UART4 */ \n\
    .word   UART5_IRQHandler           /* UART5 */ \n\
    .word   TIM6_IRQHandler            /* TIM6 */ \n\
    .word   TIM7_IRQHandler            /* TIM7 */ \n\
    .word   DMA2_Channel1_IRQHandler   /* DMA2 Channel 1 */ \n\
    .word   DMA2_Channel2_IRQHandler   /* DMA2 Channel 2 */ \n\
    .word   DMA2_Channel3_IRQHandler   /* DMA2 Channel 3 */ \n\
    .word   DMA2_Channel4_IRQHandler   /* DMA2 Channel 4 */ \n\
    .word   DMA2_Channel5_IRQHandler   /* DMA2 Channel 5 */ \n\
    .word   0 \n\
    .word   0 \n\
    .word   0 \n\
    .word   0 \n\
    .word   0 \n\
    .word   0 \n\
    .word   OTG_FS_IRQHandler          /* OTGFS */ \n\
    .word   0 \n\
    .word   0 \n\
    .word   0 \n\
    .word   UART6_IRQHandler           /* UART6 */ \n\
    .word   UART7_IRQHandler           /* UART7 */ \n\
    .word   UART8_IRQHandler           /* UART8 */ \n\
    .word   TIM9_BRK_IRQHandler        /* TIM9 Break */ \n\
    .word   TIM9_UP_IRQHandler         /* TIM9 Update */ \n\
    .word   TIM9_TRG_COM_IRQHandler    /* TIM9 Trigger and Commutation */ \n\
    .word   TIM9_CC_IRQHandler         /* TIM9 Capture Compare */ \n\
    .word   TIM10_BRK_IRQHandler       /* TIM10 Break */ \n\
    .word   TIM10_UP_IRQHandler        /* TIM10 Update */ \n\
    .word   TIM10_TRG_COM_IRQHandler   /* TIM10 Trigger and Commutation */ \n\
    .word   TIM10_CC_IRQHandler        /* TIM10 Capture Compare */ \n\
    .word   DMA2_Channel6_IRQHandler   /* DMA2 Channel 6 */ \n\
    .word   DMA2_Channel7_IRQHandler   /* DMA2 Channel 7 */ \n\
    .word   DMA2_Channel8_IRQHandler   /* DMA2 Channel 8 */ \n\
    .word   DMA2_Channel9_IRQHandler   /* DMA2 Channel 9 */ \n\
    .word   DMA2_Channel10_IRQHandler  /* DMA2 Channel 10 */ \n\
    .word   DMA2_Channel11_IRQHandler  /* DMA2 Channel 11 */ \n"
#endif
#endif
"	.option rvc; \n");

}

void handle_reset( void )
{
	asm volatile( "\n\
.option push\n\
.option norelax\n\
	la gp, __global_pointer$\n\
.option pop\n\
	la sp, _eusrstack\n"
#if __GNUC__ > 10
  ".option arch, +zicsr\n"
#endif
			);

	// Careful: Use registers to prevent overwriting of self-data.
	// This clears out BSS.
	asm volatile(
"	la a0, _sbss\n\
	la a1, _ebss\n\
	bgeu a0, a1, 2f\n\
1:	sw zero, 0(a0)\n\
	addi a0, a0, 4\n\
	bltu a0, a1, 1b\n\
2:"
	// This loads DATA from FLASH to RAM.
"	la a0, _data_lma\n\
	la a1, _data_vma\n\
	la a2, _edata\n\
	beq a1, a2, 2f\n\
1:	lw t0, 0(a0)\n\
	sw t0, 0(a1)\n\
	addi a0, a0, 4\n\
	addi a1, a1, 4\n\
	bltu a1, a2, 1b\n\
2:\n"
#ifdef CPLUSPLUS
	// Call __libc_init_array function
"	call %0 \n\t"
: : "i" (__libc_init_array)
#else
: :
#endif
: "a0", "a1", "a2", "a3", "memory"
);

	// Setup the interrupt vector, processor status and INTSYSCR.
	asm volatile(
"	li t0, 0x1f\n\
	csrw 0xbc0, t0\n"
#if defined(CH32V20x)
	// Enabled nested and hardware stack
"	li t0, 0x88\n\
	csrs mstatus, t0\n"
#elif defined(CH32V30x)
	// Enable nested and hardware stack
"	li t0, 0x0b\n\
	csrw 0x804, t0\n"
	// Enable floating point and interrupt
"	li t0, 0x688\n\
	csrs mstatus, t0\n"
#endif
"	la t0, InterruptVector\n\
	ori t0, t0, 3\n\
	csrw mtvec, t0\n"
	: : "InterruptVector" (InterruptVector) : "t0", "memory"
	);

#if defined( FUNCONF_SYSTICK_USE_HCLK ) && FUNCONF_SYSTICK_USE_HCLK && !defined(CH32V10x)
	SysTick->CTLR = 5;
#else
	SysTick->CTLR = 1;
#endif

	// set mepc to be main as the root app.
	asm volatile(
"	csrw mepc, %[main]\n"
"	mret\n" : : [main]"r"(main) );
}

#endif

#if defined( FUNCONF_USE_UARTPRINTF ) && FUNCONF_USE_UARTPRINTF
void SetupUART( int uartBRR )
{
#ifdef CH32V003
	// Enable GPIOD and UART.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

	// Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
	GPIOD->CFGLR &= ~(0xf<<(4*5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
#else
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1;

	// Push-Pull, 10MHz Output, GPIO A9, with AutoFunction
	GPIOA->CFGHR &= ~(0xf<<(4*1));
	GPIOA->CFGHR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);
#endif

	// 115200, 8n1.  Note if you don't specify a mode, UART remains off even when UE_Set.
	USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx;
	USART1->CTLR2 = USART_StopBits_1;
	USART1->CTLR3 = USART_HardwareFlowControl_None;

	USART1->BRR = uartBRR;
	USART1->CTLR1 |= CTLR1_UE_Set;
}

// For debug writing to the UART.
int _write(int fd, const char *buf, int size)
{
	for(int i = 0; i < size; i++){
	    while( !(USART1->STATR & USART_FLAG_TC));
	    USART1->DATAR = *buf++;
	}
	return size;
}

// single char to UART
int putchar(int c)
{
	while( !(USART1->STATR & USART_FLAG_TC));
	USART1->DATAR = (const char)c;
	return 1;
}
#endif

#if defined( FUNCONF_USE_DEBUGPRINTF ) && FUNCONF_USE_DEBUGPRINTF


void handle_debug_input( int numbytes, uint8_t * data ) __attribute__((weak));
void handle_debug_input( int numbytes, uint8_t * data ) { }

static void internal_handle_input( volatile uint32_t * dmdata0 )
{
	uint32_t dmd0 = *dmdata0;
	int bytes = (dmd0 & 0x3f) - 4;
	if( bytes > 0 )
	{
		handle_debug_input( bytes, ((uint8_t*)dmdata0) + 1 );
	}
}


void poll_input()
{
	volatile uint32_t * dmdata0 = (volatile uint32_t *)DMDATA0;
 	if( ((*dmdata0) & 0x80) == 0 )
	{
		internal_handle_input( dmdata0 );
		// Should be 0x80 or so, but for some reason there's a bug that retriggers.
		*dmdata0 = 0x00;
	}
}


//           MSB .... LSB
// DMDATA0: char3 char2 char1 [status word]
// where [status word] is:
//   b7 = is a "printf" waiting?
//   b0..b3 = # of bytes in printf (+4).  (5 or higher indicates a print of some kind)
//     note: if b7 is 0 in reply, but b0..b3 have >=4 then we received data from host.

int _write(int fd, const char *buf, int size)
{
	char buffer[4] = { 0 };
	int place = 0;
	uint32_t lastdmd;
	uint32_t timeout = FUNCONF_DEBUGPRINTF_TIMEOUT; // Give up after ~40ms

	if( size == 0 )
	{
		lastdmd = (*DMDATA0);
		if( lastdmd && !(lastdmd&0x80) ) internal_handle_input( (uint32_t*)DMDATA0 );
	}
	while( place < size )
	{
		int tosend = size - place;
		if( tosend > 7 ) tosend = 7;

		while( ( lastdmd = (*DMDATA0) ) & 0x80 )
			if( timeout-- == 0 ) return place;

		if( lastdmd ) internal_handle_input( (uint32_t*)DMDATA0 );

		timeout = FUNCONF_DEBUGPRINTF_TIMEOUT;

		int t = 3;
		while( t < tosend )
		{
			buffer[t-3] = buf[t+place];
			t++;
		}
		*DMDATA1 = *(uint32_t*)&(buffer[0]);
		t = 0;
		while( t < tosend && t < 3 )
		{
			buffer[t+1] = buf[t+place];
			t++;
		}
		buffer[0] = 0x80 | (tosend + 4);
		*DMDATA0 = *(uint32_t*)&(buffer[0]);

		//buf += tosend;
		place += tosend;
	}
	return size;
}

// single to debug intf
int putchar(int c)
{
	int timeout = FUNCONF_DEBUGPRINTF_TIMEOUT;
	uint32_t lastdmd = 0;

	while( ( lastdmd = (*DMDATA0) ) & 0x80 )
		if( timeout-- == 0 ) return 0;

	// Simply seeking input.
	if( lastdmd ) internal_handle_input( (uint32_t*)DMDATA0 );
	*DMDATA0 = 0x85 | ((const char)c<<8);
	return 1;
}

void SetupDebugPrintf()
{
	// Clear out the sending flag.
	*DMDATA1 = 0x0;
	*DMDATA0 = 0x80;
}

void WaitForDebuggerToAttach()
{
	while( ((*DMDATA0) & 0x80) );
}

#endif

#if (defined( FUNCONF_USE_DEBUGPRINTF ) && !FUNCONF_USE_DEBUGPRINTF) && \
    (defined( FUNCONF_USE_UARTPRINTF ) && !FUNCONF_USE_UARTPRINTF) && \
    (defined( FUNCONF_NULL_PRINTF ) && FUNCONF_NULL_PRINTF)

int _write(int fd, const char *buf, int size)
{
	return size;
}

// single to debug intf
int putchar(int c)
{
	return 1;
}
#endif

void DelaySysTick( uint32_t n )
{
#ifdef CH32V003
	uint32_t targend = SysTick->CNT + n;
	while( ((int32_t)( SysTick->CNT - targend )) < 0 );
#elif defined(CH32V20x) || defined(CH32V30x)
	uint64_t targend = SysTick->CNT + n;
	while( ((int64_t)( SysTick->CNT - targend )) < 0 );
#elif defined(CH32V10x)
	uint32_t targend = SysTick->CNTL + n;
	while( ((int32_t)( SysTick->CNTL - targend )) < 0 );
#endif
}

void SystemInit()
{
#if FUNCONF_HSE_BYPASS
	#define HSEBYP (1<<18)
#else
	#define HSEBYP 0
#endif

#if defined(FUNCONF_USE_CLK_SEC) && FUNCONF_USE_CLK_SEC
#define RCC_CSS RCC_CSSON									 	// Enable clock security system
#else
#define RCC_CSS 0
#endif

#if defined(FUNCONF_USE_PLL) && FUNCONF_USE_PLL
	#if defined(CH32V003)
		#define BASE_CFGR0 RCC_HPRE_DIV1 | RCC_PLLSRC_HSI_Mul2    // HCLK = SYSCLK = APB1 And, enable PLL
	#else
		#define BASE_CFGR0 RCC_HPRE_DIV1 | RCC_PPRE2_DIV1 | RCC_PPRE1_DIV2 | PLL_MULTIPLICATION
	#endif
#else
	#if defined(CH32V003)
		#define BASE_CFGR0 RCC_HPRE_DIV1     					  // HCLK = SYSCLK = APB1 And, no pll.
	#else
		#define BASE_CFGR0 RCC_HPRE_DIV1 | RCC_PPRE2_DIV1 | RCC_PPRE1_DIV1
	#endif
#endif

// HSI always ON - needed for the Debug subsystem
#define BASE_CTLR	(((FUNCONF_HSITRIM) << 3) | RCC_HSION | HSEBYP | RCC_CSS)
//#define BASE_CTLR	(((FUNCONF_HSITRIM) << 3) | HSEBYP | RCC_CSS)	// disable HSI in HSE modes

#if defined(FUNCONF_USE_HSI) && FUNCONF_USE_HSI
	#if defined(CH32V30x) || defined(CH32V20x) || defined(CH32V10x)
		EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;
	#endif
	#if defined(FUNCONF_USE_PLL) && FUNCONF_USE_PLL
		RCC->CFGR0 = BASE_CFGR0;
		RCC->CTLR  = BASE_CTLR | RCC_HSION | RCC_PLLON; 			// Use HSI, enable PLL.
	#else
		RCC->CFGR0 = RCC_HPRE_DIV1;                               	// PLLCLK = HCLK = SYSCLK = APB1
		RCC->CTLR  = BASE_CTLR | RCC_HSION;     					// Use HSI, Only.
	#endif

#elif defined(FUNCONF_USE_HSE) && FUNCONF_USE_HSE

	#if defined(CH32V003)
		RCC->CTLR = BASE_CTLR | RCC_HSION | RCC_HSEON ;       		  // Keep HSI on while turning on HSE
	#else
		RCC->CTLR = RCC_HSEON;							  			  // Only turn on HSE.
	#endif

	// Values lifted from the EVT.  There is little to no documentation on what this does.
	while(!(RCC->CTLR&RCC_HSERDY));

	#if defined(CH32V003)
		RCC->CFGR0 = RCC_PLLSRC_HSE_Mul2 | RCC_SW_HSE;
	#else
		RCC->CFGR0 = BASE_CFGR0 | RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE;
	#endif

	#if defined(FUNCONF_USE_PLL) && FUNCONF_USE_PLL
		RCC->CTLR  = BASE_CTLR | RCC_HSEON | RCC_PLLON;            // Turn off HSI.
	#else
		RCC->CTLR  = RCC_HSEON | HSEBYP;                           // Turn off PLL and HSI.
	#endif
#endif

	// CH32V10x flash prefetch buffer
#if defined(CH32V10x)
	// Enable Prefetch Buffer
	FLASH->ACTLR |= FLASH_ACTLR_PRFTBE;
#endif

	// CH32V003 flash latency
#if defined(CH32V003)
	#if FUNCONF_SYSTEM_CORE_CLOCK > 25000000
		FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;               // +1 Cycle Latency
	#else
		FLASH->ACTLR = FLASH_ACTLR_LATENCY_0;               // +0 Cycle Latency
	#endif
#endif

	// CH32V10x flash latency
#if defined(CH32V10x)
	#if defined(FUNCONF_USE_HSE) && FUNCONF_USE_HSE
		#if !defined(FUNCONF_USE_PLL) || !FUNCONF_USE_PLL
			FLASH->ACTLR = FLASH_ACTLR_LATENCY_0;           // +0 Cycle Latency
		#else
			#if FUNCONF_SYSTEM_CORE_CLOCK < 56000000
				FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;       // +1 Cycle Latency
			#else
				FLASH->ACTLR = FLASH_ACTLR_LATENCY_2;       // +2 Cycle Latency
			#endif
		#endif
	#else
		FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;       		// +1 Cycle Latency
	#endif
#endif

	RCC->INTR  = 0x009F0000;                               // Clear PLL, CSSC, HSE, HSI and LSI ready flags.

#if defined(FUNCONF_USE_PLL) && FUNCONF_USE_PLL
	while((RCC->CTLR & RCC_PLLRDY) == 0);                       	// Wait till PLL is ready
	uint32_t tmp32 = RCC->CFGR0 & ~(0x03);							// clr the SW
	RCC->CFGR0 = tmp32 | RCC_SW_PLL;                       			// Select PLL as system clock source
	while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08); 	// Wait till PLL is used as system clock source
#endif

#if defined( FUNCONF_USE_UARTPRINTF ) && FUNCONF_USE_UARTPRINTF
	SetupUART( UART_BRR );
#endif
#if defined( FUNCONF_USE_DEBUGPRINTF ) && FUNCONF_USE_DEBUGPRINTF
	SetupDebugPrintf();
#endif
}

// C++ Support

#ifdef CPLUSPLUS
// This is required to allow pure virtual functions to be defined.
extern void __cxa_pure_virtual() { while (1); }

// These magic symbols are provided by the linker.
extern void (*__preinit_array_start[]) (void) __attribute__((weak));
extern void (*__preinit_array_end[]) (void) __attribute__((weak));
extern void (*__init_array_start[]) (void) __attribute__((weak));
extern void (*__init_array_end[]) (void) __attribute__((weak));

void __libc_init_array(void)
{
	size_t count;
	size_t i;

	count = __preinit_array_end - __preinit_array_start;
	for (i = 0; i < count; i++)
		__preinit_array_start[i]();

	count = __init_array_end - __init_array_start;
	for (i = 0; i < count; i++)
		__init_array_start[i]();
}
#endif
