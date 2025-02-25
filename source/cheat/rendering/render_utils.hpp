class render_utils {
private:
	render_utils() {};
public:
	[[nodiscard]] static auto get() -> render_utils& {
		static render_utils instance;
		return instance;
	}

	[[nodiscard]] auto get_screen_pos(const CVector& position, CVector& out) const noexcept -> bool {
		return reinterpret_cast<bool(*)(const CVector*, CVector*)>(0x71DAB0)(&position, &out);
	}

	auto draw_circle_filled(const CVector& vec_center, const float radius, const ImColor& color) const noexcept -> void {
		ImGui::GetBackgroundDrawList()->AddCircleFilled({ vec_center.x, vec_center.y }, radius, color, 128);
	}

	auto draw_string(const std::string_view text, CVector position, const float size, const ImColor& color, const bool centered) const noexcept -> void
	{
		const auto font = menu::get().font();

		ImVec2 vecTextSize = font->CalcTextSizeA(size, FLT_MAX, 0.f, text.data());

		if (centered)
			position.x -= vecTextSize.x / 2.f;

		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x + 1, position.y + 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x - 1, position.y - 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x + 1, position.y - 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x - 1, position.y + 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());

		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x + 1, position.y }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x - 1, position.y }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x, position.y - 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());
		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x, position.y + 1 }, ImGui::GetColorU32({ 0, 0, 0, 128 }), text.data());

		ImGui::GetBackgroundDrawList()->AddText(font, size, { position.x, position.y }, color, text.data());
	}
};