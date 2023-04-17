/*
	This project serves as a simple demonstration for the article "Combining Raw Input and keyboard Hook to selectively block input from multiple keyboards",
	which you should be able to find in this project folder (HookingRawInput.html), or on the CodeProject website (http://www.codeproject.com/).
	The project is based on the idea shown to me by Petr Medek (http://www.hidmacros.eu/), and is published with his permission, huge thanks to Petr!
	The source code is licensed under The Code Project Open License (CPOL), feel free to adapt it.
	Vít Blecha (sethest@gmail.com), 2014
*/

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include "inputblocker_dll.h"

#pragma data_seg (".SHARED")
// Windows message for communication between main executable and DLL module
UINT const WM_HOOK = WM_APP + 1;
// HWND of the main executable (managing application)
HWND hwndServer = NULL;
#pragma data_seg ()
#pragma comment (linker, "/section:.SHARED,RWS")

HINSTANCE instanceHandle;
HHOOK hookHandle;

DWORD_PTR dwResult;

BOOL APIENTRY DllMain (HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			instanceHandle = hModule;
			hookHandle = NULL;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

// Keyboard Hook procedure
static LRESULT CALLBACK KeyboardProc (int code, WPARAM wParam, LPARAM lParam)
{
	if (code != 0)
	{ 
		return CallNextHookEx (hookHandle, code, wParam, lParam);
	}
	
	// Report the event to the main window. If the return value is 1, block the input; otherwise pass it along the Hook Chain
	DWORD res = SendMessageTimeout(hwndServer, WM_HOOK, wParam, lParam, SMTO_ABORTIFHUNG, 100, &dwResult);
	if (res != 0 && dwResult != 0)
	{	
		dwResult = 0;
		return 1;
	}

	/*if (SendMessage(hwndServer, WM_HOOK, wParam, lParam)) {
		return 1;
	}*/
	
	return CallNextHookEx (hookHandle, code, wParam, lParam);
}

BOOL InstallHook (HWND hwndParent)
{
	if (hwndServer != NULL)
	{
		// Already hooked
		return FALSE;
	}

	// Register keyboard Hook
	hookHandle = SetWindowsHookEx (WH_KEYBOARD, (HOOKPROC)KeyboardProc, instanceHandle, 0);
	if (hookHandle == NULL)
	{
		return FALSE;
	}
	hwndServer = hwndParent;
	return TRUE;
}

BOOL UninstallHook ()
{
	if (hookHandle == NULL)
	{
		return TRUE;
	}
	// If unhook attempt fails, check whether it is because of invalid handle (in that case continue)
	if (!UnhookWindowsHookEx (hookHandle))
	{
		DWORD error = GetLastError ();
		if (error != ERROR_INVALID_HOOK_HANDLE)
		{
			return FALSE;
		}
	}
	hwndServer = NULL;
	hookHandle = NULL;
	return TRUE;
}
