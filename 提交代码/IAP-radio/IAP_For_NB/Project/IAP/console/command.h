/*
 * vivi/include/command.h
 *
 */
#ifndef _VIVI_COMMAND_H_
#define _VIVI_COMMAND_H_
#include <string.h>
#include <stdint.h>

#define  MEMSET memset
#define  STRNCMP strncmp
#define  STRLEN  strlen
#define  STRCAT  strcat

#define BOOT_IAP    0xA2
#define BOOT_APP    0x3C

#define APP_OK      0x4D
#define APP_ERR     0xFF

#define HOP_TAB_SIZE	10
typedef struct user_data {
  uint16_t boot_mode;
  uint16_t app_en;
  uint32_t flag;
  char id[40]; //ID
  char key[120]; //KEY
  char ip[64]; //
  unsigned char nbandFlag;
  int port;
  int mode;
}user_data_t;

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


