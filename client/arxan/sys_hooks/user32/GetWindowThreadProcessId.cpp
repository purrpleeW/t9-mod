#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
DWORD Client::Arxan::SysHooks::HK_GetWindowThreadProcessId::hkCallback(HWND hWnd, LPDWORD lpdwProcessId) {
	_Unreferenced_parameter_(hWnd);
	_Unreferenced_parameter_(lpdwProcessId);

	return 0;
}
