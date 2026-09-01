#include "common.hpp"
#include "arxan/arxan_utility.hpp"
#include "arxan/ntdll_restore.hpp"
#include "arxan/sys_calls.hpp"
#include "memory/memory.hpp"

#include <MinHook.h>

#define ASMJIT_STATIC
#include <asmjit/core/jitruntime.h>
#include <asmjit/core/operand.h>
#include <asmjit/x86/x86assembler.h>
#include <asmjit/x86/x86operand.h>

namespace Client::Arxan {
	namespace {
		enum ChecksumType {
			IntactSmall,
			IntactBig,
			Split
		};

		struct InlineAsmStub {
			void* m_FunctionAddress;
			uint8_t* m_Buffer;
			size_t m_BufferSize;
			ChecksumType m_Type;
		};

		struct NtdllDbgLocation {
			const char* m_FunctionName;
			void* m_AddrLocation;
			uint8_t m_PatchedByArxanBuffer[14];
		};

		struct IntactChecksumHook {
			uint64_t* m_FunctionAddress;
			uint8_t m_Buffer[7];
		};

		struct IntactBigChecksumHook {
			uint64_t* m_FunctionAddress;
			uint8_t m_Buffer[7 + 3];
		};

		struct SplitChecksumHook {
			uint64_t* m_FunctionAddress;
			uint8_t m_Buffer[8];
		};

		struct ChecksumHealingLocation {
			std::vector<Memory::ScannedResult<void>> m_ChecksumPattern;
			size_t m_Length;
		};

		// TODO: Sig these so they work on 1.34.1 too
		NtdllDbgLocation s_NtdllDbgLocations[] = {
			{ "DbgBreakPoint",						(void*)0x1BFA100E, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUserBreakPoint",					(void*)0x1E529177, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiConnectToDbg",					(void*)0x07694928, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiContinue",						(void*)0x1EA8F9BA, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiConvertStateChangeStructure",	(void*)0x1DBD8612, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiDebugActiveProcess",			(void*)0x1DFB2F44, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiGetThreadDebugObject",			(void*)0x1BD3FBF2, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiIssueRemoteBreakin",			(void*)0x1DB74F0B, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiRemoteBreakin",					(void*)0x1CB7AFD3, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiSetThreadDebugObject",			(void*)0x1BD63117, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiStopDebugging",					(void*)0x1BDB3908, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgUiWaitStateChange",				(void*)0x1BF02D1B, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgPrintReturnControlC",				(void*)0x05E9DFE8, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
			{ "DbgPrompt",							(void*)0x1D24ABA6, { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE0, 0x15, 0xB1, 0xFA, 0x7F, 0x00, 0x00 } },
		};

		std::vector<InlineAsmStub> s_InlineStubs{};

		std::vector<IntactChecksumHook> s_IntactChecksumHooks;
		std::vector<IntactBigChecksumHook> s_IntactBigChecksumHooks;
		std::vector<SplitChecksumHook> s_SplitChecksumHooks;

		void GeneralTlsCallbackStub() {
			return;
		}

		void ReplaceNtdllStackWithOurOwn(uint64_t addr, uint64_t locationCount) {
			uint64_t addressToReplace = *(uint64_t*)(char*)(addr - 0x8);
			uint64_t* addressReplace = (uint64_t*)addr;

			// we have to get the address location to exitprocess correctly or we will crash
			uint64_t kernel32ExitProcessAddress = (uint64_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.dll")), "ExitProcess");
			memcpy((char*)s_NtdllDbgLocations[locationCount].m_PatchedByArxanBuffer + 6, &kernel32ExitProcessAddress, sizeof(uint64_t));

			int counter = 0;
			for (int i = 0; i < INT_MAX; i++) {
				addressReplace++;
				uint64_t addressResult = *(uint64_t*)addressReplace;

				if (addressResult == addressToReplace) {
					*addressReplace = (uint64_t)s_NtdllDbgLocations[locationCount].m_PatchedByArxanBuffer;
					counter++;

					// there are 2 pointers that point to the ntdll dbg locations we nopped out
					// we replace those 2 pointers with our own location to satisfy arxan so that the game doesnt crash
					if (counter == 2) {
						return;
					}
				}
			}
		}

		bool IsRelativelyFar(const void* pointer, const void* data) {
			const int64_t diff = size_t(data) - (size_t(pointer) + 5);
			const auto small_diff = int32_t(diff);
			return diff != int64_t(small_diff);
		}

		uint8_t* AllocateSomewhereNear(const void* base_address, const size_t size) {
			size_t offset = 0;
			while (true) {
				offset += size;
				auto* target_address = static_cast<const uint8_t*>(base_address) - offset;
				if (IsRelativelyFar(base_address, target_address)) {
					return nullptr;
				}

				const auto res = VirtualAlloc(const_cast<uint8_t*>(target_address), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (res) {
					if (IsRelativelyFar(base_address, target_address)) {
						VirtualFree(res, 0, MEM_RELEASE);
						return nullptr;
					}

					return static_cast<uint8_t*>(res);
				}
			}
		}

		uint32_t ReverseBytes(uint32_t bytes) {
			uint32_t aux = 0;
			uint8_t byte;
			int i;

			for (i = 0; i < 32; i += 8) {
				byte = (bytes >> i) & 0xff;
				aux |= byte << (32 - 8 - i);
			}

			return aux;
		}

		int FixChecksum(uint64_t rbpOffset, uint64_t ptrOffset, uint64_t* ptrStack, uint32_t jmpInstructionDistance, uint32_t calculatedChecksumFromArg) {
			_Unreferenced_parameter_(jmpInstructionDistance);

			// get size of image from codcw
			uint64_t baseAddressStart = (uint64_t)GetModuleHandle(nullptr);
			IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
			IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);
			auto sizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
			uint64_t baseAddressEnd = baseAddressStart + sizeOfImage;

			// check if our checksum hooks got overwritten
			// we could probably ifdef this now but it's a good indicator to know if our checksum hooks still exist
			{
				for (int i = 0; i < s_IntactChecksumHooks.size(); i++) {
					DWORD oldProtect{};

					if (memcmp(s_IntactChecksumHooks[i].m_FunctionAddress, s_IntactChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 7)) {
						uint64_t idaAddress = (uint64_t)s_IntactChecksumHooks[i].m_FunctionAddress - baseAddressStart + 0x140000000;

						MessageBoxA(nullptr, "Oh no! Our checksum... it's broken.", "t9-mod/CWHook", MB_OK);
						//printf("%llx %llx got changed\n", idaAddress, (uint64_t)s_IntactChecksumHooks[i].m_FunctionAddress);
						//fprintf(logFile, "%llx got changed\n", idaAddress);
						//fflush(logFile);
					}

					VirtualProtect(s_IntactChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 7, PAGE_EXECUTE_READWRITE, &oldProtect);
					memcpy(s_IntactChecksumHooks[i].m_FunctionAddress, s_IntactChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 7);
					VirtualProtect(s_IntactChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 7, oldProtect, &oldProtect);
					FlushInstructionCache(GetCurrentProcess(), s_IntactChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 7);
				}

				for (int i = 0; i < s_IntactBigChecksumHooks.size(); i++) {
					DWORD oldProtect{};

					if (memcmp(s_IntactBigChecksumHooks[i].m_FunctionAddress, s_IntactBigChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 7)) {
						uint64_t idaAddress = (uint64_t)s_IntactBigChecksumHooks[i].m_FunctionAddress - baseAddressStart + 0x140000000;

						MessageBoxA(nullptr, "Oh no! Our big checksum... it's broken.", "t9-mod/CWHook", MB_OK);
						//printf("%llx %llx got changed\n", idaAddress, (uint64_t)s_IntactBigChecksumHooks[i].m_FunctionAddress);
						//fprintf(logFile, "%llx got changed\n", idaAddress);
						//fflush(logFile);
					}

					VirtualProtect(s_IntactBigChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 10, PAGE_EXECUTE_READWRITE, &oldProtect);
					memcpy(s_IntactBigChecksumHooks[i].m_FunctionAddress, s_IntactBigChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 10);
					VirtualProtect(s_IntactBigChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 10, oldProtect, &oldProtect);
					FlushInstructionCache(GetCurrentProcess(), s_IntactBigChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 10);
				}

				for (int i = 0; i < s_SplitChecksumHooks.size(); i++) {
					DWORD old_protect{};

					if (memcmp(s_SplitChecksumHooks[i].m_FunctionAddress, s_SplitChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 7)) {
						uint64_t idaAddress = (uint64_t)s_SplitChecksumHooks[i].m_FunctionAddress - baseAddressStart + 0x140000000;

						MessageBoxA(nullptr, "Oh no! Our split... it's broken.", "t9-mod/CWHook", MB_OK);
						//printf("%llx %llx got changed\n", idaAddress, (uint64_t)s_SplitChecksumHooks[i].m_FunctionAddress);
						//fprintf(logFile, "%llx got changed\n", idaAddress);
						//fflush(logFile);
					}

					VirtualProtect(s_SplitChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 8, PAGE_EXECUTE_READWRITE, &old_protect);
					memcpy(s_SplitChecksumHooks[i].m_FunctionAddress, s_SplitChecksumHooks[i].m_Buffer, sizeof(uint8_t) * 8);
					VirtualProtect(s_SplitChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 8, old_protect, &old_protect);
					FlushInstructionCache(GetCurrentProcess(), s_SplitChecksumHooks[i].m_FunctionAddress, sizeof(uint8_t) * 8);
				}
			}

			static int fixChecksumCalls = 0;
			fixChecksumCalls++;

			uint32_t calculatedChecksum = calculatedChecksumFromArg;
			uint32_t reversedChecksum = ReverseBytes(calculatedChecksumFromArg);
			uint32_t* calculatedChecksumPtr = (uint32_t*)((char*)ptrStack + 0x120); // 0x120 is a good starting point to decrement downwards to find the calculated checksum on the stack
			uint32_t* calculatedReversedChecksumPtr = (uint32_t*)((char*)ptrStack + 0x120); // 0x120 is a good starting point to decrement downwards to find the calculated checksum on the stack

			bool doubleTextChecksum = false;
			uint64_t* previousResultPtr = nullptr;
			if (ptrOffset == 0 && rbpOffset < 0x90) {
				uint64_t* textPtr = (uint64_t*)((char*)ptrStack + rbpOffset + (rbpOffset % 0x8)); // make sure rbpOffset is aligned by 8 bytes
				int pointerCounter = 0;

				for (int i = 0; i < 20; i++) {
					uint64_t derefPtr = *(uint64_t*)textPtr;

					if (derefPtr >= baseAddressStart && derefPtr <= baseAddressEnd) {
						uint64_t derefResult = **(uint64_t**)textPtr;
						pointerCounter++;

						// store the ptr above 0xffffffffffffffff and then use it in our originalchecksum check
						if (derefResult == 0xffffffffffffffff) {
							if (pointerCounter > 2) {
								doubleTextChecksum = true;

								// because textptr will be pointing at 0xffffffffffffffff, increment it once 
								// so we are pointing to the correct checksum location

								// TODO: remove this, doesnt do anything, confirm with checksum 0x79d397c8
								// since we use previousResultPtr which doesnt rely on this
								textPtr++;
							}

							break;
						}

						previousResultPtr = textPtr;
					}

					textPtr--;
				}
			}
			else {
				// for debugging stack traces on bigger rbp offset checksums
				uint64_t* textPtr = (uint64_t*)((char*)ptrStack + rbpOffset + (rbpOffset % 0x8)); // make sure rbpOffset is aligned by 8 bytes

				for (int i = 0; i < 30; i++) {
					uint64_t derefPtr = *(uint64_t*)textPtr;

					if (derefPtr >= baseAddressStart && derefPtr <= baseAddressEnd) {
						uint64_t derefResult = **(uint64_t**)textPtr;
					}

					textPtr--;
				}
			}

			// find calculatedChecksumPtr, we will overwrite this later with the original checksum
			for (int i = 0; i < 80; i++) {
				uint32_t derefPtr = *(uint32_t*)calculatedChecksumPtr;

				if (derefPtr == calculatedChecksum) {
					break;
				}

				calculatedChecksumPtr--;
			}

			// find calculatedReversedChecksumPtr, we will overwrite this later with the original checksum
			for (int i = 0; i < 80; i++) {
				uint32_t derefPtr = *(uint32_t*)calculatedReversedChecksumPtr;

				if (derefPtr == reversedChecksum) {
					break;
				}

				calculatedReversedChecksumPtr--;
			}

			uint64_t* textPtr = (uint64_t*)((char*)ptrStack + rbpOffset + (rbpOffset % 0x8)); // add remainder to align ptr
			uint32_t originalChecksum = NULL;
			uint32_t* originalChecksumPtr = nullptr;

			// searching for a .text pointer that points to the original checksum, upwards from the rbp	
			for (int i = 0; i < 10; i++) {
				uint64_t derefPtr = *(uint64_t*)textPtr;

				if (derefPtr >= baseAddressStart && derefPtr <= baseAddressEnd) {
					if (ptrOffset == 0 && rbpOffset < 0x90) {
						if (doubleTextChecksum) {
							originalChecksum = **(uint32_t**)previousResultPtr;
						}
						else {
							originalChecksum = *(uint32_t*)derefPtr;
						}
					}
					else {
						originalChecksum = *(uint32_t*)((char*)derefPtr + ptrOffset * 4); // if ptrOffset is used the original checksum is in a different spot
						originalChecksumPtr = (uint32_t*)((char*)derefPtr + ptrOffset * 4);
					}

					break;
				}

				textPtr--;
			}

			*calculatedChecksumPtr = (uint32_t)originalChecksum;
			*calculatedReversedChecksumPtr = ReverseBytes((uint32_t)originalChecksum);

			// for big intact we need to keep overwriting 4 more times
			// seems to still run even if we comment this out wtf?
			uint32_t* tmpOriginalChecksumPtr = originalChecksumPtr;
			uint32_t* tmpCalculatedChecksumPtr = calculatedChecksumPtr;
			uint32_t* tmpReversedChecksumPtr = calculatedReversedChecksumPtr;
			if (originalChecksumPtr != nullptr)
			{
				for (int i = 0; i <= ptrOffset; i++)
				{
					*tmpCalculatedChecksumPtr = *(uint32_t*)tmpOriginalChecksumPtr;
					*tmpReversedChecksumPtr = ReverseBytes(*(uint32_t*)tmpOriginalChecksumPtr);

					tmpOriginalChecksumPtr--;
					tmpCalculatedChecksumPtr--;
					tmpReversedChecksumPtr--;
				}
			}

			return originalChecksum;
		}

		bool ArxanHealingChecksum(uint64_t rbp) {
			// check if rbpAddressLocationPtr is within the range of 8 bytes up & down from every checksum that we placed.
			uint64_t rbpAddressLocationPtr = *(uint64_t*)(rbp + 0x10);

			for (int i = 0; i < s_InlineStubs.size(); i++) {
				// 0x8
				// TODO: if 0x7 is too big then "mov [rdx], al" will make the game crash probably because its trying to overwrite areas next to our hooks that have to get modified.
				// we could do two seperate functions since "mov [rdx], eax" would be a 32 byte offset (?) and "mov [rdx], al" would be 4 byte offset (?)

				if (rbpAddressLocationPtr + 0x7 >= (uint64_t)s_InlineStubs[i].m_FunctionAddress &&
					rbpAddressLocationPtr - 0x7 <= (uint64_t)s_InlineStubs[i].m_FunctionAddress)
				{
					return true;
				}
			}

			return false;
		}
	}

	void Utility::DisableTlsCallbacks() {
		Common::Utility::NT::Library game{};
		std::vector<Common::Utility::NT::Library::TlsCallback*> callbacks = game.GetTlsCallbacks();

		for (Common::Utility::NT::Library::TlsCallback* callback : callbacks) {
			if (MH_CreateHook(callback, &GeneralTlsCallbackStub, nullptr) != MH_OK) {
				LOG("Arxan/DisableTlsCalbacks", ERROR, "Failed to create hook @ 0x{:016X} (0x{:016X})", PTR_AS(std::uintptr_t, callback),
					PTR_AS(std::uintptr_t, callback) - PTR_AS(std::uintptr_t, game.GetPtr()) + 0x140000000);
				continue;
			}

			if (MH_EnableHook(callback) != MH_OK) {
				LOG("Arxan/DisableTlsCalbacks", ERROR, "Failed to enable hook @ 0x{:016X} (0x{:016X})", PTR_AS(std::uintptr_t, callback),
					PTR_AS(std::uintptr_t, callback) - PTR_AS(std::uintptr_t, game.GetPtr()) + 0x140000000);
			}
		}

		LOG("Arxan/DisableTlsCallbacks", INFO, "Disabled TLS callbacks.");
	}

	void Utility::DisableKiUserApcDispatcherHook() {
		void* procAddr = Common::Utility::NT::Library("ntdll.dll").GetProc<void*>("KiUserApcDispatcher");
		if (procAddr == nullptr) {
			LOG("Arxan/DisableKiUserApcDispatcherHook", ERROR, "Couldn't find KiUserApcDispatcher.");
			return;
		}

		// TODO: Pull the bytes before Arxan decides to hook KiUserApcDispatcher, or we might cause crashes for other ntdll variants,
		//       however this works on Windows 11 24H2 so... good enough?
		uint8_t bytes[14] = {
			0x48, 0x8B, 0x4C, 0x24, 0x18,	// mov     rcx, [rsp+arg_10]
			0x48, 0x8B, 0xC1,				// mov     rax, rcx
			0x4C, 0x8B, 0xCC,				// mov     r9, rsp
			0x48, 0xC1, 0xF9				// sar     rcx,
		};

		DWORD oldProtect{};
		VirtualProtect(procAddr, sizeof(bytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(procAddr, bytes, sizeof(bytes));
		VirtualProtect(procAddr, sizeof(bytes), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), procAddr, sizeof(bytes));
	}

	void Utility::RestoreKernel32ThreadInitThunkFunction() {
		// cold war removes the function ptr from ntdll Kernel32ThreadInitThunkFunction to its own, redirecting createremotethread
		// does rdtsc checks which in turn makes it so that if the process is completely suspended, will crash on created threads

		void* rtlUserThreadStartAddr = Common::Utility::NT::Library("ntdll.dll").GetProc<void*>("RtlUserThreadStart");
		void* baseThreadInitThunkAddr = Common::Utility::NT::Library("ntdll.dll").GetProc<void*>("BaseThreadInitThunk");

		if (rtlUserThreadStartAddr == nullptr || baseThreadInitThunkAddr == nullptr) {
			if (rtlUserThreadStartAddr == nullptr) {
				LOG("Arxan/RestoreKernel32ThreadInitThunkFunction", ERROR, "Couldn't find RtlUserThreadStart.");
			}
			if (baseThreadInitThunkAddr == nullptr) {
				LOG("Arxan/RestoreKernel32ThreadInitThunkFunction", ERROR, "Couldn't find BaseThreadInitThunk.");
			}
			return;
		}

		PVOID rtlUserThreadStart = PTR_AS(PVOID, PTR_AS(DWORD64, rtlUserThreadStartAddr) + 0x7);
		DWORD64 rtlUserThreadStartFuncOffset = PTR_AS(DWORD64, PTR_AS(PUCHAR, rtlUserThreadStart)
			+ DEREF_PTR_AS(ULONG, PTR_AS(PUCHAR, rtlUserThreadStart) + 0x3) + 0x7);
		uint64_t* baseThreadInitPtr = PTR_AS(uint64_t*, rtlUserThreadStartFuncOffset);
		memcpy(baseThreadInitPtr, &baseThreadInitThunkAddr, sizeof(uint64_t));
	}

	void Utility::RemoveNtdllChecksumChecks() {
		void* baseModule = GetModuleHandle(nullptr);

		const size_t allocationSize = sizeof(uint8_t) * 0x100 * 1000;
		LPVOID healingStubLocation = AllocateSomewhereNear(GetModuleHandle(nullptr), allocationSize);
		memset(healingStubLocation, 0x90, allocationSize);

		// avoid stub generation collision
		char* previousStubOffset = nullptr;
		// for jmp distance calculation
		char* currentStubOffset = nullptr;

		size_t amountOfLocations = sizeof(s_NtdllDbgLocations) / sizeof(NtdllDbgLocation);
		for (uint64_t i = 0; i < amountOfLocations; i++) {
			void* functionAddress = (char*)s_NtdllDbgLocations[i].m_AddrLocation + (uint64_t)baseModule;

			// movzx   eax, byte ptr [rax]		0F B6 00
			// jmp     loc_7FF71B61F9FA			E9 80 EC D3 E3
			// 0F B6 00 E9 42 F5 BC E4 = 8 bytes
			const uint64_t instructionsInBytes = 0x8;

			int32_t jumpDistance = 0;
			uint64_t locationToJump;

			// we don't know the previous offset yet
			if (currentStubOffset == nullptr) {
				currentStubOffset = (char*)healingStubLocation;
			}

			if (previousStubOffset != nullptr) {
				currentStubOffset = previousStubOffset;
			}

			memcpy(&jumpDistance, (char*)functionAddress + 4, 4); // ptr after 0xE9
			locationToJump = jumpDistance + instructionsInBytes + (uint64_t)functionAddress;

			static asmjit::JitRuntime runtime;
			asmjit::CodeHolder code;
			code.init(runtime.environment());

			using namespace asmjit::x86;
			Assembler a(&code);

			asmjit::Label DEBUG = a.newLabel();

			a.movq(xmm15, rsp);
			pushad64();
			a.sub(rsp, 0x20);
			a.movq(rcx, xmm15);
			a.mov(rdx, i);
			a.call(ReplaceNtdllStackWithOurOwn);
			a.add(rsp, 0x20);
			popad64();
			a.push(rax);
			a.mov(rax, 0);
			a.movq(xmm15, rax);
			a.pop(rax);

			uint64_t patchedBuffer = (uint64_t)s_NtdllDbgLocations[i].m_PatchedByArxanBuffer;
			a.mov(rax, patchedBuffer);
			a.mov(rcx, patchedBuffer);
			a.movzx(eax, byte_ptr(rax));
			a.jmp(locationToJump);

			void* asmjitResult = nullptr;
			runtime.add(&asmjitResult, &code);

			// copy over the content to the stub
			uint8_t* tempBuffer = (uint8_t*)malloc(sizeof(uint8_t) * code.codeSize());
			memcpy(tempBuffer, asmjitResult, code.codeSize());
			memcpy(currentStubOffset, tempBuffer, sizeof(uint8_t) * code.codeSize());

			DWORD oldProtect{};
			size_t instructionLength = 0x8;

			VirtualProtect(functionAddress, instructionLength, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset(functionAddress, 0x90, instructionLength);
			VirtualProtect(functionAddress, instructionLength, oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), functionAddress, instructionLength);

			uint64_t jmpDistance = (uint64_t)currentStubOffset - (uint64_t)functionAddress - 5;
			uint8_t* jmpInstructionBuffer = (uint8_t*)malloc(sizeof(uint8_t) * instructionLength);
			memset(jmpInstructionBuffer, 0x90, instructionLength);

			// E8 cd CALL rel32  Call near, relative, displacement relative to next instruction
			jmpInstructionBuffer[0] = 0xE9;
			jmpInstructionBuffer[1] = (jmpDistance >> (0 * 8));
			jmpInstructionBuffer[2] = (jmpDistance >> (1 * 8));
			jmpInstructionBuffer[3] = (jmpDistance >> (2 * 8));
			jmpInstructionBuffer[4] = (jmpDistance >> (3 * 8));

			VirtualProtect(functionAddress, instructionLength, PAGE_EXECUTE_READWRITE, &oldProtect);
			memcpy(functionAddress, jmpInstructionBuffer, instructionLength);
			VirtualProtect(functionAddress, instructionLength, oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), functionAddress, instructionLength);

			previousStubOffset = currentStubOffset + sizeof(uint8_t) * code.codeSize() + 0x8;
		}

		LOG("Arxan/RemoveNtdllChecksumChecks", INFO, "Done.");
	}

	void Utility::DbgRemove() {
		bool forceDebuggedNum = false;
		bool setOneTime = false;
		while (true) {
			auto* const peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
			peb->BeingDebugged = 0x8F; // this could end up getting the game to crash cause of arxan

			NtDllRestore::RestoreDebugFunctions();
			Sleep(100);
		}
	}

	void Utility::CreateInlineAsmStub() {
		Common::Utility::NT::Library game{};

		auto locationsIntact = Memory::VectoredSigScan<void*>("89 04 8A 83 45 ? FF", game.GetName(), "Arxan Locations Intact");
		auto locationsIntactBig = Memory::VectoredSigScan<void*>("89 04 8A 83 85", game.GetName(), "Arxan Locations Intact Big");
		auto locationsSplit = Memory::VectoredSigScan<void*>("89 04 8A E9", game.GetName(), "Arxan Locations Split");

		uint64_t baseAddr = reinterpret_cast<uint64_t>(game.GetPtr());

		size_t intactCount = locationsIntact.size();
		size_t intactBigCount = locationsIntactBig.size();
		size_t splitCount = locationsSplit.size();
		size_t totalCount = intactCount + intactBigCount + splitCount;

		const size_t allocationSize = sizeof(uint8_t) * 128;
		s_InlineStubs.clear();

		for (int i = 0; i < intactCount; i++) {
			s_InlineStubs.push_back(InlineAsmStub{ locationsIntact.at(i).As<void*>(), nullptr, 7, IntactSmall });
		}

		for (int i = 0; i < intactBigCount; i++) {
			s_InlineStubs.push_back(InlineAsmStub{ locationsIntactBig.at(i).As<void*>(), nullptr, 10, IntactBig });
		}

		for (int i = 0; i < splitCount; i++) {
			s_InlineStubs.push_back(InlineAsmStub{ locationsSplit.at(i).As<void*>(), nullptr, 8, Split });
		}

		LPVOID asmBigStubLocation = AllocateSomewhereNear(game.GetPtr(), allocationSize * 0x80);
		memset(asmBigStubLocation, 0x90, allocationSize * 0x80);

		// avoid stub generation collision
		char* previousStubOffset = nullptr;
		// for jmp distance calculation
		char* currentStubOffset = nullptr;

		// TODO: once we are done with that merge all the checksum fix stub generators into one function
		// make that also use one big allocated memory page

		// TODO: fix the asm stub that requires a movzx, registers maybe are getting owned?

		for (int i = 0; i < s_InlineStubs.size(); i++) {
			// we don't know the previous offset yet
			if (currentStubOffset == nullptr) {
				currentStubOffset = (char*)asmBigStubLocation;
			}

			if (previousStubOffset != nullptr) {
				currentStubOffset = previousStubOffset;
			}

			void* functionAddress = s_InlineStubs[i].m_FunctionAddress;
			uint64_t jmpDistance = (uint64_t)currentStubOffset - (uint64_t)functionAddress - 5; // 5 bytes from relative call instruction

			// backup instructions that will get destroyed
			const int length = sizeof(uint8_t) * 8;
			uint8_t instructionBuffer[8] = {};
			memcpy(instructionBuffer, functionAddress, length);

			uint32_t instructionBufferJmpDistance = 0;
			if (instructionBuffer[3] == 0xE9) {
				memcpy(&instructionBufferJmpDistance, (char*)functionAddress + 0x4, 4); // 0x4 so we skip 0xE9
			}

			uint64_t rbpOffset = 0x0;
			bool jumpDistanceNegative = instructionBufferJmpDistance >> 31; // get sign bit from jump distance
			int32_t jumpDistance = instructionBufferJmpDistance;

			if (s_InlineStubs[i].m_Type == Split) {
				// TODO: receive the rbpOffset by going through the jmp instruction
				// on big rbp offsets we could do the same hack we did on big intact where we do rbpOffset+0x100 if its below 0x60
				char* rbpOffsetPtr = nullptr;

				// TODO: just use jumpDistance once we got a working test case
				if (jumpDistanceNegative) {
					rbpOffsetPtr = (char*)((uint64_t)functionAddress + jumpDistance + 0x8);
				}
				else {
					rbpOffsetPtr = (char*)((uint64_t)functionAddress + instructionBufferJmpDistance + 0x8);
				}

				rbpOffsetPtr++;

				// depending on the rbp offset from add dword ptr we need one more byte for the rbpOffset
				if (*(unsigned char*)rbpOffsetPtr == 0x45) {		// add dword ptr [rbp+68],-01
					rbpOffsetPtr++;
					rbpOffset = *(char*)rbpOffsetPtr;
				}
				else if (*(unsigned char*)rbpOffsetPtr == 0x85)	{	// add dword ptr [rbp+1CC],-01
					rbpOffsetPtr++;
					rbpOffset = *(short*)rbpOffsetPtr;
				}
			}

			// create assembly stub content
			// TODO: we could create three different asmjit build sections for each type
			// so we don't have if statements inbetween instructions for the cost of LOC but it would be more readible
			static asmjit::JitRuntime runtime;
			asmjit::CodeHolder code;
			code.init(runtime.environment());
			asmjit::x86::Assembler a(&code);

			if (s_InlineStubs[i].m_Type != Split) {
				rbpOffset = instructionBuffer[5];
			}

			a.sub(asmjit::x86::rsp, 0x32);
			pushad64();

			a.mov(asmjit::x86::qword_ptr(asmjit::x86::rsp, 0x20), asmjit::x86::rax);
			a.mov(asmjit::x86::rdx, asmjit::x86::rcx);	// offset within text section pointer (ecx*4)

			// we dont use rbpoffset since we only get 1 byte from the 2 byte offset (rbpOffset)
			// 0x130 is a good starting ptr to decrement downwards so we can find the original checksum
			if (s_InlineStubs[i].m_Type == IntactBig) {
				a.mov(asmjit::x86::rcx, 0x120);
			}
			else {
				a.mov(asmjit::x86::rcx, rbpOffset);
			}

			a.mov(asmjit::x86::r8, asmjit::x86::rbp);

			if (s_InlineStubs[i].m_Type == Split) {
				if (jumpDistanceNegative) {
					a.mov(asmjit::x86::r9, jumpDistance);
				}
				else {
					a.mov(asmjit::x86::r9, instructionBufferJmpDistance);
				}
			}
			else {
				a.mov(asmjit::x86::r9, instructionBufferJmpDistance); // incase we mess up a split checksum
			}

			a.mov(asmjit::x86::rax, (uint64_t)(void*)FixChecksum);
			a.call(asmjit::x86::rax);
			a.add(asmjit::x86::rsp, 0x8 * 4); // so that r12-r15 registers dont get corrupt

			popad64WithoutRAX();
			a.add(asmjit::x86::rsp, 0x32);

			a.mov(ptr(asmjit::x86::rdx, asmjit::x86::rcx, 2), asmjit::x86::eax); // mov [rdx+rcx*4], eax

			if (instructionBufferJmpDistance == 0) {
				if (s_InlineStubs[i].m_Type == IntactBig) {
					rbpOffset += 0x100;
				}

				a.add(dword_ptr(asmjit::x86::rbp, rbpOffset), -1); // add dword ptr [rbp+rbpOffset], 0FFFFFFFFh
			}
			else {
				// jmp loc_7FF641C707A5
				// push the desired address on to the stack and then perform a 64 bit RET
				a.add(asmjit::x86::rsp, 0x8); // pop return address off the stack cause we will jump
				uint64_t addressToJump = (uint64_t)functionAddress + instructionBufferJmpDistance;

				if (s_InlineStubs[i].m_Type == Split) {
					// TODO: just use jumpDistance once we got a working test case
					if (jumpDistanceNegative) {
						addressToJump = (uint64_t)functionAddress + jumpDistance + 0x8; // 0x8 call instruction + offset + 2 nops
					}
					else {
						addressToJump = (uint64_t)functionAddress + instructionBufferJmpDistance + 0x8; // 0x8 call instruction + offset + 2 nops
					}
				}

				a.mov(asmjit::x86::r11, addressToJump);	// r11 is being used but should be fine based on documentation

				if (s_InlineStubs[i].m_Type == Split) {
					a.add(asmjit::x86::rsp, 0x8); // since we dont pop off rax we need to sub 0x8 the rsp
				}

				a.push(asmjit::x86::r11);
			}

			if (s_InlineStubs[i].m_Type != Split) {
				a.add(asmjit::x86::rsp, 0x8); // since we dont pop off rax we need to sub 0x8 the rsp
			}

			a.ret();

			void* asmjitResult = nullptr;
			runtime.add(&asmjitResult, &code);

			// copy over the content to the stub
			uint8_t* tempBuffer = (uint8_t*)malloc(sizeof(uint8_t) * code.codeSize());
			memcpy(tempBuffer, asmjitResult, code.codeSize());
			memcpy(currentStubOffset, tempBuffer, sizeof(uint8_t) * code.codeSize());

			size_t callInstructionBytes = s_InlineStubs[i].m_BufferSize;
			size_t callInstructionLength = sizeof(uint8_t) * callInstructionBytes;

			DWORD old_protect{};
			VirtualProtect(functionAddress, callInstructionLength, PAGE_EXECUTE_READWRITE, &old_protect);
			memset(functionAddress, 0, callInstructionLength);
			VirtualProtect(functionAddress, callInstructionLength, old_protect, &old_protect);
			FlushInstructionCache(GetCurrentProcess(), functionAddress, callInstructionLength);

			// E8 cd CALL rel32  Call near, relative, displacement relative to next instruction
			uint8_t* jmpInstructionBuffer = (uint8_t*)malloc(sizeof(uint8_t) * callInstructionBytes);
			jmpInstructionBuffer[0] = 0xE8;
			jmpInstructionBuffer[1] = (jmpDistance >> (0 * 8));
			jmpInstructionBuffer[2] = (jmpDistance >> (1 * 8));
			jmpInstructionBuffer[3] = (jmpDistance >> (2 * 8));
			jmpInstructionBuffer[4] = (jmpDistance >> (3 * 8));
			jmpInstructionBuffer[5] = 0x90;
			jmpInstructionBuffer[6] = 0x90;

			if (s_InlineStubs[i].m_Type == IntactBig) {
				jmpInstructionBuffer[7] = 0x90;
				jmpInstructionBuffer[8] = 0x90;
				jmpInstructionBuffer[9] = 0x90;
			}

			if (s_InlineStubs[i].m_Type == Split) {
				jmpInstructionBuffer[7] = 0x90;
			}

			VirtualProtect(functionAddress, callInstructionLength, PAGE_EXECUTE_READWRITE, &old_protect);
			memcpy(functionAddress, jmpInstructionBuffer, callInstructionLength);
			VirtualProtect(functionAddress, callInstructionLength, old_protect, &old_protect);
			FlushInstructionCache(GetCurrentProcess(), functionAddress, callInstructionLength);

			// store location & bytes to check if arxan is removing our hooks
			if (s_InlineStubs[i].m_Type == IntactSmall) {
				IntactChecksumHook intactChecksum = {};
				intactChecksum.m_FunctionAddress = (uint64_t*)functionAddress;
				memcpy(intactChecksum.m_Buffer, jmpInstructionBuffer, sizeof(uint8_t) * s_InlineStubs[i].m_BufferSize);
				s_InlineStubs[i].m_Buffer = intactChecksum.m_Buffer;
				s_IntactChecksumHooks.push_back(intactChecksum);
			}

			if (s_InlineStubs[i].m_Type == IntactBig) {
				IntactBigChecksumHook intactBigChecksum = {};
				intactBigChecksum.m_FunctionAddress = (uint64_t*)functionAddress;
				memcpy(intactBigChecksum.m_Buffer, jmpInstructionBuffer, sizeof(uint8_t) * s_InlineStubs[i].m_BufferSize);
				s_InlineStubs[i].m_Buffer = intactBigChecksum.m_Buffer;
				s_IntactBigChecksumHooks.push_back(intactBigChecksum);
			}

			if (s_InlineStubs[i].m_Type == Split) {
				SplitChecksumHook splitChecksum = {};
				splitChecksum.m_FunctionAddress = (uint64_t*)functionAddress;
				memcpy(splitChecksum.m_Buffer, jmpInstructionBuffer, sizeof(uint8_t) * s_InlineStubs[i].m_BufferSize);
				s_InlineStubs[i].m_Buffer = splitChecksum.m_Buffer;
				s_SplitChecksumHooks.push_back(splitChecksum);
			}

			previousStubOffset = currentStubOffset + sizeof(uint8_t) * code.codeSize() + 0x8;
		}

		LOG("Arxan/CreateInlineAsmStub", DEBUG, "Intact checksum count: {}", intactCount);
		LOG("Arxan/CreateInlineAsmStub", DEBUG, "Intact big checksum count: {}", intactBigCount);
		LOG("Arxan/CreateInlineAsmStub", DEBUG, "Split checksum count: {}", splitCount);
	}

	void Utility::CreateChecksumHealingStub() {
		void* baseModule = GetModuleHandle(nullptr);
		Common::Utility::NT::Library game{};

		ChecksumHealingLocation healingLocations[]{
			{ Memory::VectoredSigScan<void*>("89 02 8B 45 20", game.GetName(), "Arxan Healing Locations [1]"), 5 },
			{ Memory::VectoredSigScan<void*>("88 02 83 45 20 FF", game.GetName(), "Arxan Healing Locations [2]"), 6 },
			{ Memory::VectoredSigScan<void*>("89 02 E9", game.GetName(), "Arxan Healing Locations [3]"), 7 },
			{ Memory::VectoredSigScan<void*>("88 02 E9", game.GetName(), "Arxan Healing Locations [4]"), 7 },
		};

		const size_t allocationSize = sizeof(uint8_t) * 0x100 * 1000;
		LPVOID healingStubLocation = AllocateSomewhereNear(GetModuleHandle(nullptr), allocationSize);
		memset(healingStubLocation, 0x90, allocationSize);

		// avoid stub generation collision
		char* previousStubOffset = nullptr;
		// for jmp distance calculation
		char* currentStubOffset = nullptr;

		size_t amountOfPatterns = sizeof(healingLocations) / sizeof(ChecksumHealingLocation);
		for (int type = 0; type < amountOfPatterns; type++) {
			size_t locations = healingLocations[type].m_ChecksumPattern.size();
			for (int i = 0; i < locations; i++) {
				uint8_t instructionBuffer[4] = {}; // 88 02 E9: 4          
				int32_t jumpDistance = 0;
				size_t callInstructionOffset = 5; // 0xE8 ? ? ? ?
				uint64_t jumpInstruction;
				uint64_t locationToJump;

				// we don't know the previous offset yet
				if (currentStubOffset == nullptr) {
					currentStubOffset = (char*)healingStubLocation;
				}

				if (previousStubOffset != nullptr) {
					currentStubOffset = previousStubOffset;
				}

				void* functionAddress = healingLocations[type].m_ChecksumPattern.at(i).As<void*>();

				if (*(uint8_t*)((uint8_t*)functionAddress + 2) == 0xE9) {
					memcpy(&jumpDistance, (char*)functionAddress + 3, 4); // ptr after 0xE9
					jumpInstruction = (uint64_t)functionAddress + 2; 		// at the jmp instruction
					locationToJump = jumpInstruction + jumpDistance + callInstructionOffset;

					// get size of image from codcw
					uint64_t baseAddressStart = (uint64_t)GetModuleHandle(nullptr);
					IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
					IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);
					auto sizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
					uint64_t baseAddressEnd = baseAddressStart + sizeOfImage;

					if ((locationToJump > baseAddressStart && locationToJump < baseAddressEnd) != true) {
						continue;
					}

					memcpy(instructionBuffer, (char*)locationToJump, sizeof(uint8_t) * 4);

					if (type == 2) {
						uint8_t instruction[3] = { 0x8B, 0x45, 0x20 };
						if (memcmp(instructionBuffer, instruction, sizeof(uint8_t) * 3) != 0) {
							continue;
						}
					}

					if (type == 3) {
						uint8_t instruction[4] = { 0x83, 0x45, 0x20, 0xFF };
						if (memcmp(instructionBuffer, instruction, sizeof(uint8_t) * 4) != 0) {
							continue;
						}
					}
				}

				static asmjit::JitRuntime runtime;
				asmjit::CodeHolder code;
				code.init(runtime.environment());

				using namespace asmjit::x86;
				Assembler a(&code);
				asmjit::Label L1 = a.newLabel();
				asmjit::Label DEBUG = a.newLabel();

				a.sub(rsp, 0x32);
				pushad64_Min();

				a.mov(rcx, rbp);
				a.mov(r15, (uint64_t)(void*)ArxanHealingChecksum);
				a.call(r15);
				a.movzx(r15, al);	// if arxan tries to replace our checksum set r15 to 1

				popad64_Min();
				a.add(rsp, 0x32);

				switch (type) {
				case 0:
					/*
						mov     [rdx], eax
						mov     eax, [rbp+20h]
					*/
					// dont replace our checksum if r15 is 1
					a.cmp(r15, 1);
					a.je(L1);
					a.mov(qword_ptr(rdx), eax);

					a.bind(L1);
					a.mov(eax, qword_ptr(rbp, 0x20));
					a.ret();
					break;
				case 1:
					/*
						mov     [rdx], al
						add     dword ptr [rbp+20h], -1
					*/
					// dont replace our checksum if r15 is 1
					a.cmp(r15, 1);
					a.je(L1);
					a.mov(qword_ptr(rdx), al);

					a.bind(L1);
					a.add(dword_ptr(rbp, 0x20), -1);
					a.ret();
					break;
				case 2:
					/*
						mov     [rdx], eax
						jmp     loc_7FF7366C7B94
					*/
					// dont replace our checksum if r15 is 1
					a.cmp(r15, 1);
					a.je(L1);
					a.mov(qword_ptr(rdx), eax);

					a.bind(L1);
					a.add(rsp, 0x8);
					a.mov(r15, locationToJump);
					a.push(r15);
					a.ret();
					break;
				case 3:
					/*
						mov     [rdx], al
						jmp     loc_7FF738FB7A45
					*/
					// dont replace our checksum if r15 is 1
					a.cmp(r15, 1);
					a.je(L1);
					a.mov(qword_ptr(rdx), al);

					a.bind(L1);
					a.add(rsp, 0x8);
					a.mov(r15, locationToJump);
					a.push(r15);
					a.ret();
					break;
				default:
					LOG("Arxan/CreateChecksumHealingStub", ERROR, "We shouldn't be here.");
					getchar();
					abort();
				}

				void* asmjitResult = nullptr;
				runtime.add(&asmjitResult, &code);

				// copy over the content to the stub
				uint8_t* tempBuffer = (uint8_t*)malloc(sizeof(uint8_t) * code.codeSize());
				memcpy(tempBuffer, asmjitResult, code.codeSize());
				memcpy(currentStubOffset, tempBuffer, sizeof(uint8_t) * code.codeSize());

				size_t callInstructionBytes = healingLocations[type].m_Length;
				size_t callInstructionLength = sizeof(uint8_t) * callInstructionBytes;

				DWORD old_protect{};
				VirtualProtect(functionAddress, callInstructionLength, PAGE_EXECUTE_READWRITE, &old_protect);
				memset(functionAddress, 0, callInstructionLength);
				VirtualProtect(functionAddress, callInstructionLength, old_protect, &old_protect);
				FlushInstructionCache(GetCurrentProcess(), functionAddress, callInstructionLength);

				uint64_t jmpDistance = (uint64_t)currentStubOffset - (uint64_t)functionAddress - 5;
				uint8_t* jmpInstructionBuffer = (uint8_t*)malloc(sizeof(uint8_t) * callInstructionBytes);

				// E8 cd CALL rel32  Call near, relative, displacement relative to next instruction
				jmpInstructionBuffer[0] = 0xE8;
				jmpInstructionBuffer[1] = (jmpDistance >> (0 * 8));
				jmpInstructionBuffer[2] = (jmpDistance >> (1 * 8));
				jmpInstructionBuffer[3] = (jmpDistance >> (2 * 8));
				jmpInstructionBuffer[4] = (jmpDistance >> (3 * 8));

				for (int v = 0; v < callInstructionBytes - 5; v++) {
					jmpInstructionBuffer[5 + v] = 0x90;
				}

				VirtualProtect(functionAddress, callInstructionLength, PAGE_EXECUTE_READWRITE, &old_protect);
				memcpy(functionAddress, jmpInstructionBuffer, callInstructionLength);
				VirtualProtect(functionAddress, callInstructionLength, old_protect, &old_protect);
				FlushInstructionCache(GetCurrentProcess(), functionAddress, callInstructionLength);

				previousStubOffset = currentStubOffset + sizeof(uint8_t) * code.codeSize() + 0x8;

				// debugging printf
				if (i == 0) {
					LOG("Arxan/CreateChecksumHealingStub", DEBUG, "Type {} @ 0x{:016X} (0x{:016X})", type, PTR_AS(std::uintptr_t, functionAddress),
						PTR_AS(std::uintptr_t, functionAddress) - PTR_AS(std::uintptr_t, game.GetPtr()) + 0x140000000);
				}
			}
		}
	}
}
