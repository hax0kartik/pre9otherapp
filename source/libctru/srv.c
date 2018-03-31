/*
  srv.c _ Service manager.
*/

#include <string.h>
#include "types.h"
#include "result.h"
#include "srv.h"
#include "svc.h"
#include "ipc.h"

Handle srvHandle;

Result srvInit(void)
{
	Result rc = 0;

	rc = svcConnectToPort(&srvHandle, "srv:");
	if (R_FAILED(rc)) goto end;

	rc = srvRegisterClient();
end:
	if (R_FAILED(rc)) srvExit();
	return rc;
}

void srvExit(void)
{

	if (srvHandle != 0) svcCloseHandle(srvHandle);
	srvHandle = 0;
}

Handle *srvGetSessionHandle(void)
{
	return &srvHandle;
}

Result srvGetServiceHandle(Handle* out, const char* name)
{
	return srvGetServiceHandleDirect(out, name);
}

Result srvRegisterClient(void)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,2); // 0x10002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvEnableNotification(Handle* semaphoreOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0);

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(semaphoreOut) *semaphoreOut = cmdbuf[3];

	return cmdbuf[1];
}

Result srvRegisterService(Handle* out, const char* name, int maxSessions)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,4,0); // 0x30100
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = maxSessions;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result srvUnregisterService(const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,3,0); // 0x400C0
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvGetServiceHandleDirect(Handle* out, const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,4,0); // 0x50100
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = 0x0; // per-thread setting, default is blocking

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result srvRegisterPort(const char* name, Handle clientHandle)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,3,2); // 0x600C2
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = IPC_Desc_SharedHandles(1);
	cmdbuf[5] = clientHandle;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvUnregisterPort(const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,3,0); // 0x700C0
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvGetPort(Handle* out, const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,4,0); // 0x80100
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = 0x0;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result srvWaitForPortRegistered(const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,4,0); // 0x80100
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = 0x1;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;
	return cmdbuf[1];
}

Result srvSubscribe(u32 notificationId)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1] = notificationId;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvUnsubscribe(u32 notificationId)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,1,0); // 0xA0040
	cmdbuf[1] = notificationId;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvReceiveNotification(u32* notificationIdOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(notificationIdOut) *notificationIdOut = cmdbuf[2];

	return cmdbuf[1];
}

Result srvPublishToSubscriber(u32 notificationId, u32 flags)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,2,0); // 0xC0080
	cmdbuf[1] = notificationId;
	cmdbuf[2] = flags;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvPublishAndGetSubscriber(u32* processIdCountOut, u32* processIdsOut, u32 notificationId)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1] = notificationId;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(processIdCountOut) *processIdCountOut = cmdbuf[2];
	if(processIdsOut) memcpy(processIdsOut, &cmdbuf[3], cmdbuf[2] * sizeof(u32));

	return cmdbuf[1];
}

Result srvIsServiceRegistered(bool* registeredOut, const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,3,0); // 0xE00C0
	strncpy((char*) &cmdbuf[1], name,8);
	cmdbuf[3] = strlen(name);

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(registeredOut) *registeredOut = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

Result srvIsPortRegistered(bool* registeredOut, const char* name)
{
	Handle port;
	Result rc = srvGetPort(&port, name);

	if(rc == 0xD8801BFA)
	{
		if(registeredOut) *registeredOut = false;
		return 0;
	}
	else if(R_SUCCEEDED(rc))
	{
		if(registeredOut) *registeredOut = true;
		svcCloseHandle(port);
	}
	return rc;
}
