#pragma once

#include <string>
#include "../../dependencies/imgui/imgui.h"
#include "libs/imgui_tricks.hpp"
#include "config/config.hpp"

namespace ui
{
	namespace Static {
		inline std::string current_tab = "LEGIT";
		extern bool render_interface;
	}

	extern std::string logged_in;

	extern std::vector<std::pair<ImVec2, ImVec2>> lines;

	extern std::vector<std::pair<math::types::vector_t, std::string>> waypoints;

	void set_style();
	void render();

	bool tab_button(const char* label, const char* icon, int& tab, int index);

	bool button(const char* label);

	bool centered_button(const char* label);

	void check_box(const char* label, bool* v);

	bool slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, bool non_interactable = false);

	bool slider_int(const char* label, int* v, int v_min, int v_max, const char* format, bool non_interactable = false);

	bool begin_combo(const char* label, const char* preview_value, ImGuiComboFlags flags);

	bool BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags);

	bool Selectable(const char* label, bool selected);

	bool hotkey(const char* label, int* k);

	void color_picker(const char* identifier, ImU32* col);

	ImVec2 calc_text_size(const char* text, float font_size, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
}