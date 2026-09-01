#include "common.hpp"
#include "arxan/sys_hooks.hpp"
#include "memory/memory.hpp"
#include "game/game.hpp"

#include <utility/memory.hpp>
#include <utility/nt.hpp>

namespace Client::Arxan {
	SysHooks::SysHooks() {
		this->m_GetThreadContextHK = nullptr;
		//this->m_GetThreadContextHK = new Memory::MinHook("kernelbase.dll", "GetThreadContext");
		//this->m_GetThreadContextHK->Hook<HK_GetThreadContext>();

		this->m_SetThreadContextHK = new Memory::MinHook("kernelbase.dll", "SetThreadContext");
		this->m_SetThreadContextHK->Hook<HK_SetThreadContext>();

		this->m_CheckRemoteDebuggerPresentHK = new Memory::MinHook("kernelbase.dll", "CheckRemoteDebuggerPresent");
		this->m_CheckRemoteDebuggerPresentHK->Hook<HK_CheckRemoteDebuggerPresent>();

		this->m_AddVectoredExceptionHandlerHK = new Memory::MinHook("kernel32.dll", "AddVectoredExceptionHandler");
		this->m_AddVectoredExceptionHandlerHK->Hook<HK_AddVectoredExceptionHandler>();

		this->m_CreateThreadHK = new Memory::MinHook("kernel32.dll", "CreateThread");
		this->m_CreateThreadHK->Hook<HK_CreateThread>();

		this->m_CreateMutexExAHK = nullptr;
		//this->m_CreateMutexExAHK = new Memory::MinHook("kernel32.dll", "CreateMutexExA");
		//this->m_CreateMutexExAHK->Hook<HK_CreateMutexExA>();

		// we need this hook for hwbp syscall stuff to work
		this->m_NtAllocateVirtualMemoryHK = new Memory::MinHook("ntdll.dll", "NtAllocateVirtualMemory");
		this->m_NtAllocateVirtualMemoryHK->Hook<HK_NtAllocateVirtualMemory>();

		this->m_GetWindowThreadProcessIdHK = new Memory::MinHook("user32.dll", "GetWindowThreadProcessId");
		this->m_GetWindowThreadProcessIdHK->Hook<HK_GetWindowThreadProcessId>();

		this->m_GetClassNameAHK = new Memory::MinHook("user32.dll", "GetClassNameA");
		this->m_GetClassNameAHK->Hook<HK_GetClassNameA>();

		this->m_GetWindowTextAHK = new Memory::MinHook("user32.dll", "GetWindowTextA");
		this->m_GetWindowTextAHK->Hook<HK_GetWindowTextA>();

		this->m_EnumChildWindowsHK = new Memory::MinHook("user32.dll", "EnumChildWindows");
		this->m_EnumChildWindowsHK->Hook<HK_EnumChildWindows>();

		this->m_GetMenuHK = new Memory::MinHook("user32.dll", "GetMenu");
		//this->m_GetMenuHK->Hook<HK_GetMenu>();

		this->m_GetMenuStringAHK = new Memory::MinHook("user32.dll", "GetMenuStringA");
		//this->m_GetMenuStringAHK->Hook<HK_GetMenuStringA>();

		this->m_GetSubMenuHK = new Memory::MinHook("user32.dll", "GetSubMenu");
		//this->m_GetSubMenuHK->Hook<HK_GetSubMenu>();

		this->m_SetWindowsHookExWHK = new Memory::MinHook("user32.dll", "SetWindowsHookExW");
		//this->m_SetWindowsHookExWHK->Hook<HK_SetWindowsHookExW>();

		this->m_CreateWindowExWHK = new Memory::MinHook("user32.dll", "CreateWindowExW");
		//this->m_CreateWindowExWHK->Hook<HK_CreateWindowExW>();

		this->m_NtUserQueryWindowHK = new Memory::MinHook("win32u.dll", "NtUserQueryWindow");
		//this->m_NtUserQueryWindowHK->Hook<HK_NtUserQueryWindow>();

		this->m_NtUserGetForegroundWindowHK = new Memory::MinHook("win32u.dll", "NtUserGetForegroundWindow");
		//this->m_NtUserGetForegroundWindowHK->Hook<HK_NtUserGetForegroundWindow>();

		this->m_NtUserBuildHwndListHK = new Memory::MinHook("win32u.dll", "NtUserBuildHwndList");
		//this->m_NtUserBuildHwndListHK->Hook<HK_NtUserBuildHwndList>();

		this->m_NtUserFindWindowExHK = new Memory::MinHook("win32u.dll", "NtUserFindWindowEx");
		//this->m_NtUserFindWindowExHK->Hook<HK_NtUserFindWindowEx>();

		this->m_NtUserWindowFromPointHK = new Memory::MinHook("win32u.dll", "NtUserWindowFromPoint");
		//this->m_NtUserWindowFromPointHK->Hook<HK_NtUserWindowFromPoint>();

		this->m_NtUserInternalGetWindowTextHK = new Memory::MinHook("win32u.dll", "NtUserInternalGetWindowText");
		//this->m_NtUserInternalGetWindowTextHK->Hook<HK_NtUserInternalGetWindowText>();

		this->m_NtUserGetWindowProcessHandleHK = new Memory::MinHook("win32u.dll", "NtUserGetWindowProcessHandle");
		//this->m_NtUserGetWindowProcessHandleHK->Hook<HK_NtUserGetWindowProcessHandle>();

		this->m_NtUserGetTopLevelWindowHK = new Memory::MinHook("win32u.dll", "NtUserGetTopLevelWindow");
		//this->m_NtUserGetTopLevelWindowHK->Hook<HK_NtUserGetTopLevelWindow>();

		this->m_NtUserChildWindowFromPointExHK = new Memory::MinHook("win32u.dll", "NtUserChildWindowFromPointEx");
		//this->m_NtUserChildWindowFromPointExHK->Hook<HK_NtUserChildWindowFromPointEx>();

		this->m_NtUserInternalGetWindowIconHK = new Memory::MinHook("win32u.dll", "NtUserInternalGetWindowIcon");
		//this->m_NtUserInternalGetWindowIconHK->Hook<HK_NtUserInternalGetWindowIcon>();

		this->m_NtUserRealChildWindowFromPointHK = new Memory::MinHook("win32u.dll", "NtUserRealChildWindowFromPoint");
		//this->m_NtUserRealChildWindowFromPointHK->Hook<HK_NtUserRealChildWindowFromPoint>();

		this->m_NtUserWindowFromDCHK = new Memory::MinHook("win32u.dll", "NtUserWindowFromDC");
		//this->m_NtUserWindowFromDCHK->Hook<HK_NtUserWindowFromDC>();

		this->m_NtUserGetClassNameHK = new Memory::MinHook("win32u.dll", "NtUserGetClassName");
		//this->m_NtUserGetClassNameHK->Hook<HK_NtUserGetClassName>();
	}

	SysHooks::~SysHooks() {
		this->DeleteHook(&this->m_NtUserGetClassNameHK);
		this->DeleteHook(&this->m_NtUserWindowFromDCHK);
		this->DeleteHook(&this->m_NtUserRealChildWindowFromPointHK);
		this->DeleteHook(&this->m_NtUserInternalGetWindowIconHK);
		this->DeleteHook(&this->m_NtUserChildWindowFromPointExHK);
		this->DeleteHook(&this->m_NtUserGetTopLevelWindowHK);
		this->DeleteHook(&this->m_NtUserGetWindowProcessHandleHK);
		this->DeleteHook(&this->m_NtUserInternalGetWindowTextHK);
		this->DeleteHook(&this->m_NtUserWindowFromPointHK);
		this->DeleteHook(&this->m_NtUserFindWindowExHK);
		this->DeleteHook(&this->m_NtUserBuildHwndListHK);
		this->DeleteHook(&this->m_NtUserGetForegroundWindowHK);
	}
}
