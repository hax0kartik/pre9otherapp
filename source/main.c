#include <string.h>
#include "gspgpu.h"
#include "text.h"
#include "memchunkhax.h"
#include "brahma.h"
#include "libctru/types.h"
#include "libctru/svc.h"
#include "libctru/srv.h"

const char* const aptServiceNames[] = {"APT:U", "APT:A", "APT:S"};

#define aptSessionInit() \
	int aptIndex; \
	for(aptIndex = 0; aptIndex < 3; aptIndex++)	if(!srvGetServiceHandle(&aptuHandle, (char*)aptServiceNames[aptIndex]))break;\
	svcCloseHandle(aptuHandle);\

#define aptOpenSession() \
	svcWaitSynchronization(aptLockHandle, U64_MAX);\
	srvGetServiceHandle(&aptuHandle, (char*)aptServiceNames[aptIndex]);\

#define aptCloseSession()\
	svcCloseHandle(aptuHandle);\
	svcReleaseMutex(aptLockHandle);\
	
#define HID_PAD (*(vu32*)0x1000001C)


Result APT_GetLockHandle(Handle* handle, u16 flags, Handle* lockHandle)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x10040; //request header code
	cmdbuf[1] = flags;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	if(lockHandle)*lockHandle=cmdbuf[5];
	
	return cmdbuf[1];
}

Result APT_CheckNew3ds(Handle *handle, bool *out)
{
	u32 *cmdbuf = getThreadCommandBuffer(); 
	cmdbuf[0]=0x1020000;

	Result ret = svcSendSyncRequest(*handle);
	if (ret == 0)
		*out = cmdbuf[2] & 0xFF;

	return ret;
}

const u8 hexTable[]=
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void hex2str(char* out, u32 val)
{
	int i;
	for(i=0;i<8;i++){out[7-i]=hexTable[val&0xf];val>>=4;}
	out[8]=0x00;
}

void renderString(char* str, int x, int y)
{
	u8 *ptr = GSP_GetScreenFBADR(0);
	if(ptr==NULL)return;
	drawString(ptr,str,x,y);
	GSP_FlushDCache((u32*)ptr, 240*400*3);
}

void centerString(char* str, int y)
{
	renderString(str, 200-(strlen(str)*4), y);
}

void drawHex(u32 val, int x, int y)
{
	char str[9];

	hex2str(str,val);
	renderString(str,x,y);
}

void doGspwn(u32* src, u32* dst, u32 size)
{
	Result (*gxcmd4)(u32 *src, u32 *dst, u32 size, u16 width0, u16 height0, u16 width1, u16 height1, u32 flags);
	u32 *paramblk = (u32*)*((u32*)0xFFFFFFC);
	gxcmd4=(void*)paramblk[0x1c>>2];
	gxcmd4(src, dst, size, 0, 0, 0, 0, 0x8);
}

void clearScreen(u8 shade)
{
	u8 *ptr = GSP_GetScreenFBADR(0);
	if(ptr==NULL)return;
	memset(ptr, shade, 240*400*3);
	GSP_FlushDCache((u32*)ptr, 240*400*3);
}

void errorScreen(char* str, u32* dv, u8 n)
{
	clearScreen(0x00);
	renderString("FATAL ERROR",0,0);
	renderString(str,0,10);
	if(dv && n)
	{
		int i;
		for(i=0;i<n;i++)drawHex(dv[i], 8, 50+i*10);
	}
	while(1);
}

void drawTitleScreen(char* str)
{
	clearScreen(0x00);
	centerString("OTHERAPP KARTIK",0);
	centerString("TRY BUILD",10);
	centerString("smealum.github.io/ninjhax2/",20);
	renderString(str, 0, 40);
}

bool g_is_new3ds;
char* test_result;

s32 set_test_result(void)
{
   __asm__ volatile("cpsid aif \n\t");
   test_result = "SUCCESS !!!";
   return 0;
}

extern u8 *g_ext_arm9_buf;
extern u64 g_ext_arm9_size;

int main(u32 loaderparam, char** argv)
{
	u32 *paramblk = (u32*)loaderparam;
	Handle* gspHandle=(Handle*)paramblk[0x58>>2];
	u32* linear_buffer = (u32*)((((u32)paramblk) + 0x1000) & ~0xfff);
	
	// put framebuffers in linear mem so they're writable
	u8* top_framebuffer = &linear_buffer[0x00100000/4];
	u8* low_framebuffer = &top_framebuffer[0x00046500];

	srvInit();
	
	GSP_SetBufferSwap(*gspHandle, 0, (GSPGPU_FramebufferInfo){0, (u32*)top_framebuffer, (u32*)top_framebuffer, 240 * 3, (1<<8)|(1<<6)|1, 0, 0});
	GSP_SetBufferSwap(*gspHandle, 1, (GSPGPU_FramebufferInfo){0, (u32*)low_framebuffer, (u32*)low_framebuffer, 240 * 3, 1, 0, 0});
	
	drawTitleScreen("");
	
	Handle aptuHandle;
	Handle aptLockHandle;
	bool isN3ds;
	aptSessionInit();
	
	aptOpenSession();
	APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle);
	aptCloseSession();
	
	aptOpenSession();
	APT_CheckNew3ds(&aptuHandle, &isN3ds);
	aptCloseSession();
	
	svcCloseHandle(aptLockHandle);
	
	drawTitleScreen("");
	drawHex(isN3ds, 8, 120);
	
	renderString("Trying memchunkhax", 8, 50);
	do_memchunkhax1();
	
	svc_7b((backdoor_fn)k_enable_all_svcs, isN3ds);
	renderString("Unblocked svcs    ", 8, 50);
	svcSleepThread(1e+9);
	
	test_result ="FAILED !!!";
	svcBackdoor(set_test_result);
	renderString(test_result, 8, 60);
	
	renderString("Unblocking access to all services", 8, 70);
	unlock_services(isN3ds);
	
	renderString("Initing brahma", 8, 80);
	Result ret = brahma_init();
	drawHex(ret, 8, 90);
	
	ret = load_arm9_payload_offset ("/arm9.bin", 0, 0);
	drawHex(ret, 8, 100);

	ret = firm_reboot(isN3ds);
	drawHex((u32)ret, 8, 110);
	
	svcSleepThread(100000000); //sleep long enough for memory to be written
	//drawTitleScreen("\n   The homemenu ropbin is ready.");
	while(1);
	svcCloseHandle(*gspHandle);


	svcExitProcess();

	while(1);
	return 0;
}
