/*
 * vivi/include/command.h
 *
 */
#ifndef _VIVI_COMMAND_H_
#define _VIVI_COMMAND_H_
#include <string.h>
#include "printk.h"

#define  MEMSET memset
#define  STRNCMP strncmp
#define  STRLEN  strlen
#define  STRCAT  strcat

#define BOOT_IAP    0xA2
#define BOOT_REC    0x2B
#define BOOT_APP    0x3C

#define APP_OK      0x4D
#define APP_ERR     0xFF

#define REC_DONE    0x2E
#define REC_NULL    0xFF

typedef struct user_data{
  unsigned char boot_mode;
  unsigned char app_en;
  unsigned char rec_sta;
  unsigned char reserved;
}user_data_t;
//struct user_command_t;

typedef struct user_command {
	const char *name;
	void (*cmdfunc)(int argc, const char **);
	struct user_command *next_cmd;
	const char *helpstr;
} user_command_t;

typedef struct user_subcommand {
	const char *name;
	void (*cmdfunc)(int argc, const char **);
	const char *helpstr;
} user_subcommand_t;
/* General interfaces */
extern void add_command(user_command_t *cmd);
void execcmd(int, const char **);
void exec_string(char *);
void execsubcmd(user_subcommand_t *, int, const char **);
void print_usage(char *strhead, user_command_t *);
void invalid_cmd(const char *cmd_name);
int init_builtin_cmds(void);

#endif /* _VIVI_COMMAND_H_ */


