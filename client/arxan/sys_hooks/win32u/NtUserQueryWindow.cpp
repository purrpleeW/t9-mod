#include "common.hpp"
#include "arxan/sys_hooks.hpp"

#include <utility/nt.hpp>

template <>
HANDLE Client::Arxan::SysHooks::HK_NtUserQueryWindow::hkCallback(HWND hwnd, int windowInfo) {
	if (windowInfo == 0 /* WindowProcess */) {
		return NtCurrentTeb()->Reserved1[8]; // ClientId.UniqueProcess
	}

	if (windowInfo == 1 /* WindowThread */) {
		return NtCurrentTeb()->Reserved1[9]; // ClientId.UniqueThread
	}

	return m_Original(hwnd, windowInfo);
}
