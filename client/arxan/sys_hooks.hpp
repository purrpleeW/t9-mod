#pragma once
#include "common.hpp"
#include "hooks/hook_types.hpp"
#include "memory/minhook.hpp"

namespace Client {
	namespace Arxan {
		class SysHooks {
		public:
			using HK_GetThreadContext = HookPlate::StdcallHook<"kernelbase/GetThreadContext", BOOL,
				HANDLE, LPCONTEXT>;
			Memory::MinHook<>* m_GetThreadContextHK;

			using HK_SetThreadContext = HookPlate::StdcallHook<"kernelbase/SetThreadContext", BOOL,
				HANDLE, const CONTEXT*>;
			Memory::MinHook<>* m_SetThreadContextHK;

			using HK_CheckRemoteDebuggerPresent = HookPlate::StdcallHook<"kernelbase/CheckRemoteDebuggerPresent", BOOL,
				HANDLE, PBOOL>;
			Memory::MinHook<>* m_CheckRemoteDebuggerPresentHK;

			using HK_AddVectoredExceptionHandler = HookPlate::StdcallHook<"kernel32/AddVectoredExceptionHandler", PVOID,
				ULONG, PVECTORED_EXCEPTION_HANDLER>;
			Memory::MinHook<>* m_AddVectoredExceptionHandlerHK;

			using HK_CreateThread = HookPlate::StdcallHook<"kernel32/CreateThread", HANDLE,
				LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, __drv_aliasesMem LPVOID, DWORD, LPDWORD>;
			Memory::MinHook<>* m_CreateThreadHK;

			using HK_CreateMutexExA = HookPlate::StdcallHook<"kernel32/CreateMutexExA", HANDLE,
				LPSECURITY_ATTRIBUTES, LPCSTR, DWORD, DWORD>;
			Memory::MinHook<>* m_CreateMutexExAHK;

			// we need this hook for hwbp syscall stuff to work
			using HK_NtAllocateVirtualMemory = HookPlate::StdcallHook<"ntdll/NtAllocateVirtualMemory", NTSTATUS,
				HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG>;
			Memory::MinHook<>* m_NtAllocateVirtualMemoryHK;

			using HK_GetWindowThreadProcessId = HookPlate::StdcallHook<"user32/GetWindowThreadProcessId", DWORD,
				HWND, LPDWORD>;
			Memory::MinHook<>* m_GetWindowThreadProcessIdHK;

			using HK_GetClassNameA = HookPlate::StdcallHook<"user32/GetClassNameA", int,
				HWND, LPSTR, int>;
			Memory::MinHook<>* m_GetClassNameAHK;

			using HK_GetWindowTextA = HookPlate::StdcallHook<"user32/GetWindowTextA", int,
				HWND, LPSTR, int>;
			Memory::MinHook<>* m_GetWindowTextAHK;

			using HK_EnumChildWindows = HookPlate::StdcallHook<"user32/EnumChildWindows", BOOL,
				HWND, WNDENUMPROC, LPARAM>;
			Memory::MinHook<>* m_EnumChildWindowsHK;

			using HK_GetMenu = HookPlate::StdcallHook<"user32/GetMenu", HMENU,
				HWND>;
			Memory::MinHook<>* m_GetMenuHK;

			using HK_GetMenuStringA = HookPlate::StdcallHook<"user32/GetMenuStringA", int,
				HMENU, UINT, LPSTR, int, UINT>;
			Memory::MinHook<>* m_GetMenuStringAHK;

			using HK_GetSubMenu = HookPlate::StdcallHook<"user32/GetSubMenu", HMENU,
				HMENU, int>;
			Memory::MinHook<>* m_GetSubMenuHK;

			using HK_SetWindowsHookExW = HookPlate::StdcallHook<"user32/SetWindowsHookExW", HHOOK,
				int, HOOKPROC, HINSTANCE, DWORD>;
			Memory::MinHook<>* m_SetWindowsHookExWHK;

			using HK_CreateWindowExW = HookPlate::StdcallHook<"user32/CreateWindowExW", HWND,
				DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID>;
			Memory::MinHook<>* m_CreateWindowExWHK;

			using HK_NtUserQueryWindow = HookPlate::StdcallHook<"win32u/NtUserQueryWindow", HANDLE,
				HWND, int /* WINDOWINFOCLASS */>;
			Memory::MinHook<>* m_NtUserQueryWindowHK;

			using HK_NtUserGetForegroundWindow = HookPlate::StdcallHook<"win32u/NtUserGetForegroundWindow", HWND>;
			Memory::MinHook<>* m_NtUserGetForegroundWindowHK;

			using HK_NtUserBuildHwndList = HookPlate::StdcallHook<"win32u/NtUserBuildHwndList", NTSTATUS,
				HDESK, HWND, BOOL, BOOL, DWORD, UINT, HWND*, PULONG>;
			Memory::MinHook<>* m_NtUserBuildHwndListHK;

			using HK_NtUserFindWindowEx = HookPlate::StdcallHook<"win32u/NtUserFindWindowEx", HWND,
				HWND, HWND, PUNICODE_STRING, PUNICODE_STRING>;
			Memory::MinHook<>* m_NtUserFindWindowExHK;

			using HK_NtUserWindowFromPoint = HookPlate::StdcallHook<"win32u/NtUserWindowFromPoint", HWND,
				LONG, LONG>;
			Memory::MinHook<>* m_NtUserWindowFromPointHK;

			using HK_NtUserInternalGetWindowText = HookPlate::StdcallHook<"win32u/NtUserInternalGetWindowText", ULONG,
				HWND, LPWSTR, ULONG>;
			Memory::MinHook<>* m_NtUserInternalGetWindowTextHK;

			using HK_NtUserGetWindowProcessHandle = HookPlate::StdcallHook<"win32u/NtUserGetWindowProcessHandle", void*>;
			Memory::MinHook<>* m_NtUserGetWindowProcessHandleHK;

			using HK_NtUserGetTopLevelWindow = HookPlate::StdcallHook<"win32u/NtUserGetTopLevelWindow", void*>;
			Memory::MinHook<>* m_NtUserGetTopLevelWindowHK;

			using HK_NtUserChildWindowFromPointEx = HookPlate::StdcallHook<"win32u/NtUserChildWindowFromPointEx", HWND,
				HWND, POINT, ULONG>;
			Memory::MinHook<>* m_NtUserChildWindowFromPointExHK;

			using HK_NtUserInternalGetWindowIcon = HookPlate::StdcallHook<"win32u/NtUserInternalGetWindowIcon", HICON,
				HWND, ULONG>;
			Memory::MinHook<>* m_NtUserInternalGetWindowIconHK;

			using HK_NtUserRealChildWindowFromPoint = HookPlate::StdcallHook<"win32u/NtUserRealChildWindowFromPoint", HWND,
				HWND, POINT>;
			Memory::MinHook<>* m_NtUserRealChildWindowFromPointHK;

			using HK_NtUserWindowFromDC = HookPlate::StdcallHook<"win32u/NtUserWindowFromDC", HWND,
				HDC>;
			Memory::MinHook<>* m_NtUserWindowFromDCHK;

			using HK_NtUserGetClassName = HookPlate::StdcallHook<"win32u/NtUserGetClassName", int,
				HWND, BOOL, PUNICODE_STRING>;
			Memory::MinHook<>* m_NtUserGetClassNameHK;

			explicit SysHooks();
			~SysHooks();

			template <typename T>
			void DeleteHook(Memory::MinHook<T>** hook, std::vector<int> indexes = {}) {
				if (!hook || !*hook) {
					return;
				}

				if (indexes.empty()) {
					(*hook)->Unhook();
				}
				else {
					for (int index : indexes) {
						(*hook)->Unhook(index);
					}
				}

				delete* hook;
				*hook = nullptr;
			}
		};
	}

	inline std::unique_ptr<Arxan::SysHooks> g_ArxanSysHooks{};
}
