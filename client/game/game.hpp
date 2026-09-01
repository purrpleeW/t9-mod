#pragma once
#include "common.hpp"
#include "game/function_types.hpp"
#include "memory/scanned_result.hpp"

namespace Client {
	namespace Game {
		class Pointers {
		public:
			explicit Pointers();

			Functions::BB_AlertT* m_BB_Alert{};
			Functions::CL_DisconnectT* m_CL_Disconnect{};
			Functions::CL_DrawTextPhysicalT* m_CL_DrawTextPhysical{};
			Functions::Com_SessionMode_SetNetworkModeT* m_Com_SessionMode_SetNetworkMode{};
			Functions::Dvar_SetBoolFromSourceT* m_Dvar_SetBoolFromSource{};
			Functions::Dvar_ShowOverStackT* m_Dvar_ShowOverStack{};
			Functions::LiveUser_GetUserDataForControllerT* m_LiveUser_GetUserDataForController{};
			Functions::LobbyBase_SetNetworkModeT* m_LobbyBase_SetNetworkMode{};
			Functions::Unk_SetScreenT* m_Unk_SetScreen{};
			Functions::Unk_SetUsernameT* m_Unk_SetUsername{};

			std::uintptr_t** m_Dvar_NoDW{};
			bool* m_Scr_Initialized{};
			T9::ContentManager** m_unk_ContentManager{};
			char* m_unk_Config_1{};
			char* m_unk_Config_2{};
			T9::Font_s** m_unk_WatermarkFont{};

			Functions::RtlDispatchExceptionT* m_RtlDispatchException{};

			void PatchAuth();
			void SetMode(int mode, int menu);
		};
	}
	inline std::unique_ptr<Game::Pointers> g_Pointers{};
}

#define MATERIAL_WHITE Client::g_Pointers->m_DB_FindXAssetHeader(Client::T9::XAssetType::ASSET_TYPE_MATERIAL, "white", true, -1).material
