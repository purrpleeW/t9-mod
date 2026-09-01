#pragma once
#include "memory/minhook.hpp"
#include <global/string_literal.hpp>

namespace Client::HookPlate {
	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct StdcallHook {
		using funcType = Type __stdcall(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct CdeclHook {
		using funcType = Type __cdecl(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct FastcallHook {
		using funcType = Type __fastcall(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename TypeDecl>
	struct FnHook {
		using funcType = TypeDecl;
		static funcType* hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	class EventHandlerStore {
	public:
		using EventHandlerCallback = void();

		class EventHandler {
		public:
			EventHandler(std::string name, std::uintptr_t identifier, std::uintptr_t* address, EventHandlerCallback* callback)
				: m_Name(name)
				, m_Identifier(identifier)
				, m_Address(address)
				, m_Callback(callback)
			{}

			std::string m_Name;
			std::uintptr_t m_Identifier;
			std::uintptr_t* m_Address;
			EventHandlerCallback* m_Callback;

			std::uintptr_t m_Original = NULL;

			bool IsActive() {
				return this->m_Original != NULL;
			}

			void Enable() {
				if (this->IsActive()) {
					return;
				}

				this->m_Original = *this->m_Address;
				*this->m_Address = this->m_Identifier;
			}

			void Disable() {
				if (!this->IsActive()) {
					return;
				}

				*this->m_Address = this->m_Original;
				this->m_Original = NULL;
			}
		};

		EventHandler* FindHandler(std::uintptr_t identifier) {
			auto it = std::find_if(this->m_Handlers.begin(), this->m_Handlers.end(), [identifier](const EventHandler& handler) {
				return handler.m_Identifier == identifier;
			});

			if (it != this->m_Handlers.end()) {
				return it._Ptr;
			}
			return nullptr;
		}

		template <typename T>
		void AddHandler(std::string name, std::uintptr_t identifier, T* address, EventHandlerCallback* callback) {
			EventHandler handler = EventHandler(name, identifier, reinterpret_cast<std::uintptr_t*>(address), callback);
			handler.Enable();
			this->m_Handlers.push_back(handler);
		}

		void RemoveHandler(std::uintptr_t identifier) {
			std::size_t idx = 0;
			for (const auto& handler : this->m_Handlers) {
				if (handler.m_Identifier == identifier) {
					this->m_Handlers.erase(this->m_Handlers.begin() + idx);
					break;
				}
				idx++;
			}
		}

		void EnableHandler(std::uintptr_t identifier) {
			EventHandler* handler = this->FindHandler(identifier);
			if (handler != nullptr) {
				handler->Enable();
			}
		}

		void DisableHandler(std::uintptr_t identifier) {
			EventHandler* handler = this->FindHandler(identifier);
			if (handler != nullptr) {
				handler->Disable();
			}
		}

		std::vector<EventHandler> m_Handlers{};
	};
}