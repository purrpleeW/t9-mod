#include "common.hpp"
#include "arxan/sys_hooks.hpp"

#include <utility/nt.hpp>

template <>
BOOL Client::Arxan::SysHooks::HK_SetThreadContext::hkCallback(HANDLE thread, const CONTEXT* context) {
	static Common::Utility::NT::Library game{};
	Common::Utility::NT::Library source = Common::Utility::NT::Library::GetByAddress(_ReturnAddress());

	if (source == game && !g_Debugging) {
		return FALSE;
	}
	
	return m_Original(thread, context);
}
