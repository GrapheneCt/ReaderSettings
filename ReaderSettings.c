#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/io/dirent.h> 
#include <psp2/registrymgr.h>
#include <taihen.h>
#include <string.h>

#define auth_path "ux0:data/rddat/settings/fake_auth"
#define sync_path "ux0:data/rddat/settings/auto_sync"
#define guide_path "ux0:data/rddat/settings/guidebook"
#define sample_path "ux0:data/rddat/settings/sample"
#define locux_path "ux0:data/rddat/settings/ux"
#define locuma_path "ux0:data/rddat/settings/uma"
#define locur_path "ux0:data/rddat/settings/ur"
#define locgrw_path "ux0:data/rddat/settings/grw"

static SceUID g_hooks[5];

static tai_hook_ref_t g_sceRegMgrGetKeyInt_SceSystemSettingsCore_hook;
static int sceRegMgrGetKeyInt_SceSystemSettingsCore_patched(const char *category, const char *name, int *value) 
{
	if (sceClibStrncmp(category, "/CONFIG/READER", 14) == 0) 
	{
		int dfd;
		if (value) 
		{
			if (sceClibStrncmp(name, "fake_auth", 9) == 0)
			{
				dfd = sceIoDopen(auth_path);
				sceIoDclose(dfd);
				*value = 1;
				if (dfd < 0)
				{
					*value = 0;
				}
			}
			else if (sceClibStrncmp(name, "auto_sync", 9) == 0) 
			{
				dfd = sceIoDopen(sync_path);
				sceIoDclose(dfd);
				*value = 1;
				if (dfd < 0)
					*value = 0;
			}
			else if (sceClibStrncmp(name, "guidebook", 9) == 0)
			{
				dfd = sceIoDopen(guide_path);
				sceIoDclose(dfd);
				*value = 1;
				if (dfd < 0)
					*value = 0;
			}
			else if (sceClibStrncmp(name, "sample", 6) == 0)
			{
				dfd = sceIoDopen(sample_path);
				sceIoDclose(dfd);
				*value = 1;
				if (dfd < 0)
					*value = 0;
			}
			else if (sceClibStrncmp(name, "path", 4) == 0)
			{
				dfd = sceIoDopen(locux_path);
				sceIoDclose(dfd);
				if (dfd >= 0)
					*value = 0;
				else 
				{
					dfd = sceIoDopen(locuma_path);
					sceIoDclose(dfd);
					if (dfd >= 0)
						*value = 1;
					else
					{
						dfd = sceIoDopen(locur_path);
						sceIoDclose(dfd);
						if (dfd >= 0)
							*value = 2;
						else
						{
							dfd = sceIoDopen(locgrw_path);
							sceIoDclose(dfd);
							if (dfd >= 0)
								*value = 3;
						}
					}
				}
			}
		}
		return 0;
	}
	return TAI_CONTINUE(int, g_sceRegMgrGetKeyInt_SceSystemSettingsCore_hook, category, name, value);
}

static tai_hook_ref_t g_sceRegMgrSetKeyInt_SceSystemSettingsCore_hook;
static int sceRegMgrSetKeyInt_SceSystemSettingsCore_patched(const char *category, const char *name, int value) 
{
	if (sceClibStrncmp(category, "/CONFIG/READER", 14) == 0) 
	{
		if (sceClibStrncmp(name, "fake_auth", 9) == 0) 
		{
			if (value == 1)
				sceIoMkdir(auth_path, 0777);
			else
				sceIoRmdir(auth_path);
		}
		else if (sceClibStrncmp(name, "auto_sync", 9) == 0) 
		{
			if (value == 1)
				sceIoMkdir(sync_path, 0777);
			else
				sceIoRmdir(sync_path);
		}
		else if (sceClibStrncmp(name, "guidebook", 9) == 0) 
		{
			if (value == 1)
				sceIoMkdir(guide_path, 0777);
			else
				sceIoRmdir(guide_path);
		}
		else if (sceClibStrncmp(name, "sample", 6) == 0)
		{
			if (value == 1)
				sceIoMkdir(sample_path, 0777);
			else
				sceIoRmdir(sample_path);
		}
		else if (sceClibStrncmp(name, "path", 4) == 0)
		{
			sceIoRmdir(locux_path);
			sceIoRmdir(locuma_path);
			sceIoRmdir(locur_path);
			sceIoRmdir(locgrw_path);
			if (value == 0)
				sceIoMkdir(locux_path, 0777);
			else if (value == 1)
				sceIoMkdir(locuma_path, 0777);
			else if (value == 2)
				sceIoMkdir(locur_path, 0777);
			else if (value == 3)
				sceIoMkdir(locgrw_path, 0777);
		}
		return 0;
	}
	return TAI_CONTINUE(int, g_sceRegMgrSetKeyInt_SceSystemSettingsCore_hook, category, name, value);
}

typedef struct {
	int size;
	const char *name;
	int type;
	int unk;
} SceRegMgrKeysInfo;

static tai_hook_ref_t g_sceRegMgrGetKeysInfo_SceSystemSettingsCore_hook;
static int sceRegMgrGetKeysInfo_SceSystemSettingsCore_patched(const char *category, SceRegMgrKeysInfo *info, int unk) {
	if (sceClibStrncmp(category, "/CONFIG/READER", 14) == 0) {
		if (info)
			info->type = 0x00040000; // type integer
		return 0;
	}
	return TAI_CONTINUE(int, g_sceRegMgrGetKeysInfo_SceSystemSettingsCore_hook, category, info, unk);
}

static SceUID g_system_settings_core_modid = -1;
static tai_hook_ref_t g_sceKernelLoadStartModule_SceSettings_hook;
static SceUID sceKernelLoadStartModule_SceSettings_patched(char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status) {
	SceUID ret = TAI_CONTINUE(SceUID, g_sceKernelLoadStartModule_SceSettings_hook, path, args, argp, flags, option, status);
	if (ret >= 0 && sceClibStrncmp(path, "vs0:app/NPXS10015/system_settings_core.suprx", 44) == 0) {
		g_system_settings_core_modid = ret;
		g_hooks[2] = taiHookFunctionImport(&g_sceRegMgrGetKeyInt_SceSystemSettingsCore_hook,
			"SceSystemSettingsCore",
			0xC436F916, // SceRegMgr
			0x16DDF3DC,
			sceRegMgrGetKeyInt_SceSystemSettingsCore_patched);
		g_hooks[3] = taiHookFunctionImport(&g_sceRegMgrSetKeyInt_SceSystemSettingsCore_hook,
			"SceSystemSettingsCore",
			0xC436F916, // SceRegMgr
			0xD72EA399,
			sceRegMgrSetKeyInt_SceSystemSettingsCore_patched);
		g_hooks[4] = taiHookFunctionImport(&g_sceRegMgrGetKeysInfo_SceSystemSettingsCore_hook,
			"SceSystemSettingsCore",
			0xC436F916, // SceRegMgr
			0x58421DD1,
			sceRegMgrGetKeysInfo_SceSystemSettingsCore_patched);
	}
	return ret;
}

static tai_hook_ref_t g_sceKernelStopUnloadModule_SceSettings_hook;
static int sceKernelStopUnloadModule_SceSettings_patched(SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status) {
	if (modid == g_system_settings_core_modid) {
		g_system_settings_core_modid = -1;
		if (g_hooks[2] >= 0) taiHookRelease(g_hooks[2], g_sceRegMgrGetKeyInt_SceSystemSettingsCore_hook);
		if (g_hooks[3] >= 0) taiHookRelease(g_hooks[3], g_sceRegMgrSetKeyInt_SceSystemSettingsCore_hook);
		if (g_hooks[4] >= 0) taiHookRelease(g_hooks[4], g_sceRegMgrGetKeysInfo_SceSystemSettingsCore_hook);
	}
	return TAI_CONTINUE(int, g_sceKernelStopUnloadModule_SceSettings_hook, modid, args, argp, flags, option, status);
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp)
{

	g_hooks[0] = taiHookFunctionImport(&g_sceKernelLoadStartModule_SceSettings_hook,
		"ReaderSettings",
		0xCAE9ACE6, // SceLibKernel
		0x2DCC4AFA,
		sceKernelLoadStartModule_SceSettings_patched);
	g_hooks[1] = taiHookFunctionImport(&g_sceKernelStopUnloadModule_SceSettings_hook,
		"ReaderSettings",
		0xCAE9ACE6, // SceLibKernel
		0x2415F8A4,
		sceKernelStopUnloadModule_SceSettings_patched);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp)
{
	if (g_hooks[0] >= 0) taiHookRelease(g_hooks[0], g_sceKernelLoadStartModule_SceSettings_hook);
	if (g_hooks[1] >= 0) taiHookRelease(g_hooks[1], g_sceKernelStopUnloadModule_SceSettings_hook);
	if (g_hooks[2] >= 0) taiHookRelease(g_hooks[2], g_sceRegMgrGetKeyInt_SceSystemSettingsCore_hook);
	if (g_hooks[3] >= 0) taiHookRelease(g_hooks[3], g_sceRegMgrSetKeyInt_SceSystemSettingsCore_hook);
	if (g_hooks[4] >= 0) taiHookRelease(g_hooks[4], g_sceRegMgrGetKeysInfo_SceSystemSettingsCore_hook);

	return SCE_KERNEL_STOP_SUCCESS;
}
