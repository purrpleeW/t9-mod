#pragma once
#include "common.hpp"

#include <utility/nt.hpp>

namespace Client::Arxan {
	class NtDllRestore {
	public:
		using Buffer = uint8_t[15];
	private:
		static inline void UtilCopy(void* place, const void* data, const size_t length) {
			DWORD oldProtect{};
			VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &oldProtect);

			std::memmove(place, data, length);

			VirtualProtect(place, length, oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, length);
		}
	public:
		static inline void RestoreDebugFunctions() {
			static Common::Utility::NT::Library ntdll("ntdll.dll");
			static Buffer buffers[ARRAYSIZE(s_DebugFunctions)] = {};
			static bool loaded = false;

			for (auto i = 0u; i < ARRAYSIZE(s_DebugFunctions); ++i) {
				void* functionAddr = ntdll.GetProc<void*>(s_DebugFunctions[i]);
				if (functionAddr == nullptr) {
					continue;
				}

				if (!loaded) {
					memcpy(buffers[i], functionAddr, sizeof(Buffer));
				}
				else {
					UtilCopy(functionAddr, buffers[i], sizeof(Buffer));
				}
			}

			loaded = true;
		}

		static inline bool EnableDebugPrivilege() {
			bool bResult = false;
			HANDLE hToken = NULL;

			do {
				if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
					break;
				}

				TOKEN_PRIVILEGES tp;
				tp.PrivilegeCount = 1;
				if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
					break;
				}

				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
				if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
					break;
				}

				bResult = true;
			} while (0);

			if (hToken) {
				CloseHandle(hToken);
			}

			return bResult;
		}
	private:
		static inline const char* s_DebugFunctions[] = {
			"DbgBreakPoint",
			"DbgUserBreakPoint",
			"DbgUiConnectToDbg",
			"DbgUiContinue",
			"DbgUiConvertStateChangeStructure",
			"DbgUiDebugActiveProcess",
			"DbgUiGetThreadDebugObject",
			"DbgUiIssueRemoteBreakin",
			"DbgUiRemoteBreakin",
			"DbgUiSetThreadDebugObject",
			"DbgUiStopDebugging",
			"DbgUiWaitStateChange",
			"DbgPrintReturnControlC",
			"DbgPrompt"
		};
	};
}
