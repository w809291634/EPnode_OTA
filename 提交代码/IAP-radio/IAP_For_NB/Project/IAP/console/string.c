/*
 * vivi/lib/string.c
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 06:22:25 $
 *
 * $Revision: 1.1.1.1 $
 * $Id: string.c,v 1.1.1.1 2004/02/04 06:22:25 laputa Exp $
 *
 *
 */

//#include "string.h" 
//#include "serial.h"
//#include "printk.h"
#include "stdio.h"
#include <string.h>

#define  MEMSET memset
#define  STRNCMP strncmp
#define  STRLEN  strlen
#define  STRCAT  strcat
#define  MEMCPY  memcpy


#ifdef DEBUG_STRING
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

static char hex_to_ascii_table[17] = "0123456789ABCDEF";

/*
 * Simple print string
 */
void putcc(unsigned char dat);
void putnstr(const char *str, size_t n)
{
	if (str == NULL)
		return;

	while (n && *str != '\0') {
		putcc(*str);
		str++;
		n--;
	}
}

void putstr(const char *str)
{
	putnstr(str, STRLEN(str));
}

void u32todecimal(char *buf, unsigned long x)
{
        int i = 0;
        int j = 0;
        char localbuf[16];

        if (x != 0) {
                while (x > 0) {
                        unsigned long rem = x % 10;
                        localbuf[i++] = hex_to_ascii_table[rem];
                        x /= 10;
                }
                /* now reverse the characters into buf */
                while (i > 0) {
                        i--;
                        buf[j++] = localbuf[i];
                }
                buf[j] = '\0';
        } else {
                buf[0] = '0';
                buf[1] = '\0';
        }
}

void binarytohex(char *buf, long x, int nbytes)
{
        int i;
        int s = 4*(2*nbytes - 1);
        if (hex_to_ascii_table[0] != '0')
                putstr("hex_to_ascii_table corrupted\r\n");
        for (i = 0; i < 2*nbytes; i++){
                buf[i] = hex_to_ascii_table[(x >> s) & 0xf];
                s -= 4;
        }
        buf[2*nbytes] = 0;
}


/*
 *
 */
 #if 1
unsigned  stringtoul(const char *str, int *ret)
{
        unsigned long num = 0;
        char c;
        unsigned char digit;
        int base = 10;
        int nchars = 0;
        int leadingZero = 0;

        *ret = 0;

        while ((c = *str) != '\0') {
                if (nchars == 0 && c == '0') {
                        leadingZero = 1;
                        DPRINTK("strtoul(): leadingZero nchar=%d", nchars);
                        goto step;
                } else if (leadingZero && nchars == 1) {
                        if (c == 'x') {
                                base = 16;
                                DPRINTK("strtoul(): base 16 nchars=%d", nchars);
                                goto step;
                        } else if (c == 'o') {
                                base = 8;
                                DPRINTK("strtoul(): base8 nchars=%d", nchars);
                                goto step;
                        }
                }
                DPRINTK("strtoul: c=%c", c);
                if (c >= '0' && c <= '9') {
                        digit = c - '0';
                } else if (c >= 'a' && c <= 'z') {
                        digit = c - 'a' + 10;
                } else if (c >= 'A' && c <= 'Z') {
                        digit = c - 'A' + 10;
                } else {
                        *ret = 3;
                        return 0;
                }
                if (digit >= base) {
                        *ret = 4;
                        return 0;
                }
                num *= base;
                num += digit;
step:
                str++;
                nchars++;

        }
        return num;
}

#endif


void putstr_hex(const char *str, unsigned long value)
{
	char buf[9]; 
	binarytohex(buf, value, 4);
	putstr(str);
	putstr(buf);
	putstr("\r\n");
}
#define BL_ISPRINT(ch)		(((ch) >= ' ') && ((ch) < 128))
void hex_dump(unsigned char *data, size_t num)
{
	int i;
	long oldNum;
	char buf[90];
	char *bufp;
	int line_resid;

	while (num) {
		bufp = buf;
		binarytohex(bufp, (unsigned long)data, 4);
		bufp += 8;
		*bufp++ = ':';
		*bufp++ = ' ';

		oldNum = num;

		for (i = 0; i < 16 && num; i++, num--) {
			binarytohex(bufp, (unsigned long)data[i], 1);
			bufp += 2;
			*bufp++ = (i == 7) ? '-' : ' ';
		}

		line_resid = (16 - i) * 3;
		if (line_resid) {
			MEMSET(bufp, ' ', line_resid);
			bufp += line_resid;
		}

		MEMCPY(bufp, "| ", 2);
		bufp += 2;

		for (i = 0; i < 16 && oldNum; i++, oldNum--)
			*bufp++ = BL_ISPRINT(data[i]) ? data[i] : '.';

		line_resid = 16 - i;
		if (line_resid) {
			MEMSET(bufp, ' ', 16 - i);
			bufp += 16 - i;
		}

		*bufp++ = '\r';
		*bufp++ = '\n';
		*bufp++ = '\0';
		putstr(buf);
		data += 16;
	}
}
