namespace ImGui
{
	template <typename... Args>
	inline auto colored_tooltip(const ImVec4& color, const std::string_view text, Args&&... args) noexcept -> void {
		LOG("fnAddMessageToChat");

		ImGui::TextColored(color, "(?)");

		if (text.empty() == true)
			return;

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(std::vformat(text, std::make_format_args(args...)).c_str());
			ImGui::EndTooltip();
		}
	}

	inline auto vectored_tooltip(const ImVec4& color, const std::string_view text, std::unordered_set<std::string_view> data) noexcept -> void {
		LOG("fnAddMessageToChat");

		ImGui::TextColored(color, "(?)");

		if (text.empty() == true)
			return;

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(text.data());
			for (const auto& entry : data)
				ImGui::TextUnformatted(entry.data());
			ImGui::EndTooltip();
		}
	}

	template <typename... Args>
	inline auto Text2(const std::string_view text, Args&&... args) noexcept -> void {
		ImGui::Text(std::vformat(text, std::make_format_args(args...)).c_str());
	}

	inline auto Text2(const std::string_view text) noexcept -> void {
		ImGui::Text(text.data());
	}

	static auto Tab(const char* label, const char* itemText, bool selected, const ImVec2& size_arg, bool last_item = false, ImGuiButtonFlags flags = NULL) noexcept -> bool
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 text_size = CalcTextSize(itemText, NULL, true);

		ImVec2 pos = window->DC.CursorPos;
		if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
		ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		if (last_item)
			pos.y += 160;
		const ImRect bb(pos, pos + size);
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		if (g.ItemFlagsStack.back() & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		const ImU32 selected_col = GetColorU32(ImGuiCol_Tab, 0.5f);
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_TabActive : ImGuiCol_TabHovered);
		RenderNavHighlight(bb, id);

		if (selected)
			window->DrawList->AddRectFilled(bb.Min, { bb.Min.x + 3, bb.Max.y }, GetColorU32(ImGuiCol_Tab));
		else if (held || hovered)
			RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		if (selected) ImGui::PushStyleColor(ImGuiCol_Text, GetColorU32(ImGuiCol_Tab));
		RenderTextClipped({ bb.Min.x + style.FramePadding.x - 60, bb.Min.y + style.FramePadding.y + 5 }, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
		RenderTextClipped({ bb.Min.x + style.FramePadding.x + 40, bb.Min.y + style.FramePadding.y - 5 }, bb.Max, itemText, NULL, &text_size, style.ButtonTextAlign, &bb);
		if (selected) ImGui::PopStyleColor();

		return pressed;
	}

    inline ImVector<ImRect> s_group_panel_label_stack_;
#pragma region Fancy Group Panels
    static auto begin_group_panel(const char* name, const ImVec2& size) noexcept -> void
    {
        ImGui::BeginGroup();

        const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        const auto frame_height = ImGui::GetFrameHeight();
        ImGui::BeginGroup();

        ImVec2 effective_size;
        if (size.x < 0.0f)
            effective_size.x = ImGui::GetContentRegionAvail().x;
        else
            effective_size.x = size.x;
        ImGui::Dummy(ImVec2(effective_size.x, 0.0f));

        ImGui::Dummy(ImVec2(frame_height * 0.5f, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(frame_height * 0.5f, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextUnformatted(name);
        const auto label_min = ImGui::GetItemRectMin();
        const auto label_max = ImGui::GetItemRectMax();
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Dummy(ImVec2(0.0, frame_height + itemSpacing.y));
        ImGui::BeginGroup();

        ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frame_height * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x -= frame_height * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x -= frame_height * 0.5f;
#else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif

        ImGui::GetCurrentWindow()->Size.x -= frame_height;

        const auto item_width = ImGui::CalcItemWidth();
        ImGui::PushItemWidth(ImMax(0.0f, item_width - frame_height));

        s_group_panel_label_stack_.push_back(ImRect(label_min, label_max));
    }

    static void end_group_panel()
    {
        ImGui::PopItemWidth();

        const auto item_spacing = ImGui::GetStyle().ItemSpacing;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        const auto frame_height = ImGui::GetFrameHeight();

        ImGui::EndGroup();

        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Dummy(ImVec2(frame_height * 0.5f, 0.0f));
        ImGui::Dummy(ImVec2(0.0, frame_height - frame_height * 0.5f - item_spacing.y));

        ImGui::EndGroup();

        const auto item_min = ImGui::GetItemRectMin();
        auto item_max = ImGui::GetItemRectMax(); item_max.y += 10.f;

        auto label_rect = s_group_panel_label_stack_.back();
        s_group_panel_label_stack_.pop_back();

        const ImVec2 half_frame = ImVec2(frame_height * 0.25f, frame_height) * 0.5f;
        const ImRect frame_rect = ImRect(item_min + half_frame, item_max - ImVec2(half_frame.x, 0.0f));
        label_rect.Min.x -= item_spacing.x;
        label_rect.Max.x += item_spacing.x;

        for (int i = 0; i < 4; ++i) {
            switch (i) {
			case 0: ImGui::GetWindowDrawList()->PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(label_rect.Min.x, FLT_MAX), true); break;
            case 1: ImGui::GetWindowDrawList()->PushClipRect(ImVec2(label_rect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
            case 2: ImGui::GetWindowDrawList()->PushClipRect(ImVec2(label_rect.Min.x, -FLT_MAX), ImVec2(label_rect.Max.x, label_rect.Min.y), true); break;
            case 3: ImGui::GetWindowDrawList()->PushClipRect(ImVec2(label_rect.Min.x, label_rect.Max.y), ImVec2(label_rect.Max.x, FLT_MAX), true); break;
            }

            ImGui::GetWindowDrawList()->AddRect(frame_rect.Min, frame_rect.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)), half_frame.x);
            ImGui::PopClipRect();
        }

        ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frame_height * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x += frame_height * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x += frame_height * 0.5f;
#else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif

        ImGui::GetCurrentWindow()->Size.x += frame_height;

        ImGui::Dummy(ImVec2(0.0f, 0.0f));

        ImGui::EndGroup();
        ImGui::NewLine();
        ImGui::Spacing();
    }
#pragma endregion

    static auto draw_content_box(const char* label, std::function<void()> content_callback) noexcept -> void {
        begin_group_panel(label, ImVec2(-1, -1));
        std::invoke(content_callback);
        end_group_panel();
    }


	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<configs::route_configs_t>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).config_name.data();
		return true;
	};

	static auto ComboBoxConfigs(const char* label, int* currIndex, std::vector<configs::route_configs_t>& values) noexcept -> bool
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}
}