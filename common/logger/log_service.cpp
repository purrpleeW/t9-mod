#include "common_core.hpp"
#include "logger/log_service.hpp"
#include "utility/utility.hpp"

namespace Common {
	LogService::LogService() {
		AllocConsole();
		freopen_s((FILE**)(stdout), "CONOUT$", "w", stdout);
		SetConsoleTitleA("t6-mod");
		SetConsoleOutputCP(CP_UTF8);

		Utility::EnsureVTL();
	}

	LogService::~LogService() {
		fclose(stdout);
		FreeConsole();
	}

	Logger LogService::GetLogger(std::string name) {
		return Logger(name);
	}
}
