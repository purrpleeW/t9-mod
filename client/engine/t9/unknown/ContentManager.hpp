#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/unknown/ContentPack.hpp"

namespace T9 {
	class ContentManager {
	private:
		char pad_0000[0x0008];			// 0x0000
	public:
		int m_PackCount;				// 0x0008
	private:
		char pad_000C[0x0004];			// 0x000C
	public:
		ContentPack* m_ContentPacks;	// 0x0010
	};
	ENGINE_ASSERT_SZ(ContentManager, 0x0018);
}
