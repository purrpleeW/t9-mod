#include "common.hpp"
#include <utility/nt.hpp>

NTSTATUS Client::NtContinue(PCONTEXT threadContext, BOOLEAN raiseAlert) {
	return Common::Utility::NT::Library("ntdll.dll")
		.Invoke<NTSTATUS>("NtContinue", threadContext, raiseAlert);
}

std::string Client::GetWindowsUsername() {
    char username[256];
    DWORD size = sizeof(username);

    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    else {
        return "Unknown Soldier";
    }
}

bool Client::RemoveEvilKeywordsFromString(const UNICODE_STRING& string) {
	static const std::wstring evilKeywords[] = {
		L"IDA", L"ida", L"HxD", L"cheatengine", L"Cheat Engine", L"x96dbg", L"x32dbg", L"x64dbg", L"Wireshark",
	};

	if (!string.Buffer || !string.Length) {
		return false;
	}

	const std::wstring_view path(string.Buffer, string.Length / sizeof(string.Buffer[0]));

	bool modified = false;
	for (const auto& keyword : evilKeywords) {
		while (true) {
			const auto pos = path.find(keyword);
			if (pos == std::wstring::npos) {
				break;
			}

			modified = true;

			for (size_t i = 0; i < keyword.size(); ++i) {
				string.Buffer[pos + i] = L'a';
			}
		}
	}

	return modified;
}

bool Client::RemoveEvilKeywordsFromString(wchar_t* str, const size_t length) {
	UNICODE_STRING unicodeString{};
	unicodeString.Buffer = str;
	unicodeString.Length = static_cast<uint16_t>(length);
	unicodeString.MaximumLength = unicodeString.Length;

	return RemoveEvilKeywordsFromString(unicodeString);
}

bool Client::RemoveEvilKeywordsFromString(char* str, const size_t length) {
	std::string_view strView(str, length);
	std::wstring wstr(strView.begin(), strView.end());

	if (!RemoveEvilKeywordsFromString(wstr.data(), wstr.size())) {
		return false;
	}

	const std::string regularStr(wstr.begin(), wstr.end());
	memcpy(str, regularStr.data(), length);

	return true;
}

std::string Client::AndRel(std::uintptr_t address) {
	Common::Utility::NT::Library assignedLib = Common::Utility::NT::Library::GetByAddress(reinterpret_cast<void*>(address));
	if (assignedLib) {
		return std::format("0x{:016X} ({}+0x{:016X})", address, assignedLib.GetName(), address - reinterpret_cast<std::uintptr_t>(assignedLib.GetPtr()));
	}
	return std::format("0x{:016X}", address);
}
