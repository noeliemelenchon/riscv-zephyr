/* string.c - common string routines */

/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

typedef unsigned int uint;
/*
 * Has _libsafe_die() been called?
 */
static int dying = 0;

#define PTHREAD_STACK_SIZE	(0x1fffff)
/*
 * Return the highest memory address associated with this addr.  This is just
 * a guess.  We assume that the main thread stack starts at 0xc0000000 and is
 * 8MB.  The other threads start at 0xbf800000 (immediately after the 8MB space
 * for the main thread stack) and are all 2MB.
 */
#define find_stack_start(addr)						    \
     /* Past stack area */						    \
    ((addr > (void*)0xc0000000) ? NULL :				    \
									    \
     /* Main thread stack */						    \
     (addr > (void*)0xbf800000) ? (void*)0xc0000000 :			    \
									    \
     /* Other thread stacks */						    \
	((void*)(((uint)addr & (~PTHREAD_STACK_SIZE)) + PTHREAD_STACK_SIZE))\
									    \
    )
int _libsafe_exclude=0;

//#include <libsafe_util.h>
/**
 *
 * @brief Copy a string
 *
 * @return pointer to destination buffer <d>
 */

char *strcpy(char *ZRESTRICT d, const char *ZRESTRICT s)
{
	char *dest = d;

	while (*s != '\0') {
		*d = *s;
		d++;
		s++;
	}

	*d = '\0';

	return dest;
}

/**
 *
 * @brief Copy part of a string
 *
 * @return pointer to destination buffer <d>
 */

char *strncpy(char *ZRESTRICT d, const char *ZRESTRICT s, size_t n)
{
	char *dest = d;

	while ((n > 0) && *s != '\0') {
		*d = *s;
		s++;
		d++;
		n--;
	}

	while (n > 0) {
		*d = '\0';
		d++;
		n--;
	}

	return dest;
}

/**
 *
 * @brief String scanning operation
 *
 * @return pointer to 1st instance of found byte, or NULL if not found
 */

char *strchr(const char *s, int c)
{
	char tmp = (char) c;

	while ((*s != tmp) && (*s != '\0')) {
		s++;
	}

	return (*s == tmp) ? (char *) s : NULL;
}

/**
 *
 * @brief String scanning operation
 *
 * @return pointer to last instance of found byte, or NULL if not found
 */

char *strrchr(const char *s, int c)
{
	char *match = NULL;

	do {
		if (*s == (char)c) {
			match = (char *)s;
		}
	} while (*s++);

	return match;
}

/**
 *
 * @brief Get string length
 *
 * @return number of bytes in string <s>
 */

size_t strlen(const char *s)
{
	size_t n = 0;

	while (*s != '\0') {
		s++;
		n++;
	}

	return n;
}

/**
 *
 * @brief Get fixed-size string length
 *
 * @return number of bytes in fixed-size string <s>
 */

size_t strnlen(const char *s, size_t maxlen)
{
	size_t n = 0;

	while (*s != '\0' && n < maxlen) {
		s++;
		n++;
	}

	return n;
}

/**
 *
 * @brief Compare two strings
 *
 * @return negative # if <s1> < <s2>, 0 if <s1> == <s2>, else positive #
 */

int strcmp(const char *s1, const char *s2)
{
	while ((*s1 == *s2) && (*s1 != '\0')) {
		s1++;
		s2++;
	}

	return *s1 - *s2;
}

/**
 *
 * @brief Compare part of two strings
 *
 * @return negative # if <s1> < <s2>, 0 if <s1> == <s2>, else positive #
 */

int strncmp(const char *s1, const char *s2, size_t n)
{
	while ((n > 0) && (*s1 == *s2) && (*s1 != '\0')) {
		s1++;
		s2++;
		n--;
	}

	return (n == 0) ? 0 : (*s1 - *s2);
}

/**
 * @brief Separate `str` by any char in `sep` and return NULL terminated
 * sections. Consecutive `sep` chars in `str` are treated as a single
 * separator.
 *
 * @return pointer to NULL terminated string or NULL on errors.
 */
char *strtok_r(char *str, const char *sep, char **state)
{
	char *start, *end;

	start = str ? str : *state;

	/* skip leading delimiters */
	while (*start && strchr(sep, *start)) {
		start++;
	}

	if (*start == '\0') {
		*state = start;
		return NULL;
	}

	/* look for token chars */
	end = start;
	while (*end && !strchr(sep, *end)) {
		end++;
	}

	if (*end != '\0') {
		*end = '\0';
		*state = end + 1;
	} else {
		*state = end;
	}

	return start;
}

char *strcat(char *ZRESTRICT dest, const char *ZRESTRICT src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

char *strncat(char *ZRESTRICT dest, const char *ZRESTRICT src,
	      size_t n)
{
	char *orig_dest = dest;
	size_t len = strlen(dest);

	dest += len;
	while ((n-- > 0) && (*src != '\0')) {
		*dest++ = *src++;
	}
	*dest = '\0';

	return orig_dest;
}

/**
 *
 * @brief Compare two memory areas
 *
 * @return negative # if <m1> < <m2>, 0 if <m1> == <m2>, else positive #
 */
int memcmp(const void *m1, const void *m2, size_t n)
{
	const char *c1 = m1;
	const char *c2 = m2;

	if (!n) {
		return 0;
	}

	while ((--n > 0) && (*c1 == *c2)) {
		c1++;
		c2++;
	}

	return *c1 - *c2;
}

/**
 *
 * @brief Copy bytes in memory with overlapping areas
 *
 * @return pointer to destination buffer <d>
 */

void *memmove(void *d, const void *s, size_t n)
{
	char *dest = d;
	const char *src  = s;

	if ((size_t) (dest - src) < n) {
		/*
		 * The <src> buffer overlaps with the start of the <dest> buffer.
		 * Copy backwards to prevent the premature corruption of <src>.
		 */

		while (n > 0) {
			n--;
			dest[n] = src[n];
		}
	} else {
		/* It is safe to perform a forward-copy */
		while (n > 0) {
			*dest = *src;
			dest++;
			src++;
			n--;
		}
	}

	return d;
}

/*
 * This is what is called when a buffer overflow on the stack is detected.  If
 * you want to add customized actions triggered by detection put them here.
 * 'name' is the name of this library, and (format,...) is similar to printf()
 * and passed to syslog().
 */
void _libsafe_die(char *format, ...)
{


    dying = 1;
    printf("_libsafe_die die\n");
}

/* Given an address 'addr' returns 0 iff the address does not point to a stack
 * variable.  Otherwise, it returns a positive number indicating the number of
 * bytes (distance) between the 'addr' and the frame pointer it resides in.
 * Note: stack grows down, and arrays/structures grow up.
 */
uint _libsafe_stackVariableP(void *addr) {
    /*
     * bufsize is the distance between addr and the end of the stack frame.
     * It's what _libsafe_stackVariableP() is trying to calculate.
     */
    uint bufsize = 0;

    /*
     * (Vandoorselaere Yoann)
     * We have now just one cast.
     */
    void *fp, *sp;

    /*
     * nextfp is used in the check for -fomit-frame-pointer code.
     */
    void *nextfp;

    /*
     * stack_start is the highest address in the memory space mapped for this
     * stack.
     */
    void *stack_start;

    /*
     * If _libsafe_die() has been called, then we don't need to do anymore
     * libsafe checking.
     */
    if (dying)
	return 0;

    /*
     * (Arash Baratloo / Yoann Vandoorselaere)
     * use the stack address of the first declared variable to get the 'sp'
     * address in a portable way.
     */
    sp = &fp;

    /*
     * Stack grows downwards (toward 0x00).  Thus, if the stack pointer is
     * above (>) 'addr', 'addr' can't be on the stack.
     */
    if (sp > addr)
	return 0;

    /*
     * Note: the program name is always stored at 0xbffffffb (documented in the
     * book Linux Kernel).  Search back through the frames to find the frame
     * containing 'addr'.
     */
    fp = __builtin_frame_address(0);

    /*
     * Note that find_stack_start(fp) should never return NULL, since fp is
     * always guaranteed to be on the stack.
     */
    stack_start = find_stack_start((void*)&fp);

    while ((sp < fp) && (fp <= stack_start)) {
	if (fp > addr) {
	    /*
	     * found the frame -- now check the rest of the stack
	     */
	    bufsize = fp - addr;
	    break;
	}

	nextfp = *(void **) fp;

	/*
	 * The following checks are meant to detect code that doesn't insert
	 * frame pointers onto the stack.  (i.e., code that is compiled with
	 * -fomit-frame-pointer).
	 */

	/*
	 * Make sure frame pointers are word aligned.
	 */
	if ((uint)nextfp & 0x03) {
	    printf("fp not word aligned; bypass enabled\n");
	    _libsafe_exclude = 1;
	    return 0;
	}

	/*
	 * Make sure frame pointers are monotonically increasing.
	 */
	if (nextfp <= fp) {
	    printf("fp not monotonically increasing; bypass enabled\n");
	    _libsafe_exclude = 1;
	    return 0;
	}

	fp = nextfp;
    }

    /*
     * If we haven't found the correct frame by now, it either means that addr
     * isn't on the stack or that the stack doesn't contain frame pointers.
     * Either way, we will return 0 to bypass checks for addr.
     */
    if (bufsize == 0) {
	return 0;
    }

    /*
     * Now check to make sure that the rest of the stack looks reasonable.
     */
    while ((sp < fp) && (fp <= stack_start)) {
	nextfp = *(void **) fp;

	if (nextfp == NULL) {
	    /*
	     * This is the only correct way to end the stack.
	     */
	    return bufsize;
	}

	/*
	 * Make sure frame pointers are word aligned.
	 */
	if ((uint)nextfp & 0x03) {
	    printf("fp not word aligned; bypass enabled\n");
	    _libsafe_exclude = 1;
	    return 0;
	}

	/*
	 * Make sure frame pointers are monotonically * increasing.
	 */
	if (nextfp <= fp) {
	    printf("fp not monotonically increasing; bypass enabled\n");
	    _libsafe_exclude = 1;
	    return 0;
	}

	fp = nextfp;
    }

    /*
     * We weren't able to say for sure that the stack contains valid frame
     * pointers, so we will return 0, which means that no check for addr will
     * be done.
     */
    return 0;
}

/**
 *
 * @brief Copy bytes in memory
 *
 * @return pointer to start of destination buffer
 */

void *real_memcpy(void *ZRESTRICT d, const void *ZRESTRICT s, size_t n)
{
	/* attempt word-sized copying only if buffers have identical alignment */
	printf("OKOK!!!!!!!!!!!");
	unsigned char *d_byte = (unsigned char *)d;
	const unsigned char *s_byte = (const unsigned char *)s;

#if !defined(CONFIG_MINIMAL_LIBC_OPTIMIZE_STRING_FOR_SIZE)
	const uintptr_t mask = sizeof(mem_word_t) - 1;

	if ((((uintptr_t)d ^ (uintptr_t)s_byte) & mask) == 0) {

		/* do byte-sized copying until word-aligned or finished */

		while (((uintptr_t)d_byte) & mask) {
			if (n == 0) {
				return d;
			}
			*(d_byte++) = *(s_byte++);
			n--;
		}

		/* do word-sized copying as long as possible */

		mem_word_t *d_word = (mem_word_t *)d_byte;
		const mem_word_t *s_word = (const mem_word_t *)s_byte;

		while (n >= sizeof(mem_word_t)) {
			*(d_word++) = *(s_word++);
			n -= sizeof(mem_word_t);
		}

		d_byte = (unsigned char *)d_word;
		s_byte = (unsigned char *)s_word;
	}
#endif

	/* do byte-sized copying until finished */

	while (n > 0) {
		*(d_byte++) = *(s_byte++);
		n--;
	}

	return d;
}

/**
 * libsafe memcpy function
 **/
void __attribute__((always_inline)) *memcpy(void *dest, const void *src, size_t n)
{
    printf("dest addr: %x\n", dest);
    printf("__builtin_frame_address; %x\n", __builtin_frame_address(0));
    if(dest+n > __builtin_frame_address(0)){
	    printf("fail\n");
    }
    /*size_t max_size;
    printf("NEW FONCTION");
    if ((max_size = _libsafe_stackVariableP(dest)) == 0) {
	printf("memcpy(<heap var> , <src>, %d)\n", n);
	return real_memcpy(dest, src, n);
    }

    printf("memcpy(<stack var> , <src>, %d) stack limit=%d)\n", n, max_size);
    if (n > max_size)
	_libsafe_die("Overflow caused by memcpy()\n");
    */
    //return real_memcpy(dest, src, n);
unsigned char *d_byte = (unsigned char *)dest;
        const unsigned char *s_byte = (const unsigned char *)src;
	while (n > 0) {
                *(d_byte++) = *(s_byte++);
                n--;
        }

}




/**
 *
 * @brief Set bytes in memory
 *
 * @return pointer to start of buffer
 */

void *memset(void *buf, int c, size_t n)
{
	/* do byte-sized initialization until word-aligned or finished */

	unsigned char *d_byte = (unsigned char *)buf;
	unsigned char c_byte = (unsigned char)c;

#if !defined(CONFIG_MINIMAL_LIBC_OPTIMIZE_STRING_FOR_SIZE)
	while (((uintptr_t)d_byte) & (sizeof(mem_word_t) - 1)) {
		if (n == 0) {
			return buf;
		}
		*(d_byte++) = c_byte;
		n--;
	}

	/* do word-sized initialization as long as possible */

	mem_word_t *d_word = (mem_word_t *)d_byte;
	mem_word_t c_word = (mem_word_t)c_byte;

	c_word |= c_word << 8;
	c_word |= c_word << 16;
#if Z_MEM_WORD_T_WIDTH > 32
	c_word |= c_word << 32;
#endif

	while (n >= sizeof(mem_word_t)) {
		*(d_word++) = c_word;
		n -= sizeof(mem_word_t);
	}

	/* do byte-sized initialization until finished */

	d_byte = (unsigned char *)d_word;
#endif

	while (n > 0) {
		*(d_byte++) = c_byte;
		n--;
	}

	return buf;
}

/**
 *
 * @brief Scan byte in memory
 *
 * @return pointer to start of found byte
 */

void *memchr(const void *s, int c, size_t n)
{
	if (n != 0) {
		const unsigned char *p = s;

		do {
			if (*p++ == (unsigned char)c) {
				return ((void *)(p - 1));
			}

		} while (--n != 0);
	}

	return NULL;
}

