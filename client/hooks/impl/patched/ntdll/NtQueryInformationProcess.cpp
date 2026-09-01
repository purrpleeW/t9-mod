#include "common.hpp"
#include "hooks/hook.hpp"

template <>
NTSTATUS WINAPI Client::Hook::Hooks::HK_NtQueryInformationProcess::hkCallback(const HANDLE handle, const PROCESSINFOCLASS infoClass,
		const PVOID info, const ULONG infoLength, const PULONG retLength) {
	NTSTATUS status = m_Original(handle, infoClass, info, infoLength, retLength);

	if (NT_SUCCESS(status)) {
		if (infoClass == ProcessImageFileName || static_cast<int>(infoClass) == 43 /* ProcessImageFileNameWin32 */) {
			RemoveEvilKeywordsFromString(*static_cast<UNICODE_STRING*>(info));
		}
	}

	return status;
}
