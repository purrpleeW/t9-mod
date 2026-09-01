#include "common.hpp"
#include "hooks/hook.hpp"
#include <utility/nt.hpp>

template <>
BOOL WINAPI Client::Hook::Hooks::HK_GetThreadContext::hkCallback(const HANDLE threadHandle, const LPCONTEXT context) {
	constexpr auto debugRegistersFlag = (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_AMD64);
	if (context && (context->ContextFlags & debugRegistersFlag)) {
		auto* source = _ReturnAddress();
		const auto game = Common::Utility::NT::Library{};
		const auto sourceModule = Common::Utility::NT::Library::GetByAddress(source);

		if (sourceModule == game) {
			context->ContextFlags &= ~debugRegistersFlag;
		}
	}

	return m_Original(threadHandle, context);
}
