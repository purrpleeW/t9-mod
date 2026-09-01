#pragma once
#include "engine/engine_common.hpp"

namespace T9 {
	class ContentPack {
	private:
		char pad_0000[0x0014];	// 0x0000
	public:
		int m_Var1;				// 0x0014
	private:
		char pad_0018[0x0030];	// 0x0018
	public:
		int m_Var2;				// 0x0048
		int m_Var3;				// 0x004C
	private:
		char pad_0050[0x0080];
	};
	ENGINE_ASSERT_SZ(ContentPack, 0x00D0);
}
