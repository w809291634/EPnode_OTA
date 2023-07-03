/*
 * vivi/lib/command.c: 
 *   - to support user commands on the boot loader
 *
 * Copyright (C) 2001 MIZI Research, Inc.
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
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2004/02/04 06:22:25 $
 *
 * $Revision: 1.1.1.1 $
 *
 * 
 * History
 * 
 * 2001-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *    - Base on bootldr/bootldr.c 
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    -  Add flash commands
 *
 * 2002-07-13: Janghoon Lyu <nandy@mizi.com>
 *
 */
#include "command.h"
#include <stdio.h>
#include "printk.h"
#include "stm32f4xx.h"
#include "flash_if.h"
//#define USER_CONFIG_AREA   (uint32_t)0x08003000
#define MAX_PROMPT_LEN	12
char prompt[MAX_PROMPT_LEN] = "V1.2";

enum ParseState {
	PS_WHITESPACE,
	PS_TOKEN,
	PS_STRING,
	PS_ESCAPE
};

enum ParseState stackedState;

static user_command_t *head_cmd = NULL;
static user_command_t *tail_cmd = NULL;
/**************************��ֲ����**************************/
__align(4)
unsigned char userdata[sizeof(user_data_t)];
/***********************************************************/
/*
 * Parse user command line
 */
 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: �û��ַ���
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void parseargs(char *argstr, int *argc_p, char **argv, char** resid)
{
	int argc = 0;
	char c;
	enum ParseState lastState = PS_WHITESPACE;

	/* tokenize the argstr */
	while ((c = *argstr) != 0) {
		enum ParseState newState;

		if (c == ';' && lastState != PS_STRING && lastState != PS_ESCAPE)
			break;

		if (lastState == PS_ESCAPE) {
			newState = stackedState;
		} else if (lastState == PS_STRING) {
			if (c == '"') {
				newState = PS_WHITESPACE;
				*argstr = 0;
			} else {
				newState = PS_STRING;
			}
		} else if ((c == ' ') || (c == '\t')) {
			/* whitespace character */
			*argstr = 0;
			newState = PS_WHITESPACE;
		} else if (c == '"') {
			newState = PS_STRING;
			*argstr++ = 0;
			argv[argc++] = argstr;
		} else if (c == '\\') {
			stackedState = lastState;
			newState = PS_ESCAPE;
		} else {
			/* token */
			if (lastState == PS_WHITESPACE) {
				argv[argc++] = argstr;
			}
			newState = PS_TOKEN;
		}

		lastState = newState;
		argstr++;
	}

#if 0 /* for debugging */
	{
		int i;
		//putLabeledWord("parseargs: argc=", argc);
		for (i = 0; i < argc; i++) {
			putstr("   ");
			putstr(argv[i]);
			putstr("\r\n");
		}
	}
#endif
	
	argv[argc] = NULL;
	if (argc_p != NULL)
		*argc_p = argc;

	if (*argstr == ';') {
		*argstr++ = '\0';
	}
	*resid = argstr;
	//printk("argc is %d\n",argc);
}

 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void unparseargs(char *argstr, int argc, const char **argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		if (argv[i] != NULL) {
			STRCAT(argstr, " ");
			STRCAT(argstr, argv[i]);
		}
	}
}

/*
 * Genernal interface
 */

/*
 * For (main) commands
 */

/* add user command */
 /*********************************************************************************************************
** ��������: add_command
** ��������:ע���û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void add_command(user_command_t *cmd)
{
	if (head_cmd == NULL) {
		head_cmd = tail_cmd = cmd;
	} else {
		tail_cmd->next_cmd = cmd;
		tail_cmd = cmd;
	}
	/*printk("Registered '%s' command\n", cmd->name);*/
}

/* find command */
 /*********************************************************************************************************
** ��������: find_cmd
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

user_command_t *find_cmd(const char *cmdname)
{
	user_command_t *curr;

	/* do da string compare for the first offset character of cmdstr
	  against each number of the cmdlist */
	curr = head_cmd;
	while(curr != NULL) {
		if (STRNCMP(curr->name, cmdname, STRLEN(cmdname)) == 0)
			return curr;
		curr = curr->next_cmd;
	}
	return NULL;
}

/* execute a function */
 /*********************************************************************************************************
** ��������: execcmd
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void execcmd(int argc, const char **argv)
{
	user_command_t *cmd = find_cmd(argv[0]);

	if (cmd == NULL) {
		printk("\tCannot find command '%s'\n", argv[0]);
		printk("\tTo inquiry available command, please input 'help'\n"); 
		return;
	}
	/*printk("execcmd: cmd=%s, argc=%d\n", argv[0], argc);*/

	cmd->cmdfunc(argc, argv);
}

/* parse and execute a string */
 /*********************************************************************************************************
** ��������: exec_string
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void exec_string(char *buf)
{
	int argc;
	char *argv[128];
	char *resid;

	while (*buf) {
		MEMSET(argv, 0, sizeof(argv));
		parseargs(buf, &argc, argv, &resid); //�����û��������
		if (argc > 0)  
			execcmd(argc, (const char **)argv);
		buf = resid;
	}
}

/*
 * For sub-commands
 */
  /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void execsubcmd(user_subcommand_t *cmds, int argc, const char **argv)
{

	while (cmds->name != NULL) {
		if (STRNCMP(argv[0], cmds->name, STRLEN(argv[0])) == 0) {
			/*printk("subexeccmd: cmd=%s, argc=%d\n", argv[0], argc);*/
			cmds->cmdfunc(argc, argv);
			return;
		}
		cmds++;
	}
	printk("Could not found '%s' sub-command\n", argv[0]);
}
 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void print_usage(char *strhead, user_command_t *cmds)
{
	printk("Usage:\n");
	if (cmds->name != NULL) {
		if (strhead)
			printk("%s ", strhead);
		if (*cmds->helpstr)
			printk("%s\n", cmds->helpstr);
	}
}

 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void invalid_cmd(const char *cmd_name)
{
//    user_command_t *curr;
//    curr=find_cmd(cmd_name);
	printk("ERROR! Invalid CMD!\n");
}


/*
 * Define (basic) built-in commands
 */

/* help command */
 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void command_help(int argc, const char **argv)
{
	user_command_t *curr;

	/* help <command>. invoke <command> with 'help' as an argument */
	if (argc == 2) {
		if (STRNCMP(argv[1], "help", STRLEN(argv[1])) == 0) {
			printk("Are you kidding?\n");
			return;
		}
		argv[0] = argv[1];
		argv[1] = "help";
		execcmd(argc, argv);
		return;
	}

	printk("Usage:\n");
	curr = head_cmd;
	while(curr != NULL) {
		printk("   %s\n", curr->helpstr);
		curr = curr->next_cmd;
	}
}

user_command_t help_cmd = {
	"help",
	command_help,
	NULL,
	"\thelp  \t\t\t\t--command help"
};
 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#define SAVE
#ifdef SAVE

#include	"flash.h"
/* save command */
void command_save(int argc,const char *argv[]){
  flash_write(FLASH_PARAM_START_ADDR, (unsigned char *)(userdata), sizeof(user_data_t));
  printk("\tParameter Saved!\n");

}
user_command_t save_cmd = {
	"save",
	command_save,
	NULL,
	"\tsave  \t\t\t\t--Save system parameter"
};
#endif

 /*********************************************************************************************************
** ��������: command_reset
** ��������: ϵͳ��λ��
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
__asm static void SystemReset(void) 
{ 
 MOV R0, #1           //;  
 MSR FAULTMASK, R0    //; ���FAULTMASK ��ֹһ���ж� 
 LDR R0, =0xE000ED0C  //; 
 LDR R1, =0x05FA0004  //; 
 STR R1, [R0]         //; ϵͳ�����λ  

deadloop 
    B deadloop        //; �ȴ���λ�ɹ�
} 
/* reset command */
void command_reset(int argc,const char *argv[]){
 printk("System reset.......\n");
 SystemReset();      
}
user_command_t reset_cmd = {
	"reset",
	command_reset,
	NULL,
	"\treset 				--System reset"
};

 
#define IAP
#ifdef IAP
//#include "hexparse.h"
//#include "sst25vf016bdrv.h"
#include "ymodem.h"
#include "saveData_manage.h"
#include "w25qxx.h"
__align(4) unsigned char ybuf[2048];

void command_iap(int argc,const char **argv){
	int i;
	user_data_t *p = (user_data_t*)userdata;
	if((argc != 2)&&(argc !=3)) {
		invalid_cmd(argv[0]);
  }	else{
		if(STRNCMP("update",argv[1],STRLEN("update"))==0){
			p->boot_mode = BOOT_IAP;
      p->app_en = APP_ERR;
			command_save(1,NULL);
			/* Flash unlock */
			FLASH_Unlock();
			printk("Programming Start...\r\n");
			if((i=Ymodem_Receive(ybuf))>0) {
				p->app_en = APP_OK;
        if(p->rec_sta != REC_DONE) { //�ָ��������ݣ�����ǰ���򿽱����ָ���
          //��ʼ��SPIFlash
          W25QXX_Init();
          rec_Code_Update();
          p->rec_sta = REC_DONE;
          printk("Recovery Code Updated!\n");
        }
				printk("OK! Programmed %d bytes\n",i);
				FLASH_Lock();
				command_save(1,NULL);
        //�����ڵ�
        NVIC_SystemReset();
        while(1);
			} else {
				printk("ERR%d: Programmed %d bytes\n",i,i);
				FLASH_Lock();
				SystemReset();
			}
		} else if(STRNCMP("rec",argv[1],STRLEN("rec"))==0){
      //���»ָ������룺����ǰflash���븴�Ƶ�SPIFlash��
      p->rec_sta = REC_NULL;
      command_save(1,NULL);
      //��ʼ��SPIFlash
      W25QXX_Init();
      rec_Code_Update();
      p->rec_sta = REC_DONE;
      printk("Recovery Code Updated!\n");
      command_save(1,NULL);
      //�����ڵ�
      NVIC_SystemReset();
      while(1);
    } else if(STRNCMP("exit",argv[1],STRLEN("exit"))==0){
			printk("\tExit IAP, enter app program...\n");
      RunUserProgram(APPLICATION_ADDRESS);
		} else {
			printk("\tCMD Error!\n");
			invalid_cmd(argv[0]);
		}
	}
}

user_command_t  iap_cmd = {
	 "IAP",
	 command_iap,
	 NULL,
	 "\tIAP <command> \t\t\t--APP Update"
 }; 
#endif

 
#define VERSION
#ifdef VERSION
extern char verst[];
extern char versd[];
char verTime[10];
 
char* getVersionTime(void) {
	int i = 0, j = 0;
	memset(verTime, 0, 10);
	while(verst[i] != '\0') {
		if(verst[i] != ':')
			verTime[j++] = verst[i];
		i++;
	}
	return verTime;
}

void command_version(int argc,const char **argv){
		printk("\n*******************X-Boot***********************\n");
    printk("Firm Version: v1.4%s               \n",getVersionTime());
    printk("Author: Cag                                         \n");
    printk("Version Date: ");
		printk("  %s\n",versd);
    printk("************************************************\n");
}

user_command_t  version_cmd = {
	 "Version",
	 command_version,
	 NULL,
	 "\tVersion \t\t\t--Programme Version Infomation"
 }; 
#endif

 /*********************************************************************************************************
** ��������: parseargs
** ��������: �����û����
** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: 
**
** ������: 
** �ա���: 
**-------------------------------------------------------------------------------------------------------
** �޸���: 
** �ա���: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

/* Register basic user commands */
int init_builtin_cmds(void)
{
#ifdef IAP
	add_command(&iap_cmd);
#endif
#ifdef VERSION
	add_command(&version_cmd);
#endif
#ifdef SAVE
	add_command(&save_cmd);
#endif
	add_command(&reset_cmd );
	add_command(&help_cmd);
	return 0;
}
