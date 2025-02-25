#pragma once

class renderer {
	bool m_initialized = false;
	renderer() {};
public:
	[[nodiscard]] static auto get() noexcept -> renderer& {
		static renderer instance;
		return instance;
	}

	auto setup() noexcept -> void {
		ImGui::CreateContext();

		D3DDEVICE_CREATION_PARAMETERS creation_params;
		rendering::get().d3d_device()->GetCreationParameters(&creation_params);

		ImGui_ImplDX9_Init(rendering::get().d3d_device());
		ImGui_ImplWin32_Init(creation_params.hFocusWindow);

		menu::get().setup_style();
		m_initialized = true;
	}

	auto begin_frame() const noexcept -> void {
		if (m_initialized && ImGui::GetCurrentContext()) {
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	auto end_frame() const noexcept -> void {
		if (m_initialized && ImGui::GetCurrentContext()) {
			ImGui::EndFrame();
			ImGui::Render();

			if (const auto draw_data = ImGui::GetDrawData())
				ImGui_ImplDX9_RenderDrawData(draw_data);
		}
	}

	[[nodiscard]] auto is_initialized() const noexcept -> bool {
		return m_initialized;
	}

	template <typename Callable>
	bool render(void (Callable::* func)() const noexcept, const Callable* obj = nullptr) const noexcept {
		if (FrontEndMenuManager.m_bMenuActive)
			return false;

		if (m_initialized && ImGui::GetCurrentContext()) {

			std::invoke(func, obj);
			return true;
		}
		return false;
	}
};