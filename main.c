#include "def.h"


VIDEO_PCI_PROP video_pci_prop;
configuration config;

static int handler(void* user, const char* section, const char* name, const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("id", "vendorid")) 
    {
        pconfig->vendorid = htoi(value);
    }
    else if(MATCH("id", "deviceid"))
    {
        pconfig->deviceid = htoi(value);
    }
    else if(MATCH("addr", "mmiobase"))
    {
        pconfig->mmiobase = htoi(value);
    }  
     else if(MATCH("i2c", "crt"))
    {
        pconfig->serialport0 = htoi(value);
    } 
    else if(MATCH("i2c", "hdmi0"))
    {
        pconfig->serialport1 = htoi(value);
    } 
    else if(MATCH("i2c", "hdmi1"))
    {
        pconfig->serialport2 = htoi(value);
    }     
    else
    {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char* config_file_name = "glenfly_tool_debug.ini";

    config.vendorid = 0x6766;
    config.deviceid = 0x3d02;

    if(argc == 2)
    {
        config_file_name = argv[1];
    }

    if(ini_parse(config_file_name, handler, &config)<0) 
    {
        printf("Can't load %s\n",config_file_name);
        printf("Load chip id default : vendorid = 0x%x, deviceid = 0x%x\n", config.vendorid, config.deviceid);
    }
    else
    {
        printf("Load chip id from file : vendorid = 0x%x, deviceid = 0x%x\n", config.vendorid, config.deviceid);
    }

    if(!find_Base_Addr(config))
    {
        //load mmio base from config.ini
        printf("Can't read mmio base address from pcie.\n");
        printf("Load base address from file: mmiobase = 0x%lx\n", config.mmiobase);
        video_pci_prop.MmioBase = config.mmiobase;
    }

    if(map_to_system_memory(video_pci_prop.MmioBase))
    {
        CToolParserCmd();
    }

	return 1;
}

			
