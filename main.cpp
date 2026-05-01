#include <atomic>
#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <thread>
#include <vector>

#include "resources/resource.h"
#include "extra/wireFrame.h"
#include "extra/windowBuffer.h"
#include "extra/playback.h"


std::atomic running = true;
CRITICAL_SECTION bufferLock;
CRITICAL_SECTION vectorLock;

struct WindowStuff {

    BITMAPINFO bitmapInfo = {};
    WindowBuffer windowBuffer = {};
    std::vector<WireFrame> wireFrames = {};
    Playback playback;
    HWND hwnd;
    bool keyPressed[256] = {false};
    bool keyPressedPrev[256] = {false};
};

WindowStuff windowStuff;

constexpr double TPS = 60;
constexpr bool LIMIT_TPS = true;
constexpr bool SHOW_FPS = false;
bool ANTI_ALIAS = false;
constexpr char windowClassName[] = "3D-Renderer";
constexpr int START_WIDTH = 1920, START_HEIGHT = 1080;
constexpr float moveAccel = 1.2f;   // Should be refactored to WireFrame
constexpr float maxSpeed = 4.0f;    // Should be refactored to WireFrame
float moveDistances[4];             // Should be refactored to WireFrame

std::string cubeFile = R"(..\extra\base-objs\cube.obj)";
std::string cylFile = R"(..\extra\base-objs\cylinder.obj)";
std::string sphereFile = R"(..\extra\base-objs\sphere.obj)";
std::string testFile = R"(..\extra\base-objs\test.obj)";
std::string pyramidFile = R"(..\extra\base-objs\pyramid.obj)";
WireFrame* selected = nullptr;
std::vector<WireFrame*> multiSelect;

void pressKeys() {
    if (windowStuff.keyPressed[VK_ESCAPE]) {
        if (MessageBox(windowStuff.hwnd, "Quit the program?", "WARNING!", MB_YESNO) == IDYES) {
            running = false;
        }
        windowStuff.keyPressed[VK_ESCAPE] = false;
    }

    if (windowStuff.keyPressed['A'] && !windowStuff.keyPressedPrev['A']) {
        windowStuff.keyPressedPrev['A'] = true;
        if (ANTI_ALIAS) ANTI_ALIAS = false;
        else ANTI_ALIAS = true;
    }


    // WireFrame Handling
    if (selected == nullptr) return;
    // Delete WireFrame
    if (windowStuff.keyPressed[VK_DELETE]) {
        EnterCriticalSection(&vectorLock);
        selected->isExpired = true;     // Expire current wireFrame
        LeaveCriticalSection(&vectorLock);

        selected = nullptr;
        return;
    }
    // Cube movement Left/Up/Right/Down
    // VK_LEFT is the first of the arrow key macros in Win32
    // This was designed this way to reduce the repetitive code
    for (int i = 0; i < 4; i++) {
        if (!windowStuff.keyPressedPrev[i + VK_LEFT]) moveDistances[i] = 1.0;
        if (windowStuff.keyPressed[i + VK_LEFT]) {
            windowStuff.keyPressedPrev[i + VK_LEFT] = true;
            if (moveDistances[i] < 1 + maxSpeed) {
                moveDistances[i] *= moveAccel;
            }
        }
    }
    // Cube rotation toggles
    // This conditional ensures that the key isn't triggered more than once
    if (windowStuff.keyPressed['X'] && !windowStuff.keyPressedPrev['X']) {  // x
        windowStuff.keyPressedPrev['X'] = true;
        selected->toggleRotation(rotateX);
    }
    if (windowStuff.keyPressed['Y'] && !windowStuff.keyPressedPrev['Y']) {  // y
        windowStuff.keyPressedPrev['Y'] = true;
        selected->toggleRotation(rotateY);
    }
    if (windowStuff.keyPressed['Z'] && !windowStuff.keyPressedPrev['Z']) {  // z
        windowStuff.keyPressedPrev['Z'] = true;
        selected->toggleRotation(rotateZ);
    }
}

void renderThreadProc() {
    // Capture initial frequency
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    // Capture initial time
    LARGE_INTEGER lastTime, currentTime;
    QueryPerformanceCounter(&lastTime);

    float accumulator = 0;

    while (running) {
        QueryPerformanceCounter(&currentTime);
        float deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / freq.QuadPart;
        lastTime = currentTime;

        accumulator += deltaTime;

        if constexpr (SHOW_FPS) std::cout << "FPS: " << 1 / deltaTime << '\n';

        EnterCriticalSection(&bufferLock);
        windowStuff.windowBuffer.clearToBlack();
        LeaveCriticalSection(&bufferLock);

        // Iterate over all WireFrames, draw to screen, rotate, and record updates
        for (WireFrame& wireFrame : windowStuff.wireFrames) {
            if (wireFrame.isExpired) continue;

            wireFrame.rotate(deltaTime, TPS);
            if (&wireFrame == selected && accumulator >= 1 / TPS) {
                accumulator = 0.0f;
                wireFrame.updateLocation({moveDistances[2] - moveDistances[0], moveDistances[1] - moveDistances[3], 0});
            }
            // Draw
            EnterCriticalSection(&bufferLock);
            windowStuff.windowBuffer.drawWireframe(wireFrame);
            LeaveCriticalSection(&bufferLock);
            // if (windowStuff.playback.recording()) {
            //     windowStuff.playback.update(wireFrame);
            // }
        }

        /* Remove expired wireFrames */
        EnterCriticalSection(&vectorLock);
        std::erase_if(windowStuff.wireFrames, [](const WireFrame& wireFrame) {
            return wireFrame.isExpired;
        });
        LeaveCriticalSection(&vectorLock);

        // Lock the thread;
        EnterCriticalSection(&bufferLock);

        // Apply anti-aliasing
        if (ANTI_ALIAS) windowStuff.windowBuffer.FXAA();

        if (HDC hdc = GetDC(windowStuff.hwnd)) {
            SetStretchBltMode(hdc, COLORONCOLOR);

            StretchDIBits(hdc,
                          0, 0, windowStuff.windowBuffer.w, windowStuff.windowBuffer.h,
                          0, 0, windowStuff.windowBuffer.w, windowStuff.windowBuffer.h,
                          windowStuff.windowBuffer.memory, &windowStuff.bitmapInfo,
                          DIB_RGB_COLORS, SRCCOPY
                          );

            ReleaseDC(windowStuff.hwnd, hdc);
        }

        LeaveCriticalSection(&bufferLock);

        ValidateRect(windowStuff.hwnd, nullptr);

        if constexpr (LIMIT_TPS) if (deltaTime < 1 / TPS) Sleep(1);
    }
}

// Win32 function for event handling
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // Resize window handling
        case WM_SIZE: {
            EnterCriticalSection(&bufferLock);

            resetWindowBuffer(&windowStuff.windowBuffer, &windowStuff.bitmapInfo, hwnd);

            LeaveCriticalSection(&bufferLock);
            break;
        }
        case WM_SIZING: {
            // Stop windows from doing something
            return TRUE;
        }
        case WM_ERASEBKGND: {
            // Stop windows from doing something
            return 1;
        }
        // Redraw window handling
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            EndPaint(hwnd, &ps);
            break;
        }
        /* Doesn't Work Right
        case WM_CREATE: {
            HMENU hMenu, hSubMenu;
            HICON hIcon, hIconSm;

            hMenu = CreateMenu();
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "E&xit");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

            hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, ID_STUFF_GO, "&Go");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Stuff");

            SetMenu(hwnd, hMenu);

            hIcon = LoadImage(NULL, "menu_two.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
            if (hIcon) {
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            } else {
                MessageBox(hwnd, "Could not load large icon!", "Error", MB_OK | MB_ICONERROR);
            }

            hIconSm = LoadImage(NULL, "menu_two.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
            if (hIconSm) {
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            } else {
                MessageBox(hwnd, "Could not load small icon!", "Error", MB_OK | MB_ICONERROR);
            }
            break;
        }
        */
        // Menu bar handling
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case ID_FILE_NEW_CUBE: {
                    // DialogBox(nullptr, MAKEINTRESOURCE(IDD_MYDIALOG), hwnd, (DLGPROC)DeleteItemProc);
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(cubeFile);
                    LeaveCriticalSection(&vectorLock);

                    selected = &windowStuff.wireFrames.back();
                    break;
                }
                case ID_FILE_NEW_SPHERE: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(sphereFile);
                    LeaveCriticalSection(&vectorLock);

                    selected = &windowStuff.wireFrames.back();
                    break;
                }
                case ID_FILE_NEW_CYLINDER: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(cylFile);
                    LeaveCriticalSection(&vectorLock);

                    selected = &windowStuff.wireFrames.back();
                    break;
                }
                case ID_FILE_NEW_PYRAMID: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(pyramidFile);
                    LeaveCriticalSection(&vectorLock);

                    selected = &windowStuff.wireFrames.back();
                    break;
                }
                case ID_FILE_RECORD_TEN: {
                    windowStuff.playback.startRecord(10);
                    break;
                }
                case ID_FILE_RECORD_FIFTEEN: {
                    windowStuff.playback.startRecord(15);
                    break;
                }
                case ID_FILE_RECORD_THIRTY: {
                    windowStuff.playback.startRecord(30);
                    break;
                }
                /* Does nothing currently
                case ID_FILE_PLAY: {
                    break;
                }
                */
                case ID_FILE_EXIT:
                    if (MessageBox(hwnd, "Are you sure?", "WARNING!", MB_YESNO) == IDYES) {
                        running = false;
                    }
                    break;
                default:
                    std::cout << LOWORD(wParam) << '\n';
            }
            break;
        }
        // Keyboard handling
        case WM_KEYDOWN: {
            windowStuff.keyPressed[wParam] = true;
            break;
        }
        case WM_KEYUP: {
            windowStuff.keyPressed[wParam] = false;
            windowStuff.keyPressedPrev[wParam] = false;
            break;
        }
        // Scroll wheel handling
        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta > 0) {
                if (selected == nullptr) {
                    for (auto& wireFrame : windowStuff.wireFrames) {
                        wireFrame.updateLocation({0, 0, -20});
                    }
                } else {
                    selected->updateLocation({0, 0, -20});
                }
            } else if (delta < 0) {
                if (selected == nullptr) {
                    for (auto& wireFrame : windowStuff.wireFrames) {
                        wireFrame.updateLocation({0, 0, 20});
                    }
                } else {
                    selected->updateLocation({0, 0, 20});
                }
            }
            break;
        }
        // Left click handling
        case WM_LBUTTONDOWN: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            std::cout << "{" << xPos << ", " << yPos << "}" << '\n';

            if (windowStuff.wireFrames.empty()) break;

            bool noHits = true;
            for (auto& wireFrame : windowStuff.wireFrames) {
                vec3 coords = windowStuff.windowBuffer.raycast.project(
                    {xPos, yPos}, wireFrame.min.z);
                // std::cout << '{' << coords.x << ", " << coords.y << ", " << coords.z << "}\n";
                if (wireFrame.intersects(coords)) {
                    selected = &wireFrame;
                    noHits = false;
                    // std::cout << "Hit!\n";
                }
            }
            if (noHits) selected = nullptr;

            break;
        }
        case WM_CLOSE:
            running = false;
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    InitializeCriticalSection(&bufferLock);
    InitializeCriticalSection(&vectorLock);

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // Register Window Class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_MYICON));
    wc.hIconSm = (HICON)LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);
    wc.lpszClassName = windowClassName;

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create Window
    hwnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            "3D Renderer",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, START_WIDTH, START_HEIGHT,
            nullptr, nullptr, hInstance, nullptr
            );

    windowStuff.hwnd = hwnd;

    if (hwnd == nullptr) {
        MessageBox(nullptr, "Window Creation Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Playback::replay(hwnd, windowStuff.windowBuffer, lpCmdLine);

    std::thread renderThread(renderThreadProc);

    while (running) {
        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        pressKeys();
    }

    if (renderThread.joinable()) {
        renderThread.join();
    }

    DeleteCriticalSection(&bufferLock);
    return Msg.wParam;
}