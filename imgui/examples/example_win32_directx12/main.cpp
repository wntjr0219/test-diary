#include "imgui.h"
#include "use/imgui_impl_win32.h"
#include "use/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include <string>
#include <fstream>
#include <iostream>
//#include <mysql/mysql.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device*                g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap*        g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap*        g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue*          g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList*   g_pd3dCommandList = nullptr;
static ID3D12Fence*                 g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3*             g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource*              g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX12 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


  

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    
    // ����
    bool save_window = false;
    bool load_window = false;

    bool m_save_window = false;
    bool monster_load_window = false;
    bool pattern_window = false;


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // default
        //static float f = 0.0f;
        static int P_Hp = 500; // 0~1000
        static int P_Damage = 50;
        static int P_DefDegree = 50;
        static int P_FarringSpeed = 50;
        static int P_AvoidDecision = 50;
        static int P_AttackSpeed = 50;
        static int P_Speed = 50;
        static int P_RunningSpeed = 50;

        static bool key_merge = true;
        static bool use_jump = true;

        static int shield = 1;
        static int weapon = 1;

        static char PlayerID[20];
        static char PresetName[20];
       
        
        // ���� ui
        {
         
            ImGui::Begin("player preset");                         
                
            ImGui::Text("player preset");

            ImGui::SetCursorPos(ImVec2(200, 50)); 
            ImGui::Text("hp");
            ImGui::SliderInt("##hp", &P_Hp, 0, 1000);

            ImGui::SetCursorPos(ImVec2(200, 100));
            ImGui::Text("damage");
            ImGui::SliderInt("##damage", &P_Damage, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 150));
            ImGui::Text("def_degree");
            ImGui::SliderInt("##def_degree", &P_DefDegree, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 200));
            ImGui::Text("farring_speed");
            ImGui::SliderInt("##farring_speed", &P_FarringSpeed, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 250));
            ImGui::Text("avoid_decision");
            ImGui::SliderInt("##avoid_decision", &P_AvoidDecision, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 300));
            ImGui::Text("attack_speed");
            ImGui::SliderInt("##attack_speed", &P_AttackSpeed, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 350));
            ImGui::Text("speed");
            ImGui::SliderInt("##speed", &P_Speed, 0, 100);

            ImGui::SetCursorPos(ImVec2(200, 400));
            ImGui::Text("running_speed");
            ImGui::SliderInt("##running_speed", &P_RunningSpeed, 0, 100);

            //
            ImGui::Checkbox("key merge", &key_merge);
            ImGui::Checkbox("use_jump", &use_jump);

            //
            ImGui::Text("shield");
            ImGui::RadioButton("small##shield", &shield, 0);
            ImGui::SameLine();
            ImGui::RadioButton("midium##shield", &shield, 1);
            ImGui::SameLine();
            ImGui::RadioButton("big##shield", &shield, 2);

            ImGui::Text("weapon");
            ImGui::RadioButton("small##weapon", &weapon, 0);  // weapon��ư ��������
            ImGui::SameLine();
            ImGui::RadioButton("midium##weapon", &weapon, 1);
            ImGui::SameLine();
            ImGui::RadioButton("big##weapon", &weapon, 2);

            //
            if (ImGui::Button("save"))
            {
                save_window = true; // â �ٲ�� �� ��
            }

            ImGui::SameLine();
            if (ImGui::Button("load"))
            {
               load_window = true;
            }

     
            ImGui::End();
        }

        std::string save_filename;

        if (save_window)
        {
            ImGui::Begin("Save Preset", &save_window);
            ImGui::Text("Enter filename to save:");

            static char filename_buf[256] = { 0 }; // ���� ũ��� �ʿ信 ���� ���� ����

            ImGui::InputText("##filename", filename_buf, IM_ARRAYSIZE(filename_buf));

            if (ImGui::Button("Save") && filename_buf[0] != '\0')
            {
                save_filename = filename_buf;

                // ���⿡ ������ �����ϴ� �ڵ� �߰�
                std::ofstream file(save_filename.c_str());
                if (file.is_open())
                {
                    file << "hp: " << P_Hp << std::endl;
                    file << "damage: " << P_Damage << std::endl;
                    file << "def_degree: " << P_DefDegree << std::endl;
                    file << "farring_speed: " << P_FarringSpeed << std::endl;
                    file << "avoid_decision: " << P_AvoidDecision << std::endl;
                    file << "attack_speed: " << P_AttackSpeed << std::endl;
                    file << "speed: " << P_Speed << std::endl;
                    file << "running_speed: " << P_RunningSpeed << std::endl;

                    file << "key_merge: " << key_merge << std::endl;
                    file << "use_jump: " << use_jump << std::endl;

                    file << "shield: " << shield << std::endl;
                    file << "weapon: " << weapon << std::endl;

                    file.close();
                }
                else
                {
                    // ���� ���� ���� ó��
                }

                // ���� �� â �ݱ�
                save_window = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                save_window = false;
            }

            ImGui::End();
        }


        if (load_window)
        {
            static char filename_buf[256] = { 0 }; // ���ϸ��� �Է��� ����
            static char textBuffer[256] = { 0 }; // ���� ������ ����� ����

            ImGui::Begin("Load Preset", &load_window);
            ImGui::Text("Enter filename to load:");

            ImGui::InputText("##filename", filename_buf, IM_ARRAYSIZE(filename_buf));

            if (ImGui::Button("Load") && filename_buf[0] != '\0')
            {
                std::ifstream file(filename_buf);
                if (file.is_open())
                {
                    std::string line;
                    while (std::getline(file, line))
                    {
                        // ':'�� �������� ���ڿ��� �Ľ��Ͽ� ������ �Ҵ�
                        std::size_t pos = line.find(':');
                        if (pos != std::string::npos)
                        {
                            std::string key = line.substr(0, pos); // ������
                            std::string value_str = line.substr(pos + 1); // ��

                            // ���� ����
                            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
                            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());

                            // ���� ���ڷ� ��ȯ�Ͽ� ������ �Ҵ�
                            if (key == "hp")
                            {
                                P_Hp = std::stoi(value_str);
                            }
                            else if (key == "damage")
                            {
                                P_Damage = std::stoi(value_str);
                            }
                            else if (key == "def_degree")
                            {
                                P_DefDegree = std::stoi(value_str);
                            }
                            else if (key == "farring_speed")
                            {
                                P_FarringSpeed = std::stoi(value_str);
                            }
                            else if (key == "avoid_decision")
                            {
                                P_AvoidDecision = std::stoi(value_str);
                            }

                            else if (key == "attack_speed")
                            {
                                P_AttackSpeed = std::stoi(value_str);
                            }
                            else if (key == "speed")
                            {
                                P_Speed = std::stoi(value_str);
                            }
                            else if (key == "running_speed")
                            {
                                P_RunningSpeed = std::stoi(value_str);
                            }
                            else if (key == "key_merge")
                            {
                                key_merge = std::stoi(value_str);
                            }
                            else if (key == "use_jump")
                            {
                                use_jump = std::stoi(value_str);
                            }
                            else if (key == "shield")
                            {
                                shield = std::stoi(value_str);
                            }
                            else if (key == "weapon")
                            {
                                weapon = std::stoi(value_str);
                            }
                        }
                    }
                    file.close();
                }
                else
                {
                    // ���� ���� ���� ó��
                }
            }

            ImGui::InputTextMultiline("##filecontents", textBuffer, IM_ARRAYSIZE(textBuffer), ImVec2(300, 200), ImGuiInputTextFlags_ReadOnly);

            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                load_window = false;
            }

            ImGui::End();
        }

      {
            static int monster = 1;
            static int m_hp = 500;
            static int m_damage = 50;
            static int m_attack_speed = 50;
            static int a1 = 50;
            static int a2 = 50;
            static int a3 = 50;
            static int a4 = 50;
            static int a5 = 50;
            static int a6 = 50;
            static int a7 = 50;
            static int a8 = 50;
            static int a9 = 50;

            // monster
            {


                ImGui::Begin("monster preset");

                ImGui::Text("monster preset");

                ImGui::SetCursorPos(ImVec2(250, 50));
                ImGui::Text("hp");
                ImGui::SliderInt("##hp", &P_Hp, 0, 1000);

                ImGui::SetCursorPos(ImVec2(250, 100));
                ImGui::Text("damage");
                ImGui::SliderInt("##damage", &P_Damage, 0, 100);

                ImGui::SetCursorPos(ImVec2(250, 150));
                ImGui::Text("attack_speed");
                ImGui::SliderInt("##attack_speed", &P_AttackSpeed, 0, 100);



                //
                ImGui::Text("monster");
                ImGui::RadioButton("small##monster", &monster, 0);
                ImGui::SameLine();
                ImGui::RadioButton("midium##monster", &monster, 1);
                ImGui::SameLine();
                ImGui::RadioButton("big##monster", &monster, 2);

                //
                ImGui::Text("pattern");
                switch (monster)
                {
                case 0:
                    ImGui::SetCursorPos(ImVec2(250, 300));
                    ImGui::Text("a1");
                    ImGui::SliderInt("##a1", &a1, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 350));
                    ImGui::Text("a2");
                    ImGui::SliderInt("##a2", &a2, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 400));
                    ImGui::Text("a3");
                    ImGui::SliderInt("##a3", &a3, 0, 100);
                    break;
                case 1:
                    ImGui::SetCursorPos(ImVec2(250, 300));
                    ImGui::Text("a4");
                    ImGui::SliderInt("##a4", &a4, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 350));
                    ImGui::Text("a5");
                    ImGui::SliderInt("##a5", &a5, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 400));
                    ImGui::Text("a6");
                    ImGui::SliderInt("##a6", &a6, 0, 100);
                    break;
                case 2:
                    ImGui::SetCursorPos(ImVec2(250, 300));
                    ImGui::Text("a7");
                    ImGui::SliderInt("##a7", &a7, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 350));
                    ImGui::Text("a8");
                    ImGui::SliderInt("##a8", &a8, 0, 100);

                    ImGui::SetCursorPos(ImVec2(250, 400));
                    ImGui::Text("a9");
                    ImGui::SliderInt("##a9", &a9, 0, 100);
                    break;
                default:
                    break;
                }

                if (ImGui::Button("save"))
                {
                    save_window = true; // â �ٲ�� �� ��
                }

                ImGui::SameLine();
                if (ImGui::Button("load"))
                {
                    load_window = true;
                }



                ImGui::End();
            }


            std::string m_save_filename;

            if (m_save_window)
            {
                ImGui::Begin("Save Preset##monster", &m_save_window);
                ImGui::Text("Enter filename to save:");

                static char filename_buf[256] = { 0 }; // ���� ũ��� �ʿ信 ���� ���� ����

                ImGui::InputText("##filename_monster", filename_buf, IM_ARRAYSIZE(filename_buf));

                if (ImGui::Button("Save") && filename_buf[0] != '\0')
                {
                    m_save_filename = filename_buf;

                    // ���⿡ ������ �����ϴ� �ڵ� �߰�
                    std::ofstream file(m_save_filename.c_str());
                    if (file.is_open())
                    {
                        file << "hp: " << P_Hp << std::endl;
                        file << "damage: " << P_Damage << std::endl;
                        file << "attack_speed: " << P_AttackSpeed << std::endl;


                        file << "a1" << a1 << std::endl;
                        file << "a2" << a2 << std::endl;
                        file << "a3" << a3 << std::endl;
                        file << "a4" << a4 << std::endl;
                        file << "a5" << a5 << std::endl;
                        file << "a6" << a6 << std::endl;
                        file << "a7" << a7 << std::endl;
                        file << "a8" << a8 << std::endl;
                        file << "a9" << a9 << std::endl;





                        file.close();
                    }
                    else
                    {
                        // ���� ���� ���� ó��
                    }

                    // ���� �� â �ݱ�
                    save_window = false;
                }

                ImGui::SameLine();
                if (ImGui::Button("Close"))
                {
                    save_window = false;
                }

                ImGui::End();
            }


            if (load_window)
            {
                static char filename_buf[256] = { 0 }; // ���ϸ��� �Է��� ����
                static char textBuffer[256] = { 0 }; // ���� ������ ����� ����

                ImGui::Begin("Load Preset", &load_window);
                ImGui::Text("Enter filename to load:");

                ImGui::InputText("##filename", filename_buf, IM_ARRAYSIZE(filename_buf));

                if (ImGui::Button("Load") && filename_buf[0] != '\0')
                {
                    std::ifstream file(filename_buf);
                    if (file.is_open())
                    {
                        std::string line;
                        while (std::getline(file, line))
                        {
                            // ':'�� �������� ���ڿ��� �Ľ��Ͽ� ������ �Ҵ�
                            std::size_t pos = line.find(':');
                            if (pos != std::string::npos)
                            {
                                std::string key = line.substr(0, pos); // ������
                                std::string value_str = line.substr(pos + 1); // ��

                                // ���� ����
                                key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
                                value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());

                                // ���� ���ڷ� ��ȯ�Ͽ� ������ �Ҵ�
                                if (key == "hp")
                                {
                                    P_Hp = std::stoi(value_str);
                                }
                                else if (key == "damage")
                                {
                                    P_Damage = std::stoi(value_str);
                                }
                            
                                else if (key == "attack_speed")
                                {
                                    P_AttackSpeed = std::stoi(value_str);
                                }
                                else if (key == "a1")
                                {
                                    a1 = std::stoi(value_str);
                                }

                                else if (key == "a2")
                                {
                                    a2 = std::stoi(value_str);
                                }
                                else if (key == "a3")
                                {
                                    a3 = std::stoi(value_str);
                                }
                                else if (key == "a4")
                                {
                                    a4 = std::stoi(value_str);
                                }
                                else if (key == "a5")
                                {
                                    a5 = std::stoi(value_str);
                                }
                                else if (key == "a6")
                                {
                                    a6 = std::stoi(value_str);
                                }
                                else if (key == "a7")
                                {
                                    a7 = std::stoi(value_str);
                                }
                                else if (key == "a8")
                                {
                                    a8 = std::stoi(value_str);
                                }
                                else if (key == "a9")
                                {
                                    a9 = std::stoi(value_str);
                                }

                            }
                        }
                        file.close();
                    }
                    else
                    {
                        // ���� ���� ���� ó��
                    }
                }

                ImGui::InputTextMultiline("##filecontents", textBuffer, IM_ARRAYSIZE(textBuffer), ImVec2(300, 200), ImGuiInputTextFlags_ReadOnly);

                ImGui::SameLine();
                if (ImGui::Button("Close"))
                {
                    load_window = false;
                }

                ImGui::End();
            }
        }
        //

      
        // Rendering
        ImGui::Render();

        FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();


        ID3D12DescriptorHeap* ppHeaps[] = { g_pd3dSrvDescHeap };
        g_pd3dCommandList->SetDescriptorHeaps(ARRAYSIZE(ppHeaps), ppHeaps);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

