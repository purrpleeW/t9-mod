#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
BOOL Client::Arxan::SysHooks::HK_CheckRemoteDebuggerPresent::hkCallback(HANDLE hProcess, PBOOL pbDebuggerPresent) {
	_Unreferenced_parameter_(hProcess);

	if (pbDebuggerPresent != nullptr) {
		*pbDebuggerPresent = false;
	}

	return true;
}
