#include <windows.h>
#include <iostream>
#include <vector>

#include "resources/resource.h"
#include "extra/wireFrame.h"
#include "extra/windowBuffer.h"
#include "extra/playback.h"

struct WindowStuff {
    bool running = true;

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
constexpr char windowClassName[] = "3D-Renderer";
constexpr int START_WIDTH = 1920, START_HEIGHT = 1080;
constexpr float moveAccel = 1.2f;   // Should be refactored to WireFrame
constexpr float maxSpeed = 4.0f;    // Should be refactored to WireFrame
float moveDistances[4];             // Should be refactored to WireFrame

std::string cubeFile = R"(..\extra\base-objs\cube.obj)";
std::string cylFile = R"(..\extra\base-objs\cylinder.obj)";
std::string sphereFile = R"(..\extra\base-objs\sphere.obj)";
std::string testFile = R"(..\extra\base-objs\test.obj)";
WireFrame cube{sphereFile};

void pressKeys() {
    if (windowStuff.keyPressed[VK_ESCAPE]) {
        if (MessageBox(windowStuff.hwnd, "Quit the program?", "WARNING!", MB_YESNO) == IDYES) {
            windowStuff.running = false;
        }
    }
    // Reset wireFrame
    if (!windowStuff.keyPressedPrev['R'] && windowStuff.keyPressed['R']) {
        windowStuff.keyPressedPrev['R'] = true;
        windowStuff.wireFrames[0] = {cubeFile};
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
    // Update the cube location
    windowStuff.wireFrames[0].updateLocation({moveDistances[2] - moveDistances[0], moveDistances[1] - moveDistances[3], 0});
    // Cube rotation toggles
    // This conditional ensures that the key isn't triggered more than once
    if (!windowStuff.keyPressedPrev['X'] && windowStuff.keyPressed['X']) {  // x
        windowStuff.keyPressedPrev['X'] = true;
        windowStuff.wireFrames[0].toggleRotation(rotateX);
    }
    if (!windowStuff.keyPressedPrev['Y'] && windowStuff.keyPressed['Y']) {  // y
        windowStuff.keyPressedPrev['Y'] = true;
        windowStuff.wireFrames[0].toggleRotation(rotateY);
    }
    if (!windowStuff.keyPressedPrev['Z'] && windowStuff.keyPressed['Z']) {  // z
        windowStuff.keyPressedPrev['Z'] = true;
        windowStuff.wireFrames[0].toggleRotation(rotateZ);
    }
}

void onIdle() {
    pressKeys();
    windowStuff.windowBuffer.clearToBlack();

    // Iterate over all WireFrames, draw to screen, rotate, and record updates
    for (WireFrame& wireFrame : windowStuff.wireFrames) {
        windowStuff.windowBuffer.drawWireframe(wireFrame);
        wireFrame.rotate();
        // if (windowStuff.playback.recording()) {
        //     windowStuff.playback.update(wireFrame);
        // }
    }
}

// Win32 function for event handling
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // Resize window handling
        case WM_SIZE: {
            resetWindowBuffer(&windowStuff.windowBuffer, &windowStuff.bitmapInfo, hwnd);
            break;
        }
        case WM_SIZING: {
            InvalidateRect(hwnd, NULL, FALSE);
            return TRUE;
        }
        // Redraw window handling
        case WM_PAINT: {
            // Move all this into a RenderFrame function that will calculate the objects
            // data and also draw to the screen
            PAINTSTRUCT ps;
            HDC DeviceContext = BeginPaint(hwnd, &ps);

            HDC hdc = GetDC(hwnd);

            SetStretchBltMode(hdc, COLORONCOLOR);

            StretchDIBits(hdc,
                          0, 0, windowStuff.windowBuffer.w, windowStuff.windowBuffer.h,
                          0, 0, windowStuff.windowBuffer.w, windowStuff.windowBuffer.h,
                          windowStuff.windowBuffer.memory, &windowStuff.bitmapInfo,
                          DIB_RGB_COLORS, SRCCOPY
                          );

            ReleaseDC(hwnd, hdc);

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
                    // windowStuff.wireFrames.emplace_back(coord{400, 300, 1000}, 100, 100, 100);
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
                        windowStuff.running = false;
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
                windowStuff.wireFrames[0].updateLocation({0, 0, -20});
            }
            if (delta < 0) {
                windowStuff.wireFrames[0].updateLocation({0, 0, 20});
            }
            break;
        }
        case WM_CLOSE:
            windowStuff.running = false;
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
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, START_WIDTH, START_HEIGHT,
            nullptr, nullptr, hInstance, nullptr
            );

    windowStuff.hwnd = hwnd;
    resetWindowBuffer(&windowStuff.windowBuffer, &windowStuff.bitmapInfo, hwnd);

    if (hwnd == nullptr) {
        MessageBox(nullptr, "Window Creation Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Playback::replay(hwnd, windowStuff.windowBuffer, lpCmdLine);

    windowStuff.wireFrames.emplace_back(cube);  // Add wireframe

    // Capture initial frequency
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    double secondsPerCount = 1.0 / static_cast<double>(freq.QuadPart);
    double ticksPerAction = static_cast<double>(freq.QuadPart) / TPS;

    // Capture initial time
    LARGE_INTEGER lastTime, currentTime;
    QueryPerformanceCounter(&lastTime);

    // "Game" Loop
    while (windowStuff.running) {
        // Process ALL messages at once
        while (PeekMessage(&Msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        if (windowStuff.running) {
            // Wait until tick interval is met
            QueryPerformanceCounter(&currentTime);

            if (LIMIT_TPS && (currentTime.QuadPart - lastTime.QuadPart) >= ticksPerAction) {
                if (SHOW_FPS) {
                    std::cout << freq.QuadPart / (currentTime.QuadPart - lastTime.QuadPart) << '\n';  // Output fps
                }
                onIdle();
                SendMessage(hwnd, WM_PAINT, 0, 0);
                lastTime = currentTime;
            }
        }
    }

    return Msg.wParam;
}