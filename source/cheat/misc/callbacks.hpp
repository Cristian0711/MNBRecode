#pragma once

class callbacks {
private:
	callbacks() {};

public:
	enum class callback_type {
		render = 1,
		every_frame,
		_script_unload,
	};

	std::unordered_map<callback_type, std::vector<std::function<void()>>> m_callbacks;

	[[nodiscard]] static auto get() -> callbacks& {
		static callbacks instance;
		return instance;
	}

	auto call_callbacks(callback_type type) const noexcept -> void{
		if (m_callbacks.empty() == true)
			return;

		for (const auto& [k, v] : m_callbacks)
			for (const auto& p : v)
				if (k == type) {
					std::invoke(p);
				}
	}

	[[nodiscard]] auto get_callbacks(callback_type type) noexcept {
		if (this->m_callbacks.contains(type)) return this->m_callbacks[type];
		else return std::vector<std::function<void()>>{};
	}

	auto add_callback(callback_type type, std::function<void()> handler) noexcept {
		if (this->m_callbacks.contains(type)) this->m_callbacks[type].push_back(handler);
		else this->m_callbacks.insert({ type, { handler } });
	}
};