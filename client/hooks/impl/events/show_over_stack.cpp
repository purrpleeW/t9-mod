#include "common.hpp"
#include "game/game.hpp"
#include "hooks/hook.hpp"

void Client::Hook::Hooks::OnShowOverStack() {
	// do stuff
	g_Pointers->PatchAuth();

	static std::string watermarkText = std::format("t9-mod: " GIT_DESCRIBE " [v{}]", g_GameIdentifier.m_Version);

	T9::Font_s* font = *g_Pointers->m_unk_WatermarkFont;
	if (font) {
		float color[4] = { .666f, .666f, .666f, .666f };
		g_Pointers->m_CL_DrawTextPhysical(watermarkText.c_str(), 0x7FFFFFFF, font,
			4.f, 4.f + static_cast<float>(font->m_FontHeight), 1.f, 1.f, 0.f, color, 0, 0);
	}
}
