#pragma once
#include "common.hpp"
#include "hooks/hook_types.hpp"
#include "memory/iat.hpp"
#include "memory/minhook.hpp"
#include "game/function_types.hpp"

namespace Client {
	namespace Hook {
		class Hooks {
		public:
			// IAT
			using HK_SetUnhandledExceptionFilter = HookPlate::StdcallHook<"kernel32/SetUnhandledExceptionFilter", LPTOP_LEVEL_EXCEPTION_FILTER,
				LPTOP_LEVEL_EXCEPTION_FILTER>;
			Memory::IAT* m_SetUnhandledExceptionFilterHK;

			// Bypasser hook
			using HK_RtlDispatchException = HookPlate::FastcallHook<"RtlDispatchException", bool,
				PEXCEPTION_RECORD, PCONTEXT>;
			Memory::MinHook<Game::Functions::RtlDispatchExceptionT>* m_RtlDispatchExceptionHK;

			// Regular game hooks
			using HK_BB_Alert = HookPlate::FastcallHook<"BB_Alert", void,
				const char*, const char*>;
			Memory::MinHook<Game::Functions::BB_AlertT>* m_BB_AlertHK;

			// ?????? hooks
			using HK_CreateMutexExA = HookPlate::StdcallHook<"CreateMutexExA", HANDLE,
				const LPSECURITY_ATTRIBUTES, const LPCSTR, const DWORD, const DWORD>;
			Memory::MinHook<decltype(CreateMutexExA)>* m_CreateMutexExAHK;

			using HK_GetThreadContext = HookPlate::StdcallHook<"GetThreadContext", BOOL,
				const HANDLE, const LPCONTEXT>;
			Memory::MinHook<>* m_GetThreadContextHK;

			using HK_NtQueryInformationProcess = HookPlate::StdcallHook<"NtQueryInformationProcess", NTSTATUS,
				const HANDLE, const PROCESSINFOCLASS, const PVOID, const ULONG, const PULONG>;
			Memory::MinHook<>* m_NtQueryInformationProcessHK;

			// Event handlers
			HookPlate::EventHandlerStore m_EventHandlerStore{};
			static void OnShowOverStack();

			explicit Hooks();
			~Hooks();

			void PostArxanDetectionHooks();

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

			void DeleteHook(Memory::IAT** hook) {
				if (!hook || !*hook) {
					return;
				}

				(*hook)->Unhook();

				delete* hook;
				*hook = nullptr;
			}
		};
	}

	inline std::unique_ptr<Hook::Hooks> g_Hooks{};
}
