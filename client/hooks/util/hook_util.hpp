#pragma once
#include "common.hpp"

namespace Client::Hook::Util {
	void* InitDbgHelp();
	LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* inf);
}
