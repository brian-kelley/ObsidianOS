#include "atadrv.h"

int ataInit()
{
    //todo: reset drive
    //select master drive
    writeport(0x1F6, 0xA0);
    return 0;	//TODO: Return 1 if something goes wrong
}

//ATA PIO driver functions (reset, read sector, write sector)
int readsector(dword sector, byte* buf) //buf must have 512 bytes allocated
{
    //wait for BSY to clear and RDY to set
    byte status;
    int timeoutCounter = 10000;
    do
    {
	status = readport(0x1F7);
	timeoutCounter--;
	if(timeoutCounter == 0)
	{
	    ataInit();	//reset drive
	    timeoutCounter = 10000; //try again
	}
    }
    while(!(status & (1 << 6)) || (status & (1 << 7)));
    writeport(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
    writeport(0x1F1, 0);
    writeport(0x1F2, 1);    //only read 1 sector
    writeport(0x1F3, sector & 0xFF);
    writeport(0x1F4, (sector >> 8) & 0xFF);
    writeport(0x1F5, (sector >> 16) & 0xFF);
    writeport(0x1F7, 0x20); //read command
    //Wait for RDY and DRQ to set, BSY to clear
    do
    {
	status = readport(0x1F7);
    }
    while(!(status & (1 << 3)) || !(status & (1 << 6)) || status & (1 << 7));
    for(int i = 0; i < 256; i++)
    {
	word data = readportw(0x1F0);
	buf[i * 2] = data & 0xFF;
	buf[i * 2 + 1] = (data & 0xFF00) >> 8;
    }
    return 0;
}

int writesector(dword sector, byte* buf)
{
    byte status;
    int timeoutCounter = 10000;
    do
    {
	status = readport(0x1F7);
	timeoutCounter--;
	if(timeoutCounter == 0)
	{
	    ataInit();	//reset drive
	    timeoutCounter = 10000; //try again
	}
    }
    while(!(status & (1 << 6)) || (status & (1 << 7)));   
    writeport(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
    writeport(0x1F1, 0);
    writeport(0x1F2, 1);    //only read 1 sector
    writeport(0x1F3, sector & 0xFF);
    writeport(0x1F4, (sector >> 8) & 0xFF);
    writeport(0x1F5, (sector >> 16) & 0xFF);
    writeport(0x1F7, 0x30); //write command
    do
    {
	status = readport(0x1F7);
    }
    while(!(status & (1 << 3)) || !(status & (1 << 6)) || status & (1 << 7));
    for(int i = 0; i < 256; i++)
    {
	word data = buf[i * 2] | ((word) buf[i * 2 + 1]) << 8;
	writeportw(0x1F0, data);
    }
    writeport(0x1F7, 0xE7); //flush write buffer in drive
    return 0;
}
