#pragma once
#include "common.hpp"

#include <utility/nt.hpp>

#include <wchar.h>
#include <tchar.h>

namespace Client::Arxan::Utility {
	inline std::uint64_t GetNtdllSize() {
		static std::optional<std::uint64_t> ntdllSize = std::nullopt;

		if (!ntdllSize.has_value()) {
			TCHAR path[MAX_PATH] = { 0 };
			if (!GetWindowsDirectory(path, ARRAYSIZE(path))) {
				LOG("Arxan/GetNtdllSize", WARN, "Failed to get Windows directory.");
				return 0;
			}

			if (_tcscat_s(path, TEXT("\\System32\\ntdll.dll")) != 0) {
				LOG("Arxan/GetNtdllSize", WARN, "Failed to append ntdll to path.");
				return 0;
			}

			HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				LOG("Arxan/GetNtdllSize", WARN, "Failed to open ntdll.");
				return 0;
			}

			LARGE_INTEGER size;
			if (!GetFileSizeEx(hFile, &size)) {
				CloseHandle(hFile);
				LOG("Arxan/GetNtdllSize", WARN, "Failed to get size of ntdll.");
				return 0;
			}

			ntdllSize = 4096 * ceil(size.QuadPart / 4096.f);
			CloseHandle(hFile);
		}

		return ntdllSize.value_or(0);
	}

	void DisableTlsCallbacks();
	void DisableKiUserApcDispatcherHook();
	void RestoreKernel32ThreadInitThunkFunction();
	void RemoveNtdllChecksumChecks();
	void DbgRemove();
	void CreateInlineAsmStub();
	void CreateChecksumHealingStub();
}
