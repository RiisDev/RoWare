#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"
#include "../dependencies/imgui/imgui_impl_dx11.h"
#include "../dependencies/imgui/imgui_internal.h"

using present_t = HRESULT(__stdcall*)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);

using resize_buffers_t = HRESULT(__stdcall*)(IDXGISwapChain* this_ptr, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);

using d3d11_capture_t = void(*)(void*, void*);

using d3d11_ps_set_shader_resources = void(__stdcall*)(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView* const*);

using d3d11_draw_indexed_instanced_t = void(__stdcall*)(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

using d3d11_draw_indexed_t = void(__stdcall*)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer::values
{
	extern float dpi_scale;

	extern WNDPROC original_wnd_proc;

	extern DXGI_SWAP_CHAIN_DESC sd;

	extern HWND roblox_window;

	extern HWND window;

	extern RECT screen_bounds;

	extern ID3D11Device* global_device;
	extern ID3D11DeviceContext* global_context;
	extern ID3D11RenderTargetView* main_render_target_view;
	extern ID3D11Texture2D* back_buffer;
	extern IDXGISwapChain* global_swapchain;

	extern present_t d3d11_present;
	extern resize_buffers_t d3d11_resize_buffers;

	extern d3d11_capture_t d3d11_capture;

	extern d3d11_ps_set_shader_resources set_shader_resources_orig;

	extern d3d11_draw_indexed_instanced_t draw_indexed_instanced;

	extern d3d11_draw_indexed_t draw_indexed;

	extern bool hooked;
}

namespace renderer
{
	void render(IDXGISwapChain* swap_chain, void* backbuffer_ptr, bool is_from_present);

	void __cdecl prestent_cpy(void*, void*);

	std::pair<std::int32_t, std::int32_t> get_screen_metrics();

	HRESULT __stdcall present_hook(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags);

	HRESULT __stdcall resize_buffers_hook(IDXGISwapChain* this_ptr, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
	void __stdcall d3d11_ps_set_shader_resources_hook(ID3D11DeviceContext* p_context, UINT start_slot, UINT num_view, ID3D11ShaderResourceView* const* shader_resource_views);

	void __stdcall d3d11_draw_indexed_instanced(ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT);

	void __stdcall d3d11_draw_indexed(ID3D11DeviceContext*, UINT, UINT, INT);

	void initialize();

	void initialize_chams();

	inline bool is_roblox_on_top() { return renderer::values::roblox_window == GetForegroundWindow(); }
}