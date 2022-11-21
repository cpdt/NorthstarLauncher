#include "pch.h"
#include "main.h"
#include "logging.h"
#include "crashhandler.h"
#include "memalloc.h"
#include "nsprefix.h"
#include "plugin_abi.h"
#include "plugins.h"
#include "version.h"
#include "pch.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

#include <string.h>
#include <filesystem>

typedef void (*initPluginFuncPtr)(void* (*getPluginObject)(PluginObject));

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

bool InitialiseNorthstar()
{
	static bool bInitialised = false;
	if (bInitialised)
		return false;

	bInitialised = true;

	// initialise the console if needed (-northstar needs this)
	InitialiseConsole();
	// initialise logging before most other things so that they can use spdlog and it have the proper formatting
	InitialiseLogging();

	InitialiseNorthstarPrefix();
	InitialiseVersion();

	g_pPluginManager = new PluginManager();
	g_pPluginManager->LoadPlugins();

	// Fix some users' failure to connect to respawn datacenters
	SetEnvironmentVariableA("OPENSSL_ia32cap", "~0x200000200000000");

	curl_global_init_mem(CURL_GLOBAL_DEFAULT, _malloc_base, _free_base, _realloc_base, _strdup_base, _calloc_base);

	InitialiseCrashHandler();
	InstallInitialHooks();
	CreateLogFiles();

	// Write launcher version to log
	spdlog::info("NorthstarLauncher version: {}", version);

	// run callbacks for any libraries that are already loaded by now
	CallAllPendingDLLLoadCallbacks();

	return true;
}
