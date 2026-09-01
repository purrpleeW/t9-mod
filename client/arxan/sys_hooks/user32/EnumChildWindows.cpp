#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
BOOL Client::Arxan::SysHooks::HK_EnumChildWindows::hkCallback(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam) {
	_Unreferenced_parameter_(hWndParent);
	_Unreferenced_parameter_(lpEnumFunc);
	_Unreferenced_parameter_(lParam);

	return FALSE;
}
