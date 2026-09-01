#pragma once
#include "engine/engine_common.hpp"

namespace T9 {
	class Font_s {
	public:
		char m_Name[0x0010];	// 0x0000
		int m_FontHeight;		// 0x0010
	private:
		char pad_0014[0x000C];	// 0x0014
	};
	ENGINE_ASSERT_SZ(Font_s, 0x0020);
}
