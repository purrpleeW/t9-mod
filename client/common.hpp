#pragma once
#include <common_core.hpp>
#include <global/game_launch_type.hpp>
#include <logger/log_service.hpp>
#include <utility/winapi.hpp>

//#include "resource.h"

#include <bcrypt.h>
#include <winternl.h>
#include <intrin.h>

#define VT_GET(ptr, idx) (*(void***)(ptr))[idx]
#define HIGH_ORDER_LOG_HOOK 0
#define PTR_AS(type, ptr) reinterpret_cast<type>((ptr))
#define DEREF_PTR_AS(type, ptr) *PTR_AS(type*, ptr)

namespace Client {
	using namespace std::chrono_literals;
	using namespace std::string_literals;

	enum class GameVersion {
		NONE,
		v1_34_0_15931218,
		v1_34_1_15931218
	};

	struct GameIdentifier {
		std::string m_Version = "Unknown version";
		GameVersion m_Checksum = GameVersion::NONE;
	};

	inline std::unordered_map<std::uint32_t, GameIdentifier> g_GameVersions{
		{ 0x64CD0B8C, { "1.34.0.15931218", GameVersion::v1_34_0_15931218 } },
		{ 0x645C43F8, { "1.34.1.15931218", GameVersion::v1_34_1_15931218 } }
	};

	inline HMODULE g_Module{};
	inline HANDLE g_MainThread{};
	inline DWORD g_MainThreadId{};
	inline std::atomic_bool g_Running{ true };
	inline std::string g_GameModuleName = "?";

	inline GameIdentifier g_GameIdentifier{};

	// CWHook / Arxan things
	inline CONTEXT g_CurrentContext{};
	inline std::atomic_bool g_SuspendNewThreads{ false };
	inline std::atomic_bool g_Debugging{ false };
	inline std::vector<void*> g_VectoredExceptionHandlers{};

	NTSTATUS NtContinue(PCONTEXT threadContext, BOOLEAN raiseAlert);
	std::string GetWindowsUsername();

	bool RemoveEvilKeywordsFromString(const UNICODE_STRING& string);
	bool RemoveEvilKeywordsFromString(wchar_t* str, const size_t length);
	bool RemoveEvilKeywordsFromString(char* str, const size_t length);

	// other things
	std::string AndRel(std::uintptr_t address);
}

// more CWHook / Arxan things

#define pushad64() a.push(asmjit::x86::rax); 	\
				a.push(asmjit::x86::rcx); 	\
				a.push(asmjit::x86::rdx);	\
				a.push(asmjit::x86::rbx);	\
				a.push(asmjit::x86::rsp);	\
				a.push(asmjit::x86::rbp);	\
				a.push(asmjit::x86::rsi);	\
				a.push(asmjit::x86::rdi);	\
				a.push(asmjit::x86::r8);	\
				a.push(asmjit::x86::r9);	\
				a.push(asmjit::x86::r10);	\
				a.push(asmjit::x86::r11);	\
				a.push(asmjit::x86::r12);	\
				a.push(asmjit::x86::r13);	\
				a.push(asmjit::x86::r14);	\
				a.push(asmjit::x86::r15);


#define popad64() a.pop(asmjit::x86::r15); 	\
				a.pop(asmjit::x86::r14);	\
				a.pop(asmjit::x86::r13);	\
				a.pop(asmjit::x86::r12);	\
				a.pop(asmjit::x86::r11);	\
				a.pop(asmjit::x86::r10);	\
				a.pop(asmjit::x86::r9);		\
				a.pop(asmjit::x86::r8);		\
				a.pop(asmjit::x86::rdi);	\
				a.pop(asmjit::x86::rsi);	\
				a.pop(asmjit::x86::rbp);	\
				a.pop(asmjit::x86::rsp);	\
				a.pop(asmjit::x86::rbx);	\
				a.pop(asmjit::x86::rdx);	\
				a.pop(asmjit::x86::rcx);	\
				a.pop(asmjit::x86::rax);

#define popad64WithoutRAX() a.pop(asmjit::x86::r11);	\
				a.pop(asmjit::x86::r10);	\
				a.pop(asmjit::x86::r9);		\
				a.pop(asmjit::x86::r8);		\
				a.pop(asmjit::x86::rdi);	\
				a.pop(asmjit::x86::rsi);	\
				a.pop(asmjit::x86::rbp);	\
				a.pop(asmjit::x86::rsp);	\
				a.pop(asmjit::x86::rbx);	\
				a.pop(asmjit::x86::rdx);	\
				a.pop(asmjit::x86::rcx);

// TODO: if we remove any of the r15 14 13 registers on the popad64 macro it crashes the game
// think we messed up the stack or something on the checksum stub generation
// fix it later, for now use these so we can actually use the r12-r15 registers since those are non violatile
#define pushad64_Min() a.push(asmjit::x86::rax); 	\
				a.push(asmjit::x86::rcx); 	\
				a.push(asmjit::x86::rdx);	\
				a.push(asmjit::x86::rbx);	\
				a.push(asmjit::x86::rsp);	\
				a.push(asmjit::x86::rbp);	\
				a.push(asmjit::x86::rsi);	\
				a.push(asmjit::x86::rdi);	\
				a.push(asmjit::x86::r8);	\
				a.push(asmjit::x86::r9);	\
				a.push(asmjit::x86::r10);	\
				a.push(asmjit::x86::r11);


#define popad64_Min() a.pop(asmjit::x86::r11);	\
				a.pop(asmjit::x86::r10);	\
				a.pop(asmjit::x86::r9);		\
				a.pop(asmjit::x86::r8);		\
				a.pop(asmjit::x86::rdi);	\
				a.pop(asmjit::x86::rsi);	\
				a.pop(asmjit::x86::rbp);	\
				a.pop(asmjit::x86::rsp);	\
				a.pop(asmjit::x86::rbx);	\
				a.pop(asmjit::x86::rdx);	\
				a.pop(asmjit::x86::rcx);	\
				a.pop(asmjit::x86::rax);