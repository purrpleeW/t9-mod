#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
int Client::Arxan::SysHooks::HK_GetClassNameA::hkCallback(HWND hWnd, LPSTR lpClassName, int nMaxCount) {
	_Unreferenced_parameter_(hWnd);
	_Unreferenced_parameter_(lpClassName);
	_Unreferenced_parameter_(nMaxCount);

	return 0;
}
