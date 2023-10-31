#include "ui.hpp"

#include "fonts/poppins.hpp"
#include "libs/keys.hpp"

#include "../../task_scheduler/task_scheduler.hpp"
#include "../../authentication/authenticate.h"
#include "../../features/misc/misc.hpp"
#include "../../games/games.hpp"

#include <format>
#include <map>

bool ui::Static::render_interface = true;

auto bg_color = ImColor{ 21, 22, 23, 255 };

std::vector<std::pair<ImVec2, ImVec2>> ui::lines;

std::vector< std::pair<math::types::vector_t, std::string>> ui::waypoints;
ImFont* smaller = nullptr;

std::string ui::logged_in = "";

void ui::set_style()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Border] = ImColor{ 52, 125, 235 };
    colors[ImGuiCol_ChildBg] = ImColor{ 52, 125, 235, 0 };
    colors[ImGuiCol_WindowBg] = bg_color;
    colors[ImGuiCol_FrameBg] = ImColor(32, 33, 34, 255);

    style->WindowPadding = { 0,0 };
    style->ChildRounding = 2.f;
    style->FramePadding = { 8.f, 5.f };
    style->ScrollbarSize = 5.f;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF,0x2000, 0x206F,0x3000, 0x30FF,0x31F0, 0x31FF, 0xFF00,
        0xFFEF,0x4e00, 0x9FAF,0x0400, 0x052F,0x2DE0, 0x2DFF,0xA640, 0xA69F, 0
    };

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Poppins_compressed_data, Poppins_compressed_size, 25, NULL, ranges);

    smaller = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Poppins_compressed_data, Poppins_compressed_size, 22, NULL, ranges);
}

static char username[300];
static char pass[300];

std::vector<const char*> options{ "Mouse", "Memory" };

void ui::render()
{
   ImGui::SetNextWindowSize(ImVec2{ 650, 575.f}, ImGuiCond_Once);

   auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

   if (!Static::render_interface)
       flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs;

    ImGui::Begin("Interface", nullptr, flags);
    {
        if (Static::render_interface)
        {
            auto draw = ImGui::GetWindowDrawList();
            auto pos = ImGui::GetWindowPos();

            draw->AddRectFilled(pos, pos + ImVec2{ 650, 45 + 3 }, ImColor(26, 27, 28, 255), 0);
            draw->AddRectFilled(pos, pos + ImVec2{ 170,575.f }, ImColor(26, 27, 28, 255), 0);
            draw->AddRectFilledMultiColor(pos + ImVec2{ 15, 40  }, pos + ImVec2{ 270, 40 + 4 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
            draw->AddText(pos + ImVec2{ 25, 15 }, ImColor{ 255,255,255 }, "Ro"); 
            draw->AddText(pos + ImVec2{ 25 + ImGui::CalcTextSize("Ro").x, 15 }, ImColor{ 72, 145, 255 }, "-");
            draw->AddText(pos + ImVec2{ 25 + ImGui::CalcTextSize("Ro-").x, 15 }, ImColor{ 255,255,255 }, "Ware | By ");
            draw->AddText(pos + ImVec2{ 25 + ImGui::CalcTextSize("Ro-Ware | By ").x, 15 }, ImColor{ 255,255,255 }, "Script");
            draw->AddText(pos + ImVec2{ 25 + ImGui::CalcTextSize("Ro-Ware | By Script").x, 15 }, ImColor{ 32, 105, 215 }, "-");
            draw->AddText(pos + ImVec2{ 25 + ImGui::CalcTextSize("Ro-Ware | By Script-").x, 15 }, ImColor{ 255,255,255 }, "Ware");

            static int tab = 0;

            ImGui::SetCursorPos({ 15, 55 });
            ImGui::BeginGroup();
            {
                //ImGui::Button("test");

                tab_button("AIM", "Legit", tab, 0);
                ////ImGui::SameLine();

                tab_button("VISUALS", "Visuals", tab, 3);

                tab_button("MISC", "Misc", tab, 2);

                tab_button("TOGGLES", "Toggles", tab, 5);

                tab_button("CONFIG", "Idk", tab, 4);

                tab_button("DOCUMENTATION", "a", tab, 7);

                //tab_button("PLAYERS", "plrs", tab, 5);

            }
            ImGui::EndGroup();

            if (tab == 7)
            {
                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,130.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 250, 63 }, ImColor{ 200,200,200, 200 }, "Documentation");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Docs", { 230, 330.f }, false);
                {
                    if (ui::centered_button("Open Documentation"))
                        ShellExecute(0, 0, "https://docs.script-ware.com/internal-interface", 0, 0, SW_SHOW);
                }
                ImGui::EndChild();
            }
            if (tab == 0)
            {

                //draw->AddRectFilled(pos + ImVec2{ 180, 355.f }, pos + ImVec2{ 410,515.f }, ImColor(26, 27, 28, 255), 4);
                //draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  385.f }, pos + ImVec2{ 395, 385.f + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                //draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 280, 363.f }, ImColor{ 200,200,200, 200 }, "Rcs");
                //ImGui::SetCursorPos({ 180, 390 });
                //ImGui::BeginChild("Rcs", { 230, 150.f }, false, ImGuiWindowFlags_NoScrollbar);
                //{
                //    ui::check_box("Enable Rcs", &std::get<bool>(menu::config::config_map["enable_rcs"]));
                //    ui::slider_int("Pitch", &std::get<int>(menu::config::config_map["rcs_pitch"]), 0, 100, "");
                //    ui::slider_int("Yaw", &std::get<int>(menu::config::config_map["rcs_yaw"]), 0, 100, "");
                //}

                //ImGui::EndChild();

                draw->AddRectFilled(pos + ImVec2{ 420, 55 }, pos + ImVec2{ 640,205.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 435,  85 }, pos + ImVec2{ 625, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 510, 63 }, ImColor{ 200,200,200, 200 }, "Other");
                ImGui::SetCursorPos({ 420, 90 });
                ImGui::BeginChild("Other", { 220.f, 110.f }, false);
                {
                    if (roblox::games::enabled_map["allow_triggerbot"])
                        ui::check_box("Trigger Bot", &std::get<bool>(menu::config::config_map["enable_trigger_bot"]));

                    ui::check_box("Team Check", &std::get<bool>(menu::config::config_map["enable_team_check"]));

                    if (roblox::games::enabled_map["allow_wallcheck"])
                        ui::check_box("Wall Check", &std::get<bool>(menu::config::config_map["enable_wall_check"]));

                    if (roblox::games::enabled_map["allow_aimbot_health"])
                        ui::check_box("Health Check", &std::get<bool>(menu::config::config_map["enable_aim_health_check"]));
                }

                ImGui::EndChild();


                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,365.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 270, 63 }, ImColor{ 200,200,200, 200 }, "Aimbot");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Aimbot", { 230, 270.f }, false);
                {
                    ui::check_box("Legit Aimbot", & std::get<bool>(menu::config::config_map["enable_aimbot"]));

                    if (ui::begin_combo("Type", options[std::get<int>(menu::config::config_map["aimbot_type"])], 0))
                    {
                        for (int n = 0; n < options.size() - static_cast<int>(!roblox::games::enabled_map["allow_memory_aimbot"]); n++)
                        {
                            bool is_selected = (std::get<int>(menu::config::config_map["aimbot_type"]) == n);
                            if (ui::Selectable(options[n], is_selected))
                                std::get<int>(menu::config::config_map["aimbot_type"]) = n;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    ui::check_box("Smoothing", & std::get<bool>(menu::config::config_map["enable_smoothing"]));
                    ui::check_box("Max Distance", &std::get<bool>(menu::config::config_map["max_distance_aim_enabled"]));
                    ui::slider_int("Smoothing Amount", &std::get<int>(menu::config::config_map["smoothing_amount"]), 1, 100, "");
                    ui::slider_int("Max Distance Amount", &std::get<int>(menu::config::config_map["max_distance_aim"]), 1, 5000, "");
                    ui::check_box("Fov Check", &std::get<bool>(menu::config::config_map["enable_fov_check"]));
                    ui::check_box("Draw Fov", &std::get<bool>(menu::config::config_map["draw_fov"]));
                    ui::slider_int("Fov", &std::get<int>(menu::config::config_map["fov_amount"]), 0, 100, "");
                    const char* items[] = { "Head", "Torso", "Random"};
                    if (ui::begin_combo("Bone", items[std::get<int>(menu::config::config_map["target_bone"])], 0))
                    {
                        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                        {
                            bool is_selected = (std::get<int>(menu::config::config_map["target_bone"]) == n);
                            if (ui::Selectable(items[n], is_selected))
                                std::get<int>(menu::config::config_map["target_bone"]) = n;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ui::hotkey("Aim Key", &std::get<int>(menu::config::config_map["aim_hotkey"]));
                }
                ImGui::EndChild();
            }
            else if (tab == 3)
            {
                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,430.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 250, 63 }, ImColor{ 200,200,200, 200 }, "Advanced Esp");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Advanced Esp", { 230, 330.f }, false);
                {
                    ui::check_box("Enable 3D square", &std::get<bool>(menu::config::config_map["enable_3d_square"]));
                    
                    ui::check_box("Enable 3D cube", &std::get<bool>(menu::config::config_map["enable_3d_cube"]));
                    
                    ui::check_box("Enable 2D square", &std::get<bool>(menu::config::config_map["enable_2d_but_good"]));
                    
                    ui::check_box("Enable 1D look direction line", &std::get<bool>(menu::config::config_map["enable_look_direction"]));
                    
                    ui::check_box("Enable rainbow", &std::get<bool>(menu::config::config_map["enable_rainbow"]));
                    ui::check_box("Enable custom esp", &std::get<bool>(menu::config::config_map["enable_custom_esp"]));
                    ui::check_box("Enable chams", &std::get<bool>(menu::config::config_map["enable_chams"]));

                    ui::color_picker("3D square", &std::get<ImU32>(menu::config::config_map["3d_square_color"]));
                    ui::color_picker("3D cube", &std::get<ImU32>(menu::config::config_map["3d_cube_color"]));
                    ui::color_picker("2D square", &std::get<ImU32>(menu::config::config_map["2d_color"]));
                    ui::color_picker("Tracers", &std::get<ImU32>(menu::config::config_map["tracers_color"]));
                    ui::color_picker("Look Direction", &std::get<ImU32>(menu::config::config_map["look_direction_color"]));

                    ui::color_picker("Custom Esp", &std::get<ImU32>(menu::config::config_map["custom_color"]));
                    ui::color_picker("Chams", &std::get<ImU32>(menu::config::config_map["chams_color"]));
                }
                ImGui::EndChild();

                draw->AddRectFilled(pos + ImVec2{ 420, 55 }, pos + ImVec2{ 640,260.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 435,  85 }, pos + ImVec2{ 625, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 520, 63 }, ImColor{ 200,200,200, 200 }, "Esp");
                ImGui::SetCursorPos({ 420, 90 });
                ImGui::BeginChild("Basic Esp", { 220.f, 160.f }, false);
                {
                    ui::slider_int("Max Distance", &std::get<int>(menu::config::config_map["esp_distance"]), 0, 5000, "");

                    ui::check_box("Enable Head Marker", &std::get<bool>(menu::config::config_map["enable_head_hitbox"]));

                    ui::check_box("Enable Distance Esp", &std::get<bool>(menu::config::config_map["enable_esp_distance"]));

                    if (roblox::games::enabled_map["allow_esp_health"])
                        ui::check_box("Enable Health Esp", &std::get<bool>(menu::config::config_map["enable_esp_health"]));

                    if(roblox::games::enabled_map["enable_esp_name"])
                        ui::check_box("Enable Name Esp", &std::get<bool>(menu::config::config_map["enable_esp_name"]));

                    if (roblox::games::enabled_map["allow_esp_tools"])
                        ui::check_box("Enable Tool Esp", &std::get<bool>(menu::config::config_map["enable_tool_esp"]));

                    ui::check_box("Enable Tracers", &std::get<bool>(menu::config::config_map["enable_tracers"]));

                    ui::check_box("Enemy Only", &std::get<bool>(menu::config::config_map["enemy_only_esp"]));
                }
                ImGui::EndChild();
            }
            else if (tab == 2)
            {
                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,290.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 270, 63 }, ImColor{ 200,200,200, 200 }, "Misc");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Misc", { 225.f, 190.f }, false);
                {
                    if (ui::slider_int("Fps Cap", &std::get<int>(menu::config::config_map["fps_limit"]), 30, 300, ""))
                        task_scheduler::set_fps_cap(std::get<int>(menu::config::config_map["fps_limit"]));

                    static int index = 0;

                    static const char* selected = "default";

                    if (ui::begin_combo("Waypoints", selected, 0))
                    {
                        for (int n = 0; n < waypoints.size(); n++)
                        {
                            const auto& waypoint = waypoints[n];

                            const auto is_selected = (selected == waypoint.second.c_str());

                            if (ui::Selectable(waypoint.second.c_str(), is_selected))
                            {
                                selected = waypoint.second.c_str();

                                index = n;
                            }

                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    static char waypoint_name[25];

                    const auto old = ImGui::GetCursorPos();

                    ImGui::SetCursorPos({ 23, old.y });

                    ImGui::PushItemWidth(182.f);

                    ImGui::InputText("##3", waypoint_name, sizeof(waypoint_name));

                    ImGui::SetCursorPos(old + ImVec2{0.f, 40.f});

                    if (ui::centered_button("Rename"))
                    {
                        if (!waypoints.empty() && index < waypoints.size())
                        {
                            waypoints[index].second = waypoint_name;
                        }
                    }

                    if (ui::centered_button("Save Waypoint"))
                    {
                        const auto parts = roblox::games::game->get_part(roblox::games::game->get_local_player());

                        if (parts.has_value())
                        {
                            const auto& [head, torso] = parts.value();

                            std::string waypoint = "waypoint";

                            waypoint.append(std::to_string(waypoints.size()));

                            waypoints.emplace_back(torso->primitive->cframe.position, waypoint);

                            index = ui::waypoints.size() - 1;

                        }
                    }

                    if (ui::centered_button("Delete Waypoint"))
                    {
                        if (!waypoints.empty() && index < waypoints.size())
                        {
                            waypoints.erase(waypoints.begin() + index);

                            index = ui::waypoints.size() - 1;
                        }
                    }

                   /* if(ui::centered_button("Teleport"))
                    {
                        if (!waypoints.empty())
                        {
                            if (const auto humanoid = roblox::games::game->get_local_player()->character->find_first_child<roblox::structs::humanoid_t>("Humanoid"); roblox::functions::is_valid_ptr(humanoid))
                            {
                            	
                            }
                        }
                    }*/

                    ui::check_box("Display info", &std::get<bool>(menu::config::config_map["enable_info"]));

                    ui::check_box("Anti Idle Kick", &std::get<bool>(menu::config::config_map["anti_idle_kick"]));

                    ui::check_box("Hide From Obs", &std::get<bool>(menu::config::config_map["obs_hide"]));
                }
                ImGui::EndChild();
            }
            else if (tab == 5)
            {
                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,290.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 270, 63 }, ImColor{ 200,200,200, 200 }, "Toggles");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Toggles", { 225.f, 190.f }, false);
                {
                    ui::hotkey("UI Toggle", &std::get<int>(menu::config::config_map["open_menu"]));
                }
                ImGui::EndChild();
            }
            else if (tab == 6)
            {
            draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,290.f }, ImColor(26, 27, 28, 255), 4);
            draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
            draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 270, 63 }, ImColor{ 200,200,200, 200 }, "Misc");
            ImGui::SetCursorPos({ 180, 90 });
            ImGui::BeginChild("Players", { 225.f, 190.f }, false);
            {
                static int index = 0;

                static const char* selected = "none";

                const auto children = roblox::globals::data_model->find_first_child("Players")->get_children<roblox::structs::player_t>();

                if (ui::begin_combo("Players", selected, 0))
                {
                    for (int n = 0; n < children.size(); n++)
                    {
                        const auto& child = children[n];

                        if (child && roblox::functions::is_valid_ptr(child))
                        {
                            const auto name = roblox::games::game->get_player_name(child);

                            if (name.has_value())
                            {

                                const auto is_selected = (selected == name.value());

                                if (ui::Selectable(name.value(), is_selected))
                                {
                                    selected = name.value();

                                    index = n;
                                }

                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ui::centered_button("Teleport"))
                {
                    if (!waypoints.empty())
                    {
                        if (const auto humanoid = roblox::games::game->get_local_player()->character->find_first_child<roblox::structs::humanoid_t>("Humanoid"); roblox::functions::is_valid_ptr(humanoid))
                        {

                        }
                    }
                }

                ui::centered_button("Whitelist"); //todo for aimbot


            }
            ImGui::EndChild();
            }
            else if (tab == 4)
            {
                draw->AddRectFilled(pos + ImVec2{ 180, 55 }, pos + ImVec2{ 410,350.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 195,  85 }, pos + ImVec2{ 395, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 270, 63 }, ImColor{ 200,200,200, 200 }, "Config");
                ImGui::SetCursorPos({ 180, 90 });
                ImGui::BeginChild("Config", { 230, 350.f }, false);
                {
                    static char config_name[25];

                    static const char* selected = "default";

                    if (ui::begin_combo("Configs", selected, 0))
                    {
                        menu::config::refresh_configs();

                        for (auto& config : menu::config::configs)
                        {
                            const auto is_selected = (selected == config);

                            if (ui::Selectable(config.data(), is_selected))
                                selected = config.data();

                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    if (ui::centered_button("Load Config"))
                        menu::config::load_cfg(selected);

                    ImGui::SetCursorPos({ 23, 80.f });

                    ImGui::PushItemWidth(182.f);

                    ImGui::InputText("##3", config_name, sizeof(config_name));

                    ImGui::PopItemWidth();

                    ImGui::SetCursorPos({ 0, 120.f });

                    if (ui::centered_button("Save Config"))
                        menu::config::dump_to_cfg(config_name);

                    if(ui::centered_button("Delete Config"))
                        menu::config::delete_cfg(selected);

                    if (ui::centered_button("Open Configs Folder"))
                    {
                        char* roaming_path;
                        std::size_t roaming_path_len;

                        if (!_dupenv_s(&roaming_path, &roaming_path_len, "APPDATA") && roaming_path)
                        {
                            const std::string roaming{ roaming_path };

                            const std::string full_path{ roaming + "\\RoWareConfigs\\" };

                            ShellExecute(0, 0, std::string{ roaming + "\\RoWareConfigs\\" }.c_str(), 0, 0, SW_SHOWMAXIMIZED);
                        }
                    }

                }
                ImGui::EndChild();

                draw->AddRectFilled(pos + ImVec2{ 420, 55 }, pos + ImVec2{ 640,300.f }, ImColor(26, 27, 28, 255), 4);
                draw->AddRectFilledMultiColor(pos + ImVec2{ 435,  85 }, pos + ImVec2{ 625, 85 + 2 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                draw->AddText(ImGui::GetDefaultFont(), 17, pos + ImVec2{ 510, 63 }, ImColor{ 200,200,200, 200 }, "Login");
                ImGui::SetCursorPos({ 420, 90 });
                ImGui::BeginChild("Login", { 250, 370.f }, false);
                {
                    ImGui::PushFont(smaller);

                    ImGui::PushItemWidth(200.f);
                    ImGui::SetCursorPos({ 10.f, 25.f });
                    ImGui::InputText("##0", username, sizeof(username));
                    ImGui::PopItemWidth();
                    //ImGui::InputText("", username, sizeof(username));
                    draw->AddText(pos + ImVec2{ 490.f, 90.f}, ImColor{ 255,255,255 }, "Username");

                    draw->AddText(pos + ImVec2{ 490.f, 150.f }, ImColor{ 255,255,255 }, "Password");
                    ImGui::PushItemWidth(200.f);
                    ImGui::SetCursorPos({ 10.f, 85.f });
                    ImGui::InputText("##1", pass, sizeof(pass), ImGuiInputTextFlags_Password);
                    ImGui::PopItemWidth();

                    ImGui::PopFont();

                    ImGui::SetCursorPos({ -5.f, 135.f });
                    if (ui::centered_button("Script-Ware Login"))
                    {
                        logged_in = perform_authentication(username, pass) ? "Auth Successful!" : "Bad Credentials!";
                           
                    }
                    ImGui::SetCursorPos({ -5.f, 170.f });
                    if (ui::centered_button("Purchase Script-Ware"))
                        ShellExecute(0, 0, "https://script-ware.com/", 0, 0, SW_SHOW);
;                }
                
                ImGui::EndChild();
                ImGui::SetCursorPos({ 215, 310 });
                ImGui::LabelText("##skid", logged_in.c_str());

            }
            //draw->AddRectFilled(pos + ImVec2{ 250, 75 }, pos + ImVec2{ 735, 540 }, ImColor(52, 125, 235, 70), 5);

      /*      draw->AddRectFilled(pos + ImVec2{ 75, 0 }, pos + ImVec2{ 770, 45 + 2 }, ImColor(35, 35, 55));
            draw->AddRectFilled(pos + ImVec2{ 0, 0 }, pos + ImVec2{ 75, 45 + 2 }, ImColor(35,35,45));
            draw->AddRectFilled(pos + ImVec2{ 75, 45 + 1 }, pos + ImVec2{ 24 + 487, 45 + 3 }, ImColor(255, 255, 255, 255));*/

            ImGui::SetNextWindowSize(ImVec2{ 350, 575.f }, ImGuiCond_Once);

            if(std::get<bool>(menu::config::config_map["enable_custom_esp"]))
            {
                flags |= ImGuiWindowFlags_NoMove;

                ImGui::Begin("Esp Customizer", nullptr, flags);
                {
                    draw = ImGui::GetWindowDrawList();

                    ImGui::SetWindowPos(pos + ImVec2{ 660.f, 0.f });

                    auto new_pos = pos + ImVec2{ 660.f, 0.f };

                    draw->AddRectFilled(new_pos, new_pos + ImVec2{ 350, 45 + 3 }, ImColor(26, 27, 28, 255), 0);
                    draw->AddRectFilled(new_pos + ImVec2{ 5.f, 45 + 3 + 5.f }, new_pos + ImVec2{ 345.f,570.f }, ImColor(26, 27, 28, 255), 0);
                    draw->AddRectFilledMultiColor(new_pos + ImVec2{ 15, 40 }, new_pos + ImVec2{ 175.f, 40 + 4 }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
                    draw->AddText(new_pos + ImVec2{ 25, 15 }, ImColor{ 255,255,255 }, "Esp Customizer");

                    ImGui::SetCursorPos({ 10.f, 40 });
                    draw->AddRectFilled(new_pos + ImVec2{ 10.f, 45 + 3 + 10.f }, new_pos + ImVec2{ 340.f,375.f }, ImColor(110, 110, 110, 40), 4);

                    static std::string curr_line = "None";

                    static int idx = 0;

                    ImGui::BeginChild("Window", { 345, 570.f }, false, ImGuiWindowFlags_NoScrollbar);
                    {
                        const auto window_center = ImVec2{ 185.f, 216.f };

                        for (auto i = 0u; i < ui::lines.size(); ++i)
                        {
                        	auto& [pos1, pos2] = ui::lines[i];

                            draw->AddLine(new_pos + window_center + pos1, new_pos + window_center + pos2, ImColor{255,255,255}, 2.f);

                            if(i == idx)
                            {
                                draw->AddCircleFilled(new_pos + window_center + pos1, 7, ImColor{ 255,255,255 });

                                draw->AddCircleFilled(new_pos + window_center + pos2, 7, ImColor{ 255,255,255 });

                                ImGui::SetCursorScreenPos(new_pos + window_center + pos1 - ImVec2{7.f, 7.f});
                                ImGui::InvisibleButton("Button1", {15.f, 15.f});

                                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                                {
                                    const auto pos = (new_pos + window_center - ImGui::GetMousePos());

                                    pos1 = ImVec2{ -std::clamp(pos.x, -155.f, 175.f), -std::clamp(pos.y, -159.f, 159.f) };
                                }

                                ImGui::SetCursorScreenPos(new_pos + window_center + pos2 - ImVec2{ 7.f, 7.f });
                                ImGui::InvisibleButton("Button2", { 15.f, 15.f });

                                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                                {
                                    const auto pos = (new_pos + window_center - ImGui::GetMousePos());

                                    pos2 = ImVec2{ -std::clamp(pos.x, -155.f, 175.f), -std::clamp(pos.y, -159.f, 159.f) };
                                }
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos({ 0.f, 380.f });
                    ImGui::BeginChild("Buttons", { 250, 200.f }, false, ImGuiWindowFlags_NoScrollbar);
                    {
                        if (ui::begin_combo("Lines", curr_line.c_str(), 0))
                        {

                            for (auto i = 0u; i < ui::lines.size(); ++i)
                            {
                                std::string line_name = "line";

                                line_name.append(std::to_string(i));

                                bool selected = i == idx;

                                if (ui::Selectable(line_name.c_str(), selected))
                                {
                                    curr_line = line_name;
                                    idx = i;
                                }

                                if (selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();

                        }

                        if(ui::centered_button("Add Line"))
                        {
                            if (ui::lines.size() < 20)
                            {
                                ui::lines.push_back({ { 0,0 }, { 100, (ui::lines.size()) * 10.f } });

                                idx = ui::lines.size() - 1;
                            }
                        }

                        if (ui::centered_button("Delete Line"))
                        {
                            if (ui::lines.size() > idx)
                            {
                                ui::lines.erase(ui::lines.begin() + idx);

                                idx = ui::lines.size() - 1;
                            }
                        }

                    }
                    ImGui::EndChild();
                }
                ImGui::End();
            }
        }
    }

}

struct TabStruct {
    float size;
    float hovered;
};

bool ui::tab_button(const char* label, const char* icon, int& tab, int index)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui; //works?
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize({ 140,30 }, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 4.0f);

    ImRect bb(pos, pos + size);
    ImGui::InvisibleButton(label, size);

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(7);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        tab = index,
        Static::current_tab = label;

    window->DrawList->AddRectFilled(bb.Min, bb.Max, tab != index ? ImColor(26, 27, 28, 100) : ImColor(32, 33, 34, 255), 2);

    ImGui::PushClipRect(bb.Min, bb.Max, false);

    ImGui::PopClipRect();

    auto center = ((bb.Max - bb.Min) / 2.f) - ImGui::CalcTextSize(label) / 2.f;

    window->DrawList->AddText(NULL, 14, bb.Min + ImVec2{10.f, 10.f}, tab != index ? ImColor(210, 210, 210, 160) :ImColor(52, 125, 235) , label);

    return false;
}

bool ui::button(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui; //works?
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos + ImVec2{10.f, 0.f};
    ImVec2 size = ImGui::CalcItemSize({ 140,30 }, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 4.0f);

    ImRect bb(pos, pos + size);
    ImGui::InvisibleButton(label, size);

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(7);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return true;

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(32, 33, 34, 255), 2);

    ImGui::PushClipRect(bb.Min, bb.Max, false);

    ImGui::PopClipRect();

    auto center = ((bb.Max - bb.Min) / 2.f) - ImGui::CalcTextSize(label) / 2.f;

    window->DrawList->AddText(NULL, 14, bb.Min + ImVec2{ 10.f, 10.f }, ImColor(52, 125, 235), label);

    return false;
}

bool ui::centered_button(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui; //works?
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos + ImVec2{ 16.f, 0.f };
    ImVec2 size = ImGui::CalcItemSize({ 200,30 }, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 4.0f);

    ImRect bb(pos, pos + size);
    ImGui::InvisibleButton(label, size);

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(7);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return true;

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(32, 33, 34, 255), 2);

    ImGui::PushClipRect(bb.Min, bb.Max, false);

    ImGui::PopClipRect();

    auto center = ((bb.Max - bb.Min) / 2.f) - ImGui::CalcTextSize(label);

    ImGui::PushFont(smaller);

    window->DrawList->AddText(NULL, 22, ImVec2{ (bb.Min.x + ((bb.Max.x - bb.Min.x) / 2.f)) - (ImGui::CalcTextSize(label).x / 2), bb.Min.y + 5.f }, ImColor(52, 125, 235), label);

    ImGui::PopFont();

    return false;
}


void ui::check_box(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    int flags = 0;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos + ImVec2{ style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f };
    ImVec2 size = { 30, 21 };

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y * 2);
    if (!ImGui::ItemAdd(bb, id))
        return;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    if (hovered)
        ImGui::SetMouseCursor(7);

    if (pressed)
        *v = !*v;

    float Active = !*v ? 0 : 15;

    window->DrawList->AddRectFilled(bb.Min + ImVec2(0, -5), bb.Min + ImVec2(30, 10), ImColor(32, 39, 49), 2);

    window->DrawList->AddRectFilled(bb.Min + ImVec2(Active, -5), bb.Min + ImVec2(Active + 15, 10), !*v ? ImColor(110, 110, 110, 175) : ImColor(52, 125, 235));

    window->DrawList->AddText(ImGui::GetDefaultFont(), 15, bb.Min + ImVec2(45, -5), ImColor(200, 200, 200, 200), label);
}

static const char* PatchFormatStringFloatToInt(const char* fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
        return "%d";
    const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
    if (fmt_end > fmt_start && fmt_end[-1] == 'f')
    {
        if (fmt_start == fmt && fmt_end[0] == 0)
            return "%d";
        ImGuiContext& g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
        return g.TempBuffer;
    }
    return fmt;
}


bool ui::slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power, bool non_interactable)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos + ImVec2{ style.FramePadding.x * 2.0f, style.FramePadding.y * 4.0f };
    ImVec2 size = { 200, 21 };

    const ImRect frame_bb(pos, pos + size);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x, 12.f));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
    else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
        format = PatchFormatStringFloatToInt(format);

    const bool hovered = ImGui::ItemHoverable(frame_bb, id);
    bool temp_input_is_active = ImGui::TempInputIsActive(id);
    bool temp_input_start = false;
    if (!temp_input_is_active && !non_interactable)
    {
        const bool focus_requested = hovered && g.IO.MouseClicked[0];
        const bool clicked = (hovered && g.IO.MouseClicked[0]);
        if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            if (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id)
            {
                temp_input_start = true;
            }
        }
    }

    ImGui::RenderFrame(frame_bb.Min + ImVec2(1, 0), frame_bb.Max - ImVec2(0, 10), ImColor(35, 42, 51), true, 2);


    ImRect grab_bb;
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, "%i", ImGuiSliderFlags_None, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);

    char value_buf[64];
    const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, "%i");

    if (grab_bb.Max.x > grab_bb.Min.x)
        window->DrawList->AddRectFilledMultiColor(frame_bb.Min, { std::stoi(value_buf) == 0 ? grab_bb.Min.x : grab_bb.Max.x, grab_bb.Max.y - 9.f}, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });

    if(!non_interactable)
        window->DrawList->AddRectFilled(grab_bb.Min - ImVec2(2, 3), grab_bb.Max - ImVec2(0,8), ImColor(110, 110, 110, 255), 2);

    window->DrawList->AddText(ImGui::GetDefaultFont(), 15, frame_bb.Min + ImVec2(170, -20), ImColor(200, 200, 200, 200), value_buf);

    window->DrawList->AddText(ImGui::GetDefaultFont(), 15, frame_bb.Min - ImVec2(0, 20), ImColor(200, 200, 200, 200), label);

    return value_changed;
}

bool ui::slider_int(const char* label, int* v, int v_min, int v_max, const char* format, bool non_interactable)
{
    return slider_scalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, 1, non_interactable);
}

bool ui::begin_combo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    const float arrow_size = 40;
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float w = ImGui::CalcItemWidth();

    ImVec2 pos = window->DC.CursorPos + ImVec2{ style.FramePadding.x * 2.0f, style.FramePadding.y * 4.0f };
    ImVec2 size = { 200, 21 };

    const ImRect frame_bb(pos, pos + size);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x, 20.f));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if ((pressed || g.NavActivateId == id) && !popup_open)
    {
        ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    // Render shape
    const ImU32 frame_col = ImColor(35, 42, 51);
    const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
    ImGui::RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersLeft);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        ImU32 bg_col = ImColor(35, 42, 51);
        ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
        if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
            ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 0.3f);
    }
    ImGui::RenderFrameBorder(frame_bb.Min, frame_bb.Max, 2.f);

    window->DrawList->AddRectFilledMultiColor(frame_bb.Min, frame_bb.Max - ImVec2(0, 25), ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });

    // Custom preview
    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(frame_bb.Min.x, frame_bb.Min.y, value_x2, frame_bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("{", "}");
        window->DrawList->AddText(ImGui::GetDefaultFont(), 15.f, frame_bb.Min + style.FramePadding - ImVec2{ 0, 3 }, ImColor(200, 200, 200, 200), preview_value);
    }
    if (label_size.x > 0)
        window->DrawList->AddText(ImGui::GetDefaultFont(), 15.f, ImVec2(frame_bb.Min.x, frame_bb.Min.y - 20.f), ImColor(200, 200, 200, 200), label);

    if (!popup_open)
        return false;

    g.NextWindowData.Flags = backup_next_window_data_flags;
    return  ui::BeginComboPopup(popup_id, frame_bb, flags);
}

using namespace ImGui;

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ui::BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(popup_id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    // Set popup size
    float w = bb.GetWidth();
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    // This is essentially a specialized version of BeginPopupEx()
    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    // Set position given a custom constraint (peak into expected window size so we can position it)
    // FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints() function?
    // FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
            ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = (flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down; // Left = "Below, Toward Left", Down = "Below, Toward Right (default)"
            ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
            ImVec2 pos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(pos);
        }
    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    //PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Horizontally align ourselves with the framed text
    //PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    bool ret = Begin(name, NULL, window_flags);
    //PopStyleVar(2);
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

bool ui::Selectable(const char* label, bool selected)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 size(200.f, 20.f);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ItemSize(size, 0.0f);

    auto flags = 0;

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = pos;
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    bool item_add;
    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    if (disabled_item)
    {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
        item_add = ItemAdd(bb, id);
        g.CurrentItemFlags = backup_item_flags;
    }
    else
    {
        item_add = ItemAdd(bb, id);
    }

    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) // Only testing this as an optimization
        BeginDisabled(true);

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Auto-select when moved into
    // - This will be more fully fleshed in the range-select branch
    // - This is not exposed as it won't nicely work with some user side handling of shift/control
    // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
    //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
    //   - (2) usage will fail with clipped items
    //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == window->DC.NavFocusScopeIdCurrent)
        if (g.NavJustMovedToId == id)
            selected = pressed = true;

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent, ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected)
    {
        const ImU32 col = GetColorU32(ImGuiCol_FrameBg);
        RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
    }
    //RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

    if (span_all_columns && window->DC.CurrentColumns)
        PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePopBackgroundChannel();

    //RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);

    window->DrawList->AddText(ImGui::GetDefaultFont(), 15.f, text_min + ImVec2{ 8, 4 }, ImColor(200, 200, 200, 200), label);

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();

    if (disabled_item && !disabled_global)
        EndDisabled();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020
}

bool ui::hotkey(const char* label, int* k)
{
    using namespace ImGui;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos + ImVec2{ 110.f, 0.f };
    ImVec2 size = { 105,40 };

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    const bool hovered = ImGui::ItemHoverable(bb, id);

    if (hovered) {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_TextInput;
    }

    const bool user_clicked = hovered && io.MouseClicked[0];

    if (user_clicked) {
        if (g.ActiveId != id) {
            // Start edition
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            *k = 0;
        }
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (io.MouseClicked[0]) {
        // Release focus when we click outside
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    int key = *k;

    if (g.ActiveId == id) {
        for (auto i = 0; i < 5; i++) {
            if (io.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    break;
                }
                value_changed = true;
                ImGui::ClearActiveID();
            }
        }
        if (!value_changed) {
            for (auto i = VK_BACK; i <= VK_RMENU; i++) {
                if (io.KeysDown[i]) {
                    key = i;
                    value_changed = true;
                    ImGui::ClearActiveID();
                }
            }
        }

        if (IsKeyPressedMap(ImGuiKey_Escape)) {
            *k = 0;
            ImGui::ClearActiveID();
        }
        else {
            *k = key;
        }
    }


    char buf_display[64] = "NONE";

    ImGui::RenderFrame(bb.Min + ImVec2(0, 0), bb.Max - ImVec2(0, 10), ImColor(33, 33, 33), false, 4);

    if (*k != 0 && g.ActiveId != id) {
        strcpy_s(buf_display, KeyNames[*k]);
    }
    else if (g.ActiveId == id) {
        strcpy_s(buf_display, "Press a key");
    }

	window->DrawList->AddText(ImGui::GetDefaultFont(), 18, ImVec2{ (bb.Min.x + ((bb.Max.x - bb.Min.x) / 2.f)) - (ui::calc_text_size(buf_display, 18).x / 2), bb.Min.y + 5.f }, ImColor(211, 211, 211, 160), buf_display);

    window->DrawList->AddText(ImGui::GetDefaultFont(), 18, bb.Min - ImVec2(90, -5), ImColor(211, 211, 211, 160), label);

    return value_changed;
}

void ui::color_picker(const char* identifier, ImU32* col)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    int flags = 0;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(identifier);

    ImVec2 pos = window->DC.CursorPos + ImVec2{ 110.f, 0.f };
    ImVec2 size = { 105,35 };

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImColor col2 = *col;

    float color[4] = { col2.Value.x, col2.Value.y, col2.Value.z,col2.Value.w };

    ImGui::SetNextWindowSize({ 530.f,  500.f });
    if (ImGui::BeginPopup(identifier, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::SetCursorPos({ 10,10 });
        ImGui::BeginGroup();
        {
            ImGui::PushFont(smaller);
            if(ImGui::ColorPicker4("##coloredit", color))
            {
                *col = ImColor{ color[0], color[1], color[2], color[3] };
            }
            ImGui::PopFont();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::RenderColorRectWithAlphaCheckerboard(window->DrawList, bb.Max - ImVec2(16, 28), bb.Max - ImVec2(0, 12), ImColor{ color[0], color[1], color[2], color[3] }, 3.5f, ImVec2(0, 0), 4);

    ImGui::SetCursorScreenPos(bb.Max - ImVec2(16, 28));

    if (ImGui::InvisibleButton(identifier, ImVec2{ 15.f, 15.f })) {
        ImGui::OpenPopup(identifier);
    }

    window->DrawList->AddText(ImGui::GetDefaultFont(), 14, bb.Min - ImVec2(90, -10), ImColor(211, 211, 211, 160), identifier);
}

ImVec2 ui::calc_text_size(const char* text, float font_size, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;

    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = FindRenderedTextEnd(text, text_end);      // Hide anything after a '##' string
    else
        text_display_end = text_end;

    ImFont* font = g.Font;

    if (text == text_display_end)
        return ImVec2(0.0f, font_size);
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

    // Round
    // FIXME: This has been here since Dec 2015 (7b0bf230) but down the line we want this out.
    // FIXME: Investigate using ceilf or e.g.
    // - https://git.musl-libc.org/cgit/musl/tree/src/math/ceilf.c
    // - https://embarkstudios.github.io/rust-gpu/api/src/libm/math/ceilf.rs.html
    text_size.x = IM_FLOOR(text_size.x + 0.99999f);

    return text_size;
}