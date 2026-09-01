#include "common.hpp"
#include "hooks/hook.hpp"
#include <utility/nt.hpp>

template <>
HANDLE Client::Hook::Hooks::HK_CreateMutexExA::hkCallback(const LPSECURITY_ATTRIBUTES attributes, const LPCSTR name, const DWORD flags, const DWORD access) {
	if (name == "$ IDA trusted_idbs"s || name == "$ IDA registry mutex $"s) {
		return nullptr;
	}

	return m_Original(attributes, name, flags, access);
}
