#include "libctru/types.h"
#include "libctru/svc.h"
#include "libctru/fs.h"
#include "libctru/os.h"
#include "libctru/pm.h"
#include <string.h>

void *payload_buf;
size_t payload_size;
s32 backdoor_res;

#define FCRAM(x)   (void *)((kver < SYSTEM_VERSION(2, 44, 6)) ? (0xF0000000 + x) : (0xE0000000 + x)) //0x20000000
#define AXIWRAM(x) (void *)((kver < SYSTEM_VERSION(2, 44, 6)) ? (0xEFF00000 + x) : (0xDFF00000 + x)) //0x1FF00000
#define KMEMORY    ((u32 *)AXIWRAM(0xF4000))     

s32 patch_arm11_codeflow(void){
	__asm__ volatile ( "CPSID AIF");
	
	u32 kver = (*(vu32*)0x1FF80000) & ~0xFF;
	memcpy(FCRAM(0x3F00000), payload_buf, payload_size); //Huge payloads seem to crash when being copied (?)
	memcpy(FCRAM(0x3FFF000), payload_buf + 0xFF000, 0xE20);
	
	for (unsigned int i = 0; i < 0x2000/4; i++){
		if (KMEMORY[i] == 0xE12FFF14 && KMEMORY[i+2] == 0xE3A01000){ //hook arm11 launch
			KMEMORY[i+3] = 0xE51FF004; //LDR PC, [PC,#-4]
			KMEMORY[i+4] = 0x23FFF000;
			backdoor_res = 0;
			break;
		}
	}
	
	__asm__ volatile ( //flush & invalidate the caches
		"MOV R0, #0\n"
		"MCR P15, 0, R0, C7, C10, 0\n"
		"MCR P15, 0, R0, C7, C5, 0"
	);
	
	return backdoor_res;
}

Result FileRead(Handle *handle, void *buffer, char *filename, u32 maxsize, size_t *readsize)
{
	Handle fileHandle;
	
	FS_archive sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	Result ret = FSUSER_OpenFileDirectly(*handle, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, filename), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret > 0) return ret;
	ret = FSFILE_GetSize(fileHandle, (u64*)readsize);
	if(*readsize > maxsize) return -1;
	ret = FSFILE_Read(fileHandle, NULL, 0, buffer, (u64)*readsize);
	if(ret > 0) return ret;
	FSFILE_Close(fileHandle);
	return 0;
}

Result debugDump(Handle *handle, void *buffer, char *filename, size_t size)
{
	Handle fileHandle;
	
	FS_archive sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	Result ret = FSUSER_OpenFileDirectly(*handle, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, filename), FS_OPEN_WRITE, FS_ATTRIBUTE_NONE);
	if(ret > 0) return ret;
	ret = FSFILE_Write(fileHandle, NULL, 0, buffer, (u64)size, 0);
	if(ret > 0) return ret;
	FSFILE_Close(fileHandle);
	return 0;
}
void drawHex(u32 val, int x, int y);
Result safehax()
{	
	Handle fsHandle;
	
	Result ret = pmInit();
	if(ret > 0) return ret;
	drawHex(ret, 8, 90);	
	ret = fsInit(&fsHandle);
	if(ret > 0) return ret;
	drawHex(ret, 8, 100);
	
	//payload_buf = memalign(0x1000, 0x100000);
	u32 tmp_Addr;
	ret = svcControlMemory((u32*)&tmp_Addr, 0, 0, 0x100000, MEMOP_ALLOC_LINEAR, MEMPERM_READ|MEMPERM_WRITE);
	
	payload_buf = tmp_Addr;
	if(ret > 0 || !payload_buf) return ret;
	
	drawHex(ret, 8, 110);
	drawHex((u32)payload_buf, 100, 110);
	
	ret = FileRead(&fsHandle, payload_buf, "/arm9.bin", 0xFF000, &payload_size);
	if(ret > 0) return ret;
	
	drawHex(payload_size, 8, 120);
	size_t arm11_size;
	extern u8 arm11_bin[];
	extern u32 arm11_bin_size;
	
	memcpy(payload_buf + 0xFF000, &arm11_bin[0], arm11_bin_size);
	arm11_size = arm11_bin_size;
	if(ret > 0) return ret;
	
	*((u32 *)(payload_buf + 0xFFE00)) = (u32)GSP_GetScreenFBADR(0) + 0xC000000;
	*((u32 *)(payload_buf + 0xFFE04)) = (u32)GSP_GetScreenFBADR(1) + 0xC000000;
	*((u32 *)(payload_buf + 0xFFE08)) = (u32)GSP_GetScreenFBADR(2) + 0xC000000;
	
	*((bool *)(payload_buf + 0xFFE10)) = 1;
	
	debugDump(&fsHandle, payload_buf, "/__debug__.bin", 0x100000);
	svcBackdoor(patch_arm11_codeflow);
	
	if(backdoor_res != 0) return 0xFA11;
	
	ret = PM_LaunchFIRMSetParams(2, 0, NULL);
	
	svcSleepThread(1e+9);
	return ret;
}