#pragma once
#include "common_core.hpp"

namespace Common {
	class GameLaunchType {
	public:
		enum {
			UNKNOWN,
			MP,
			SP,
			ZM,
			DEDICATED
		};

		static inline std::pair<int, std::string> GetLaunchInfo() {
			std::map<int, std::string> types = {
				{ DEDICATED, GetExecutable(DEDICATED) },
				{ MP, GetExecutable(MP) },
				{ SP, GetExecutable(SP) },
				{ ZM, GetExecutable(ZM) }
			};

			for (const auto& [type, module] : types) {
				if (GetModuleHandleA(module.c_str()) != nullptr) {
					return std::make_pair(type, module);
				}
			}

			return std::make_pair(UNKNOWN, "?");
		}

		static inline std::string GetExecutable(int type) {
			switch (type) {
			case DEDICATED:
			case MP:
			case SP:
			case ZM:
				return "BlackOpsColdWar.exe";
			}
			return "<unknown>";
		}

		static inline std::string GetDisplayName(int type) {
			switch (type) {
			case DEDICATED:
				return "Dedicated Server";
			case MP:
				return "Multiplayer";
			case SP:
				return "Singleplayer";
			case ZM:
				return "Zombies";
			}
			return "<unknown>";
		}
	};
}
