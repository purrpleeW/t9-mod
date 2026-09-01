#include "common.hpp"
#include "hooks/hook.hpp"
#include "hooks/util/hook_util.hpp"
#include "memory/memory.hpp"

#include <utility/nt.hpp>

template <>
bool Client::Hook::Hooks::HK_RtlDispatchException::hkCallback(PEXCEPTION_RECORD record, PCONTEXT ctx) {
	static Common::Utility::NT::Library game{};
	static std::size_t* addrDvar_GetBool_cmp = nullptr;
	static bool searchedForAddr = false;

	if (!searchedForAddr) {
		Memory::SigScan("83 79 ? 01 8B 41 ? 48 89 5C 24 ?", game.GetName(), "Dvar_GetBool [cmp]").Apply(&addrDvar_GetBool_cmp);
		g_Hooks->PostArxanDetectionHooks();
		searchedForAddr = true;
	}

	if (record->ExceptionCode == STATUS_INVALID_HANDLE) {
		NtContinue(ctx, FALSE);
		return true;
	}

	if (record->ExceptionCode == STATUS_ACCESS_VIOLATION && game.IsAddressInRange(reinterpret_cast<std::size_t>(record->ExceptionAddress))) {
		if (addrDvar_GetBool_cmp != nullptr && ctx->Rip == reinterpret_cast<std::size_t>(addrDvar_GetBool_cmp)) {
			HookPlate::EventHandlerStore::EventHandler* handler = g_Hooks->m_EventHandlerStore.FindHandler(ctx->Rcx);

			if (handler != nullptr && handler->m_Original != NULL) {
				ctx->Rcx = handler->m_Original;
				handler->m_Callback();
			}
			else {
				EXCEPTION_POINTERS ptrs{ record, ctx };
				Hook::Util::TopLevelExceptionFilter(&ptrs);
			}

			NtContinue(ctx, FALSE);
			return true;
		}
	}

	return m_Original(record, ctx);
}