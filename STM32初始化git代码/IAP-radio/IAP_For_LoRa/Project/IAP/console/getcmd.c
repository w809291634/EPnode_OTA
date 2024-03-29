/*
 * vivi/drivers/serial/getcmd.c
 *
 * Based on bootldr/bootldr.c
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 06:22:25 $
 *
 * $Revision: 1.1.1.1 $
 */

#include "getcmd.h"

char getcc(void);
void putcc(unsigned char dat);

void getcmd(char *cmd_buf, unsigned int len)
{
	char curpos = 0;	/* current position - index into cmd_buf */
	char c;
	int cmd_echo = 1;

	/* Clear out the buffer */
	MEMSET(cmd_buf, 0, MAX_CMDBUF_SIZE);

	for (;;) {
		c = getcc();
		switch (c) {
		case 0x08:
		case 0x06:
		case 0x07:
		case 0x7E:
		case 0x7F:	/* backspace or delete */
			/* we're not at the beginning of the line */
			if (curpos) {
				curpos--;
				putcc(0x08);	/* go backwards */
				putcc(' ');	/* overwrite the char */
				putcc(0x08);	/* go back again */
			}
			cmd_buf[curpos] = '\0';
			break;
		case '\r':
		case '\n':
		case '\0':
			putcc('\r');
			putcc('\n');
			curpos=0;
			goto end_cmd;
		case CTL_CH('x'):
			curpos = 0;
			break;

		default:
			if (curpos < MAX_CMDBUF_SIZE) {
				cmd_buf[curpos] = c;
				/* echo it back out to the screen */
				if (cmd_echo)
					putcc(c);
				curpos++;
			}
			break;
		}
	}
end_cmd:
	DPRINTK("COMMAND: %s\n", cmd_buf);
}
