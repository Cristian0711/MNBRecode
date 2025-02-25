#include "samp.hpp"

class modules {
private:
	modules() {};

	static inline uint32_t m_samp_base{ 0u };
public:
	[[nodiscard]] static auto samp() noexcept -> samp_t& {
		static auto instance = samp_t(m_samp_base);
		return instance;
	}

	static auto init() noexcept -> bool {
		auto jmp_to = *reinterpret_cast<uint32_t*>(0x4579C7) + 0x4579CB;
		auto base = jmp_to - 0x9F210;

		if (base == 0u) {
			return false;
		}

		m_samp_base = base;
		return true;
	}

	static auto inited() noexcept -> bool {
		return m_samp_base;
	}
};