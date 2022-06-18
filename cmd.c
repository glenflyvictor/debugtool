#include "def.h"

extern VIDEO_PCI_PROP video_pci_prop;


DBG_CMD_STRUCT dbg_cmd_table[] = {
	{"?", do_dbg_help},
	{"q", do_dbg_quit},
	{"help", do_dbg_help},
	{"mmio", do_dbg_mmio},
	{"ts", do_dbg_ts},
    {"vcore", do_dbg_voltage},
	{"sf", do_dbg_flash},
	{"prog", do_dbg_program_fw},
	{"dump", do_dbg_dump_fw},	
	{"print", do_dbg_print_info},
	{"clk", do_dbg_clk},  
	{"memtest", do_dbg_mem_test},
	{"i2c", do_dbg_i2c_test},
	{"dp", do_dbg_dp_test},		        
};

#define DBG_CMD_TABLE_SIZE (sizeof(dbg_cmd_table) / sizeof(DBG_CMD_STRUCT))


void CToolParserCmd(void)
{
	char*  line;

	line = (char*)malloc(STRINGBUFFERSIZE);
	memset(line, 0, STRINGBUFFERSIZE);

	while(1)
	{
		EchoWait();
		CToolGetInput(line, STRINGBUFFERSIZE* sizeof(char));
		if(ProcessString(line)==0)
		{	
			break;
		}			
	}

}

int ProcessString(char* buffer) // return 0 if quit, else return 1
{
	int    bStatus;
	int    i;
	char     delim[] = " \t\n,";
	char*    cmd[MAX_PARAM_COUNT];
	unsigned char    param_count;
	DBG_CMD_STRUCT  *p_cmd_struct;

	for (i=0; i<MAX_PARAM_COUNT; i++)
		cmd [i]=NULL;
	
	cmd[0]=strtok(buffer, delim);
	if (cmd[0]==NULL)
		return 1;
	for (i=1; i<(MAX_PARAM_COUNT); i++)
	{
		cmd[i]=strtok(NULL, delim);
		if (cmd[0]==NULL)
			break;
	}

	param_count = 0;
	while (cmd[param_count]!=NULL)
	{
		param_count++;
	}

/*Just for debug
    printf("\n");
    i = 0;
    while (cmd[i]!=NULL)
    {
        printf("%s ", cmd[i]);
        i ++;
    }
    printf("\n");
*/
	for (i=0,p_cmd_struct=dbg_cmd_table; i<DBG_CMD_TABLE_SIZE; i++, p_cmd_struct++)
	{
		if (strcmp(cmd[0], p_cmd_struct->name)==0)
		{
			bStatus=p_cmd_struct->execute(cmd, param_count);
			break;
		}
	}	

return bStatus;
}

int do_dbg_mmio(char * cmd[], unsigned int param_count)
{
    unsigned int 	reg_index;
	unsigned int 	reg_data;

	if(param_count == 2)	//two parameters. register read.
	{
		reg_index = StoH(cmd[1]);
        reg_data = readl(video_pci_prop.mapped_mmioBase+reg_index);
		printf("\nRead MMIO %x = 0x%08x\n", reg_index, reg_data);
	}
	else if(param_count == 3)	//register write
	{
		reg_index = StoH(cmd[1]);
		reg_data = StoH(cmd[2]);
        writel(reg_data, video_pci_prop.mapped_mmioBase+reg_index);
		printf("\nWrite MMIO %x = 0x%08x Done.\n", reg_index, reg_data);	
	}
	else
	{
		printf("\nParse Error!\n");
	}
	
	return TRUE;
}

int do_dbg_ts(char * cmd[], unsigned int param_count)
{
    printf("Temp : %d C\n",dout_to_temp(GetTemperature())/1000);
	return TRUE;
}

int do_dbg_voltage(char * cmd[], unsigned int param_count)
{
    printf("voltage : %d mV\n",( GetVoltage()*1000+1157200)/1869);
	return TRUE;
}

int do_dbg_print_info(char * cmd[], unsigned int param_count)
{
    sf_init(video_pci_prop.mapped_mmioBase);
    read_fw_version();
    read_PCIe(video_pci_prop.mapped_mmioBase);
    printf("DDR Version : DDR4\n");
    read_bitwidth(video_pci_prop.mapped_mmioBase);                        
    printf("Memory Clk : %d MHz\n",get_mpll());
    printf("Elite Clk : %d MHz\n",get_vepll(EPLL_REG));   
    printf("Temp : %d C\n",dout_to_temp(GetTemperature())/1000);
    printf("voltage : %d mV\n",( GetVoltage()*1000+1157200)/1869);     
	return true;
}

int do_dbg_flash(char * cmd[], unsigned int param_count)
{
	sf_init(video_pci_prop.mapped_mmioBase);
    sf_flash_test(param_count,cmd);
	return TRUE;
}

int do_dbg_mem_test(char * cmd[], unsigned int param_count)
{
    chip_mem_connection_test(param_count,cmd);
	return TRUE;
}

int do_dbg_i2c_test(char * cmd[], unsigned int param_count)
{
	i2c_prog(param_count,cmd);
	return TRUE;
}

int do_dbg_dp_test(char * cmd[], unsigned int param_count)
{
    chip_mem_connection_test(param_count,cmd);
	return TRUE;
}

int do_dbg_program_fw(char * cmd[], unsigned int param_count)
{
	unsigned int dump_file_size = MAX_VIDEO_ROM_SIZE;
    sf_init(video_pci_prop.mapped_mmioBase);
	if(param_count>=2)
    {
		flash_vbios(cmd[1]);
	}
	else
	{
		printf("  prog filename: program spi flash.\n");
	}
	return TRUE;
}

int do_dbg_dump_fw(char * cmd[], unsigned int param_count)
{
	unsigned int dump_file_size = MAX_VIDEO_ROM_SIZE;
    sf_init(video_pci_prop.mapped_mmioBase);
	if(param_count>=2)
    {
		if(param_count==3)
		{
			dump_file_size = atoi(cmd[2]);
		}
		else
		{
			//read file size from vbios
		}

		printf("dump file size =%d\n", dump_file_size);
		dump_vbios(cmd[1],dump_file_size);
	}
	else
	{
    	printf("  dump filename [filesize(byte)]: dump spi flash file.\n");
	}
	return TRUE;
}

int do_dbg_clk(char * cmd[], unsigned int param_count)
{
    clk_prog(param_count,cmd);
	return TRUE;
}

int do_dbg_help(char * cmd[], unsigned int param_count)
{
	helpinfo();
	return TRUE;
}

int do_dbg_quit(char * cmd[], unsigned int param_count)
{
    return FALSE;
}

void helpinfo(void)
{
    printf("  help/?     --- print help info\n");
    printf("  q          --- quit\n");
	printf("  ts         --- print temperature\n");
	printf("  vcore      --- print vcore\n");    
    printf("  clk        --- read/write clock\n");
    printf("  memtest    --- miu test connection\n");  
    printf("  prog       --- program rom to flash\n");
    printf("  dump       --- dump data from flash saved to file\n");	
    printf("  mmio       --- read/write register\n");  
    printf("  print      --- print all information(pcie info/mem info/vcore/clk/temp)\n");
}


/*
unsigned char GetKey(void)
{
	unsigned char key;    

	key = getchar();
	
	if (key == 0)
		key = (0x80 | getchar());

	return key;
}

void TestCmdLine(void)
{
	char    c = 0;
	char*   buffer;
	char*   oldbuf;
	char*   ptr;
	int  oldbuf_valid = FALSE;
	int  n;

	buffer = (char*)malloc(sizeof(char) * STRINGBUFFERSIZE);
	oldbuf = (char*)malloc(sizeof(char) * STRINGBUFFERSIZE);

	while (1) 
	{
		printf("\n>");
		fflush (stdout);
		ptr = buffer; // reset the buffer pointer
		n=0;

		// Get the command string
		while (1) 
		{
			c = GetKey();
	
			if ((c == 0x8) && (n==0))	// ignore the first BackSpace
				continue;
			else if (c == 0x8)	// not the first BackSpace
			{
				ptr--;
				n--;
				putchar(0x8);
				putchar(' ');
				putchar(0x8);
			}
			else if ((c== 0xd) || (c==0xa))	//  
			{
				*ptr = '\0';	// add a '\0' to the end of the string
				//save the current buffer to old buffer
				strcpy(oldbuf, buffer);
				oldbuf_valid=TRUE;
				break;
			}
			else if ((c < 0x20) || (c > 0x7f)) 	// not a character
			{
				if ((c==F3KEY)&&oldbuf_valid)	// "F3", find the previous command
				{
					while (n--) 
					{
						putchar(0x8);
						putchar(' ');
						putchar(0x8);
					}
					n=printf("%s",oldbuf);
					fflush (stdout);
					//copy from old buffer to current buffer
					strcpy(buffer,oldbuf);
					ptr = (unsigned char *) &buffer[n];
				}
				continue;
			}
			else 
			{
				if (ptr <= &buffer[198]) 
				{
					*ptr++ = c;
					putchar(c);
					n++;
				}
			}
		}
		if(ProcessString(buffer)==0)
		{	
			break;
		}

	}

	free(oldbuf);
	
}
*/