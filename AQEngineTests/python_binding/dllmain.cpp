// dllmain.cpp : Définit le point d'entrée pour l'application DLL.
#include <windows.h>
#include <aq/Logger.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
    aq::Logger::getInstance().setLevel(AQ_LOG_WARNING);
    break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

