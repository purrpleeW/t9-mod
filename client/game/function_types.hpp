#pragma once
#include "common.hpp"
#include "engine/t9/Font_s.hpp"
#include "engine/t9/enums/LobbyNetworkMode.hpp"
#include "engine/t9/unknown/ContentManager.hpp"

namespace Client::Game::Functions {
	using BB_AlertT = void(const char* type, const char* msg);
	using CL_DisconnectT = void(int localClientNum, bool deactivateClient, const char* message);
	using CL_DrawTextPhysicalT = void(const char* text, int maxChars, T9::Font_s* font, float x, float y, float rotation, float xScale, float yScale, const float* color, int style, int padding);
	using Com_SessionMode_SetNetworkModeT = void(int mode);
	using Dvar_SetBoolFromSourceT = void(std::uintptr_t* dvar, bool value, int source);
	using Dvar_ShowOverStackT = void();
	using LiveUser_GetUserDataForControllerT = std::uintptr_t*(int controllerIndex);
	using LobbyBase_SetNetworkModeT = void(T9::LobbyNetworkMode networkMode);
	using Unk_SetScreenT = void(int screen, char a2);
	using Unk_SetUsernameT = void(std::uintptr_t* _this, const char* username);

	using RtlDispatchExceptionT = bool(PEXCEPTION_RECORD record, PCONTEXT ctx);
}
