#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool     ImGui_ImplWin32_Init(void* hwnd);
IMGUI_IMPL_API bool     ImGui_ImplWin32_InitForOpenGL(void* hwnd);
IMGUI_IMPL_API void     ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplWin32_NewFrame();


#if 0
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

IMGUI_IMPL_API void     ImGui_ImplWin32_EnableDpiAwareness();
IMGUI_IMPL_API float    ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
IMGUI_IMPL_API float    ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor

IMGUI_IMPL_API void     ImGui_ImplWin32_EnableAlphaCompositing(void* hwnd);   // HWND hwnd

#endif // #ifndef IMGUI_DISABLE
