#include "common.hpp"
#include "hooks/hook.hpp"
#include "hooks/util/hook_util.hpp"

template <>
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Client::Hook::Hooks::HK_SetUnhandledExceptionFilter::hkCallback(LPTOP_LEVEL_EXCEPTION_FILTER filter) {
	_Unreferenced_parameter_(filter);
	return SetUnhandledExceptionFilter(Hook::Util::TopLevelExceptionFilter);
}
