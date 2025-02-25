#include "game/types/syncdata.h"
#include "game/types/info.hpp"
#include "game/types/chat.hpp"
#include "game/types/localplayer.hpp"
#include "game/types/vehicles.hpp"
#include "game/types/dialog.hpp"
#include "game/types/textdraws.hpp"

// USED ONLY IN SAMP BASED FILES
inline uint32_t g_samp_module = 0x0;
inline samp_offsets_i* g_samp_offsets = nullptr;

class samp_t {
private:
	uint32_t m_base{ 0u };
	bool m_inited{ false };

	samp_info_t* m_info{ nullptr };
	samp_chat_t* m_chat{ nullptr };
	samp_dialog_t* m_dialog{ nullptr };
	samp_localplayer_t* m_local_player{ nullptr };
	samp_vehicles_t* m_vehicles{ nullptr };
	samp_textdraws_t* m_textdraws{ nullptr };

	uint32_t m_pools{ 0u };
	uint32_t m_player_pool{ 0u };

public:
	static constexpr uint32_t max_players = 1000u;

	samp_t(uint32_t base) noexcept
		: m_base(base) {
		g_samp_module = base;
		g_samp_offsets = get_offsets();
	};

	[[nodiscard]] inline auto get_base() const noexcept -> uint32_t {
		return m_base;
	}

	[[nodiscard]] inline auto get_offsets() const noexcept -> samp_offsets_i* {
		static auto* offsets = reinterpret_cast<samp_offsets_i*>(samp_offsets_r1::get());
		return offsets;
	}

	[[nodiscard]] inline auto inited() const noexcept -> bool {
		return m_inited;
	}

	[[nodiscard]] inline auto init() noexcept -> bool {
		m_info = *reinterpret_cast<samp_info_t**>(m_base + g_samp_offsets->info());
		if (m_info == nullptr) {
			return false;
		}

		m_chat = *reinterpret_cast<samp_chat_t**>(m_base + g_samp_offsets->chat());
		if (m_chat == nullptr) {
			return false;
		}

		m_dialog = *reinterpret_cast<samp_dialog_t**>(m_base + g_samp_offsets->dialog());
		if (m_dialog == nullptr) {
			return false;
		}

		m_pools = *reinterpret_cast<uint32_t*>(m_info + g_samp_offsets->pools());
		if (m_pools == 0u) {
			return false;
		}


		m_player_pool = *reinterpret_cast<uint32_t*>(m_pools + g_samp_offsets->player_pool_offset());
		if (m_pools == 0u) {
			return false;
		}

		m_local_player = *reinterpret_cast<samp_localplayer_t**>(m_player_pool + g_samp_offsets->local_player_offset());
		if (m_local_player == nullptr) {
			return false;
		}

		m_vehicles = *reinterpret_cast<samp_vehicles_t**>(m_pools + g_samp_offsets->vehicles_pool_offset());
		if (m_vehicles == nullptr) {
			return false;
		}

		m_textdraws = *reinterpret_cast<samp_textdraws_t**>(m_pools + g_samp_offsets->textdraws());
		if (m_textdraws == nullptr) {
			return false;
		}

		m_inited = true;
		return true;
	}

	[[nodiscard]] inline auto info() const noexcept -> samp_info_t* {
		return m_info;
	}

	[[nodiscard]] inline auto chat() const noexcept -> samp_chat_t* {
		return m_chat;
	}

	[[nodiscard]] inline auto dialog() const noexcept -> samp_dialog_t* {
		return m_dialog;
	}

	[[nodiscard]] inline auto local_player() const noexcept -> samp_localplayer_t* {
		return m_local_player;
	}

	[[nodiscard]] inline auto local_player_name() const noexcept -> std::string_view {
		return ((const char* (__thiscall*)(uint32_t, uint16_t))(m_base + 0x13CE0))(m_player_pool, *reinterpret_cast<uint16_t*>(m_player_pool + 0x4));
	}


	[[nodiscard]] inline auto vehicles() const noexcept -> samp_vehicles_t* {
		return m_vehicles;
	}

	[[nodiscard]] inline auto textdraws() const noexcept -> samp_textdraws_t* {
		return m_textdraws;
	}

	inline auto patch_ac() const noexcept -> void {

		if (auto addr = g_samp_offsets->ac_patch_1(); addr != 0x0)
			memory::get().write<byte>(0xC3, g_samp_module + addr);

		if (auto addr = g_samp_offsets->ac_patch_2(); addr != 0x0)
			memory::get().write_multiple("\xB8\x45\x00\x00\x00\xC2\x1C\x00", g_samp_module + addr, 8);

		if (auto addr = g_samp_offsets->ac_patch_3(); addr != 0x0)
			memory::get().write_multiple("\xB8\x45\x00\x00\x00\xC2\x1C\x00", g_samp_module + addr, 8);
	}
};