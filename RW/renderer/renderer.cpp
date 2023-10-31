#include "renderer.hpp"

#include "../features/visuals/visuals.hpp"
#include "../features/aimbot/aimbot.hpp"

#include "menu/config/config.hpp"
#include "menu/ui.hpp"

#include "../authentication/authenticate.h"
#include "../authentication/encryption.h"
#include "../dependencies/inst_size/InstructionSize.h"
#include "../../games/games.hpp"

#include <d3dcompiler.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <mutex>
#include <Psapi.h>

#pragma warning(disable : 4996)

DXGI_SWAP_CHAIN_DESC renderer::values::sd;
WNDPROC renderer::values::original_wnd_proc;

bool renderer::values::hooked = false;

HWND renderer::values::roblox_window;
HWND renderer::values::window;
RECT renderer::values::screen_bounds;
ID3D11Device* renderer::values::global_device = nullptr;
ID3D11DeviceContext* renderer::values::global_context = nullptr;
ID3D11RenderTargetView* renderer::values::main_render_target_view;
ID3D11Texture2D* renderer::values::back_buffer = nullptr;
IDXGISwapChain* renderer::values::global_swapchain = nullptr;

present_t renderer::values::d3d11_present = nullptr;
resize_buffers_t renderer::values::d3d11_resize_buffers = nullptr;
d3d11_capture_t renderer::values::d3d11_capture = nullptr;

d3d11_ps_set_shader_resources renderer::values::set_shader_resources_orig = nullptr;
d3d11_draw_indexed_instanced_t renderer::values::draw_indexed_instanced = nullptr;
d3d11_draw_indexed_t renderer::values::draw_indexed = nullptr;

ID3D11DepthStencilState* no_depth = nullptr;
ID3D11DepthStencilState* original = nullptr;

ID3D11Buffer* ve_buff;
UINT stride;
UINT ve_buffer_offset;
D3D11_BUFFER_DESC vedesc;

ID3D11Buffer* in_buf;
DXGI_FORMAT in_format;
UINT in_offs;
D3D11_BUFFER_DESC indesc;

UINT pscStartSlot;
ID3D11Buffer* pscBuffer;
D3D11_BUFFER_DESC pscdesc;

UINT pssr;
ID3D11Resource* Resource;
D3D11_SHADER_RESOURCE_VIEW_DESC descr;
D3D11_TEXTURE2D_DESC texdesc;

ID3D11PixelShader* pixel_shader = nullptr;

static bool window_selected = false;

static std::uintptr_t* raw_swapchain_ptr = nullptr;

float renderer::values::dpi_scale = 1.0f;

LRESULT __stdcall wnd_proc(const HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	ImGuiIO& io = ImGui::GetIO();

	ImGui::GetIO().MouseDrawCursor = ui::Static::render_interface;

	switch (message)
	{
	case WM_KILLFOCUS:
		window_selected = false;
		break;
	case WM_SETFOCUS:
		window_selected = true;
		break;
	case WH_CBT:
		renderer::values::window = renderer::values::sd.OutputWindow;
		renderer::values::roblox_window = hwnd;
		break;
	case WM_KEYDOWN:
		if (w_param == std::get<int>(menu::config::config_map["open_menu"])) ui::Static::render_interface = !ui::Static::render_interface;
		break;
	case WM_DPICHANGED: // Win8.1+
		//printf("OH BOYS CMON DPI CHANGED: %i - %i (%f)\n", LOWORD(w_param), HIWORD(w_param), ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd));
		renderer::values::dpi_scale = LOWORD(w_param) / 96.0f;
		//printf("new dpi scale: %f\n", renderer::values::dpi_scale);

		break;
	}

	if (!ui::Static::render_interface)
		return CallWindowProc(renderer::values::original_wnd_proc, hwnd, message, w_param, l_param);
	ImGui_ImplWin32_WndProcHandler(hwnd, message, w_param, l_param);

	switch (message)
	{
	case 522:
	case 513:
	case 533:
	case 514:
	case 134:
	case 516:
	case 517:
	case 258:
	case 257:
	case 132:
	case 127:
	case 255:
	case 523:
	case 524:
	case 793:
		if (ui::Static::render_interface) return TRUE;
		break;
	}

	return CallWindowProc(renderer::values::original_wnd_proc, hwnd, message, w_param, l_param);
}

HRESULT GenerateShader(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, float r, float g, float b, float a)
{
	char szCast[] =
		"struct VS_OUT"
		"{"
		" float4 Position : SV_Position;"
		" float4 Color : COLOR0;"
		"};"

		"float4 main( VS_OUT input ) : SV_Target"
		"{"
		" float4 fake;"
		" fake.a = %f;"
		" fake.r = %f;"
		" fake.g = %f;"
		" fake.b = %f;"
		" return fake;"
		"}";
	ID3D10Blob* pBlob;
	char szPixelShader[1000];

	sprintf_s(szPixelShader, szCast, a, r, g, b);

	ID3DBlob* d3dErrorMsgBlob;

	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", nullptr, nullptr, "main", "ps_4_0", NULL,
	                        NULL,
	                        &pBlob, &d3dErrorMsgBlob);

	if (FAILED(hr))
		return hr;

	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pShader);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

std::once_flag is_init;

bool is_obs(std::uintptr_t address)
{
	const auto handle = GetModuleHandleA("graphics-hook32.dll");

	const auto module_base = (std::uintptr_t)handle;

	MODULEINFO modinfo{};

	GetModuleInformation(GetCurrentProcess(), handle, &modinfo, sizeof(MODULEINFO));

	return ((address >= module_base) && (address <= (module_base + modinfo.SizeOfImage)));
}

ImU32 old = 0;

void format_helper(std::ostringstream& oss, std::string_view str) //todo: move this from here
{
	oss << str;
}

template<typename T, typename... Targs>
void format_helper(std::ostringstream& oss, std::string_view str, T value, Targs... args)
{
	std::size_t openBracket = str.find_first_of('{');
	if (openBracket != std::string::npos)
	{
		std::size_t closeBracket = str.find_first_of('}', openBracket + 1);
		if (closeBracket == std::string::npos)
			throw std::runtime_error("missing closing bracket.");
		oss << str.substr(0, openBracket);
		oss << value;
		format_helper(oss, str.substr(closeBracket + 1), args...);
		return;
	}
	oss << str;
}

std::string format(std::string_view str)
{
	return std::string(str);
}

template<typename T, typename... Targs>
std::string format(std::string_view str, T value, Targs...args)
{
	std::ostringstream oss;
	format_helper(oss, str, value, args...);
	return oss.str();
}

void renderer::render(IDXGISwapChain* swap_chain, void* backbuffer_ptr, bool is_from_present)
{
	std::call_once(is_init, [&]()
	{
			if (try_auth())
				ui::logged_in = "Auth Successful!";

		swap_chain->GetDesc(&values::sd);
		swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&values::global_device));
		swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&values::back_buffer));

		values::window = values::sd.OutputWindow;
		values::original_wnd_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
			values::window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc)));

		values::global_device->GetImmediateContext(&values::global_context);
		values::global_device->CreateRenderTargetView(values::back_buffer, nullptr, &values::main_render_target_view);
		values::back_buffer->Release();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplWin32_Init(values::window);
		ImGui_ImplDX11_Init(values::global_device, values::global_context);

		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depth_stencil_desc.StencilEnable = FALSE;
		depth_stencil_desc.StencilReadMask = 0xFF;
		depth_stencil_desc.StencilWriteMask = 0xFF;

		depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		values::global_device->CreateDepthStencilState(&depth_stencil_desc, &no_depth);

		ui::set_style();
	});


	if (const auto col1 = std::get<ImU32>(menu::config::config_map["chams_color"]); col1 != old && std::get<bool>(menu::config::config_map["enable_chams"]))
	{
		static std::once_flag chams_init;

		std::call_once(chams_init, renderer::initialize_chams);

		old = col1;

		ImColor col = col1;

		GenerateShader(values::global_device, &pixel_shader, col.Value.x, col.Value.y, col.Value.z, col.Value.w);
	}

	if (values::main_render_target_view == nullptr)
	{
		swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&values::back_buffer));
		values::global_device->CreateRenderTargetView(values::back_buffer, nullptr, &values::main_render_target_view);

		values::back_buffer->Release();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGuiIO& io = ImGui::GetIO();

	// Fix the mouse position
	io.MousePos.x /= renderer::values::dpi_scale;
	io.MousePos.y /= renderer::values::dpi_scale;

	ImGui::NewFrame();

	features::visuals::callback();
	modules::aimbot::draw_fov();

	if (ui::Static::render_interface)
		ui::render();

	for (const auto& pos : ui::waypoints)
	{
		const auto screen_pos = math::functions::world_to_screen(pos.first);

		if (!screen_pos.has_value())
			continue;

		ImGui::GetBackgroundDrawList()->AddCircleFilled({ screen_pos.value().x,screen_pos.value().y }, 10.f, ImColor{ 255,255,255 });

		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetDefaultFont(), 14, { screen_pos.value().x - ui::calc_text_size(pos.second.c_str(), 14.f).x / 2.f,screen_pos.value().y + 10.f }, ImColor{ 255,255,255 }, pos.second.c_str());
	}

	std::time_t t = std::time(nullptr);
	tm tm_v;
	char mbstr[100];
	localtime_s(&tm_v, &t);
	std::strftime(mbstr, sizeof(mbstr), "%A %c", &tm_v);

	if (roblox::globals::players) //we dont wan't this to render while game not loaded lol
	{
		const auto watermark = format("Ro-Ware | Time: {} | Biggest Idle Time: {} | Current Idle Time: {} | FPS: {} | PlaceId: {:d}", mbstr, std::round(roblox::functions::time::get_biggest_idle_time()), std::round(roblox::functions::time::get_current_idle_time()), std::round(ImGui::GetIO().Framerate), static_cast<std::uint64_t>(roblox::globals::place_id));

		const auto draw_list = ImGui::GetBackgroundDrawList();

		if (std::get<bool>(menu::config::config_map["enable_info"]))
		{
			draw_list->AddRectFilled({ 100, 5 }, { 110 + ui::calc_text_size(watermark.c_str(), 17.f).x, 25 }, ImColor{ 21, 22, 23, 255 });
			draw_list->AddRectFilledMultiColor({ 100,  5.f + ui::calc_text_size(watermark.c_str(), 17.f).y }, { 110 + ui::calc_text_size(watermark.c_str(), 17.f).x, 10.f + ui::calc_text_size(watermark.c_str(), 17.f).y }, ImColor{ 52, 125, 235 }, ImColor{ 52, 125, 235 }, ImColor{ 2, 65, 185 }, ImColor{ 2, 65, 185 });
			draw_list->AddText(ImGui::GetDefaultFont(), 17.f, { 105, 5 }, ImColor{ 0.6f,0.6f,0.6f,0.9f }, watermark.c_str());
		}

	}

	ImGui::Render();
	
	ImDrawData* data = ImGui::GetDrawData();

	// ty kind paste from data->ScaleClipRects() with scaling other too - alex too op
	for (int i = 0; i < data->CmdListsCount; i++)
	{
		ImDrawList* cmd_list = data->CmdLists[i];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
			cmd->ClipRect = ImVec4(cmd->ClipRect.x * renderer::values::dpi_scale, cmd->ClipRect.y * renderer::values::dpi_scale, cmd->ClipRect.z * renderer::values::dpi_scale, cmd->ClipRect.w * renderer::values::dpi_scale);
		}
	
		for (auto& cmd : data->CmdLists[i]->VtxBuffer)
		{
			cmd.pos.x *= renderer::values::dpi_scale;
			cmd.pos.y *= renderer::values::dpi_scale;
		}
	}

	values::global_context->OMSetRenderTargets(1, &values::main_render_target_view, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void __cdecl renderer::prestent_cpy(void* backbuffer_ptr, void* swap_ptr)
{
	const auto hide = std::get<bool>(menu::config::config_map["obs_hide"]);

	if(!hide)
		render(reinterpret_cast<IDXGISwapChain*>(backbuffer_ptr), nullptr, false);

	values::d3d11_capture(backbuffer_ptr, swap_ptr); //just swap these for toggle :O

	if(hide)
		render(reinterpret_cast<IDXGISwapChain*>(backbuffer_ptr), nullptr, false);
}

std::pair<std::int32_t, std::int32_t> renderer::get_screen_metrics()
{
	return
	{
		values::screen_bounds.right - values::screen_bounds.left,
		values::screen_bounds.bottom - values::screen_bounds.top
	};
}

std::uintptr_t tramp_hook(std::uintptr_t func, std::uintptr_t new_func, std::size_t inst_size)
{
	constexpr auto extra_size = 5;

	auto clone = reinterpret_cast<std::uintptr_t>(VirtualAlloc(nullptr, inst_size + extra_size,
	                                                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	if (!clone)
		return 0;

	std::memmove(reinterpret_cast<void*>(clone), reinterpret_cast<void*>(func), inst_size);

	const auto jmp_pos = (func - clone - extra_size);

	*reinterpret_cast<std::uint8_t*>(clone + inst_size) = 0xE9;
	*reinterpret_cast<std::uintptr_t*>(clone + inst_size + 1) = jmp_pos;

	DWORD old_protect;

	VirtualProtect(reinterpret_cast<void*>(func), inst_size, 0x40, &old_protect);

	std::memset(reinterpret_cast<void*>(func), 0x90, inst_size);

	const auto rel_location = (new_func - func - extra_size);
	*reinterpret_cast<std::uint8_t*>(func) = 0xE9;
	*reinterpret_cast<std::uintptr_t*>(func + 1) = rel_location;

	VirtualProtect(reinterpret_cast<void*>(func), inst_size, old_protect, &old_protect);

	return clone;
}

HRESULT __stdcall renderer::present_hook(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags)
{
	

	if (std::get<bool>(menu::config::config_map["obs_hide"]))
	{
		if (auto handle = GetModuleHandleA("graphics-hook32.dll"); reinterpret_cast<std::uintptr_t>(handle) && handle !=
			INVALID_HANDLE_VALUE)
		{
			const auto start = reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(values::d3d11_present) + *
				reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(values::d3d11_present) + 1) + 5);

			HRESULT ret = TRUE;

			if (is_obs(
				reinterpret_cast<std::uintptr_t>(values::d3d11_present) + *reinterpret_cast<std::uintptr_t*>(
					reinterpret_cast<std::uintptr_t>(values::d3d11_present) + 1) + 5))
			{
				DWORD old_protection;

				VirtualProtect(raw_swapchain_ptr, sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &old_protection);

				raw_swapchain_ptr[8] = reinterpret_cast<std::uintptr_t>(values::d3d11_present);

				VirtualProtect(raw_swapchain_ptr, sizeof(std::uintptr_t), old_protection, &old_protection);

				auto i = 0u;

				for (i = 0u; i < 100u; ++i)
				{
					if (start[i] == 0x75 && start[i + 2] == 0x83 && start[i + 3] == 0x3D) //jne cmp dword ptr
					{
						ret = values::d3d11_present(swap_chain, sync_interval, flags);

						values::d3d11_capture = reinterpret_cast<decltype(values::d3d11_capture)>(tramp_hook(
							**reinterpret_cast<std::uintptr_t**>(start + i + 4),
							reinterpret_cast<std::uintptr_t>(prestent_cpy), 6));

						break;
					}
				}
			}

			return ret;
		}
	}

	render(swap_chain, nullptr, true);

	return values::d3d11_present(swap_chain, sync_interval, flags);
}

HRESULT __stdcall renderer::resize_buffers_hook(IDXGISwapChain* this_ptr, UINT buffer_count, UINT width, UINT height,
                                                DXGI_FORMAT new_format, UINT swap_chain_flags)
{
	if (values::main_render_target_view)
	{
		values::main_render_target_view->Release();
		values::main_render_target_view = nullptr;
	}

	values::window = values::sd.OutputWindow;

	GetWindowRect(values::roblox_window, &values::screen_bounds);

	return values::d3d11_resize_buffers(this_ptr, buffer_count, width, height, new_format, swap_chain_flags);
}

void __stdcall renderer::d3d11_draw_indexed_instanced(ID3D11DeviceContext* p_context, UINT idx, UINT inst_cnt,
                                                      UINT start_idx, INT base_v, UINT start_i)
{
	static UINT stride = 0;

	if (p_context->IAGetVertexBuffers(0, 1, &ve_buff, &stride, &ve_buffer_offset); ve_buff)
	{
		ve_buff->GetDesc(&vedesc);

		ve_buff->Release();
		ve_buff = nullptr;
	}

	if (p_context->IAGetIndexBuffer(&in_buf, &in_format, &in_offs); in_buf)
	{
		in_buf->GetDesc(&indesc);

		in_buf->Release();
		in_buf = nullptr;
	}

	static UINT start_slot = 0;

	if (p_context->PSGetConstantBuffers(start_slot, 1, &pscBuffer); pscBuffer)
	{
		pscBuffer->GetDesc(&pscdesc);

		pscBuffer->Release();
		pscBuffer = nullptr;
	}

	if (std::get<bool>(menu::config::config_map["enable_chams"]) && roblox::games::game->get_stride() == stride)
	{
		//if (132 != idx )
		//	return values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i);

		if (indesc.ByteWidth != 4992 && indesc.ByteWidth != 264)
			return values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i); //32 :: 55040 :: 4992 :: 1048576 head 32 :: 23168 :: 7644 :: 1048576 hair

		if (vedesc.ByteWidth != 55040 && vedesc.ByteWidth != 3904)
			return values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i); 

		if (pscdesc.ByteWidth != 1048576)
			return values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i);

		p_context->OMGetDepthStencilState(&original, nullptr);

		p_context->OMSetDepthStencilState(no_depth, 0);

		values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i);

		p_context->PSSetShader(pixel_shader, nullptr, 0);

		p_context->OMSetDepthStencilState(original, 0);

		original->Release();
		original = nullptr;
	}

	values::draw_indexed_instanced(p_context, idx, inst_cnt, start_idx, base_v, start_i);
}


void __stdcall renderer::d3d11_draw_indexed(ID3D11DeviceContext* p_context, UINT index_count, UINT start_index_location,
                                            INT base_vertex_location)
{
	static UINT stride = 0;

	p_context->IAGetVertexBuffers(0, 1, &ve_buff, &stride, &ve_buffer_offset);

	if (std::get<bool>(menu::config::config_map["enable_chams"]) && roblox::games::game->get_stride() == stride)
	{
		if (roblox::games::game->get_index().has_value() || roblox::games::game->get_indesc().has_value())
			return values::draw_indexed(p_context, index_count, start_index_location, base_vertex_location);



		p_context->OMGetDepthStencilState(&original, nullptr);

		p_context->OMSetDepthStencilState(no_depth, 0);

		values::draw_indexed(p_context, index_count, start_index_location, base_vertex_location);

		p_context->PSSetShader(pixel_shader, nullptr, 0);

		p_context->OMSetDepthStencilState(original, 0);

		original->Release();
		original = nullptr;
	}

	values::draw_indexed(p_context, index_count, start_index_location, base_vertex_location);
}

void __stdcall renderer::d3d11_ps_set_shader_resources_hook(ID3D11DeviceContext* p_context, UINT start_slot,
                                                            UINT num_view,
                                                            ID3D11ShaderResourceView* const* shader_resource_views)
{
	pssr = start_slot;

	values::set_shader_resources_orig(p_context, start_slot, num_view, shader_resource_views);
}

/*std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}*/

std::size_t get_inst_size(std::uintptr_t addr)
{
	auto address = reinterpret_cast<std::uint8_t*>(addr);

	auto current_size = 0;

	while (true)
	{
		const auto size = insn_len_x86_32(address);

		address += size;
		current_size += size;

		if (current_size >= 5)
			break;
	}

	return current_size;
}

void renderer::initialize()
{
	values::roblox_window = FindWindowW(nullptr, L"Roblox");

	if (!values::roblox_window)
		throw std::exception{"Couldn't Find Roblox Window"};

	SetForegroundWindow(values::roblox_window);

	GetWindowRect(values::roblox_window, &values::screen_bounds);

	// Set dpi scaling on launch
	renderer::values::dpi_scale = ImGui_ImplWin32_GetDpiScaleForHwnd(values::roblox_window);

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
	D3D_FEATURE_LEVEL obtained_level;

	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.OutputWindow = values::roblox_window;
		sd.SampleDesc.Count = 1;
		sd.Windowed = true;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.BufferDesc.Width = 1;
		sd.BufferDesc.Height = 1;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 1;
	}

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels,
	                                           sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd,
	                                           &values::global_swapchain, &values::global_device, &obtained_level,
	                                           &values::global_context);

	std::uintptr_t* vt_swapchain;
	memcpy(&vt_swapchain, values::global_swapchain, sizeof(std::uintptr_t));

	DWORD old_protection;
	VirtualProtect(vt_swapchain, sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &old_protection);

	values::d3d11_present = reinterpret_cast<decltype(values::d3d11_present)>(vt_swapchain[8]);
	values::d3d11_resize_buffers = reinterpret_cast<decltype(values::d3d11_resize_buffers)>(vt_swapchain[13]);

	vt_swapchain[13] = reinterpret_cast<std::uintptr_t>(&resize_buffers_hook);

	if (auto handle = GetModuleHandleA("graphics-hook32.dll"); reinterpret_cast<std::uintptr_t>(handle) && handle !=
		INVALID_HANDLE_VALUE)
	{
		const auto start = reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(values::d3d11_present) + *
			reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(values::d3d11_present) + 1) + 5);

		auto i = 0u;

		for (i = 0u; i < 100u; ++i)
		{
			if (start[i] == 0x75 && start[i + 2] == 0x83 && start[i + 3] == 0x3D) //jne cmp dword ptr
			{
				values::d3d11_capture = reinterpret_cast<decltype(values::d3d11_capture)>(tramp_hook(
					**reinterpret_cast<std::uintptr_t**>(start + i + 4), reinterpret_cast<std::uintptr_t>(prestent_cpy),
					6));

				break;
			}
		}
	}
	else
	{
		vt_swapchain[8] = reinterpret_cast<std::uintptr_t>(&present_hook);
	}

	VirtualProtect(vt_swapchain, sizeof(std::uintptr_t), old_protection, &old_protection);

	raw_swapchain_ptr = vt_swapchain;
}

void renderer::initialize_chams()
{
	std::uintptr_t* vt_context;
	memcpy(&vt_context, values::global_context, sizeof(std::uintptr_t));

	values::draw_indexed_instanced = reinterpret_cast<decltype(values::draw_indexed_instanced)>(tramp_hook(
		vt_context[20], reinterpret_cast<std::uintptr_t>(d3d11_draw_indexed_instanced), get_inst_size(vt_context[20])));

	values::draw_indexed = reinterpret_cast<decltype(values::draw_indexed)>(tramp_hook(
		vt_context[12], reinterpret_cast<std::uintptr_t>(d3d11_draw_indexed), get_inst_size(vt_context[12])));
}