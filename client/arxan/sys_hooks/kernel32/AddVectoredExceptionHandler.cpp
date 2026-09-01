#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
PVOID Client::Arxan::SysHooks::HK_AddVectoredExceptionHandler::hkCallback(ULONG first, PVECTORED_EXCEPTION_HANDLER handler) {
	PVOID res = m_Original(first, handler);
	g_VectoredExceptionHandlers.push_back(res);
	return res;
}
