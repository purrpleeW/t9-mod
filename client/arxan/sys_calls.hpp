#pragma once
#include "common.hpp"

#include <utility/nt.hpp>

namespace Client::Arxan {
	class SysCalls {
	private:
		std::uint32_t GetSysCall(void* procAddr) {
			return DEREF_PTR_AS(std::uint32_t, PTR_AS(std::uintptr_t, procAddr) + 4);
		}
	public:
		SysCalls() {
			Common::Utility::NT::Library ntdll("ntdll.dll");

			this->m_NtAllocateVirtualMemory = this->GetSysCall(ntdll.GetProc<void*>("NtAllocateVirtualMemory"));
			this->m_NtClose = this->GetSysCall(ntdll.GetProc<void*>("NtClose"));
			this->m_NtCreateDebugObject = this->GetSysCall(ntdll.GetProc<void*>("NtCreateDebugObject"));
			this->m_NtCreateFile = this->GetSysCall(ntdll.GetProc<void*>("NtCreateFile"));
			this->m_NtCreateThread = this->GetSysCall(ntdll.GetProc<void*>("NtCreateThread"));
			this->m_NtCreateThreadEx = this->GetSysCall(ntdll.GetProc<void*>("NtCreateThreadEx"));
			this->m_NtProtectVirtualMemory = this->GetSysCall(ntdll.GetProc<void*>("NtProtectVirtualMemory"));
			this->m_NtQueryInformationProcess = this->GetSysCall(ntdll.GetProc<void*>("NtQueryInformationProcess"));
			this->m_NtQueryInformationThread = this->GetSysCall(ntdll.GetProc<void*>("NtQueryInformationThread"));
			this->m_NtQueryObject = this->GetSysCall(ntdll.GetProc<void*>("NtQueryObject"));
			this->m_NtQuerySystemInformation = this->GetSysCall(ntdll.GetProc<void*>("NtQuerySystemInformation"));
			this->m_NtSetInformationThread = this->GetSysCall(ntdll.GetProc<void*>("NtSetInformationThread"));
		}

		std::uint32_t m_NtAllocateVirtualMemory{};
		std::uint32_t m_NtClose{};
		std::uint32_t m_NtCreateDebugObject{};
		std::uint32_t m_NtCreateFile{};
		std::uint32_t m_NtCreateThread{};
		std::uint32_t m_NtCreateThreadEx{};
		std::uint32_t m_NtProtectVirtualMemory{};
		std::uint32_t m_NtQueryInformationProcess{};
		std::uint32_t m_NtQueryInformationThread{};
		std::uint32_t m_NtQueryObject{};
		std::uint32_t m_NtQuerySystemInformation{};
		std::uint32_t m_NtSetInformationThread{};
	};

	inline SysCalls g_SysCalls{};
}
