#include "common.hpp"
#include "hooks/hook.hpp"

template <>
void Client::Hook::Hooks::HK_BB_Alert::hkCallback(const char* type, const char* msg) {
	if (strstr(msg, "Sail 630 Nuclear Bug") != nullptr) {
		std::string msgPatched = "";
		msgPatched += "Hello from ^3t9-mod^7!\n";
		msgPatched += "\n";
		msgPatched += "This is ^1not^7 an error message.\n";
		msgPatched += "\n";
		msgPatched += "Anyway, welcome to Black Ops Cold War! LAN play doesn't work as of right now, so you're stuck on you're own.\n";
		msgPatched += "\n";
		msgPatched += "- ^1xifil\n";

		auto msgPtr = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(msg));

		memset(msgPtr, 0, 4096);
		memcpy(msgPtr, msgPatched.data(), msgPatched.size());
	}

	return m_Original(type, msg);
}