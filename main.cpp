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
};

WindowStuff windowStuff;
RECT rect = {};

constexpr char windowClassName[] = "3D-Renderer";
constexpr int START_WIDTH = 1920, START_HEIGHT = 1080;
int windowWidth, windowHeight;

void onIdle(int w, int h, WindowBuffer& windowBuffer) {
    windowBuffer.clear();

    /*
    for (int i = 0; i < gameWindowBuffer.w; i++) {
        for (int j = 0; j < gameWindowBuffer.h; j++) {
            gameWindowBuffer.drawAtSafe(i, j, i % 256, j % 256, (i * j) % 256);
        }
    }
    */
    // Iterate over all WireFrames, draw to screen, rotate, and record updates
    for (WireFrame& wireFrame : windowStuff.wireFrames) {
        windowBuffer.drawCube(wireFrame);
        wireFrame.rotate();
        if (windowStuff.playback.recording()) {
            windowStuff.playback.update(wireFrame);
        }
    }
}

// Win32 function for event handling
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // Resize window handling
        case WM_SIZE: {
            GetClientRect(hwnd, &rect);
            windowHeight = rect.bottom;
            windowWidth = rect.right;
            resetWindowBuffer(&windowStuff.windowBuffer, &windowStuff.bitmapInfo, hwnd);
            break;
        }
        // Redraw window handling
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC DeviceContext = BeginPaint(hwnd, &ps);

            HDC hdc = GetDC(hwnd);

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
                    windowStuff.wireFrames.emplace_back(coord{400, 300, 1000}, 100, 100, 100);
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
            switch (wParam) {
                case VK_ESCAPE:
                    if (MessageBox(hwnd, "Quit the program?", "WARNING!", MB_YESNO) == IDYES) {
                        windowStuff.running = false;
                    }
                    break;
                // Cube movement cases up/down/left/right
                case VK_UP:
                    windowStuff.wireFrames[0].updateLocation({0, -10, 0});
                    break;
                case VK_DOWN:
                    windowStuff.wireFrames[0].updateLocation({0, 10, 0});
                    break;
                case VK_LEFT:
                    windowStuff.wireFrames[0].updateLocation({-10, 0, 0});
                    break;
                case VK_RIGHT:
                    windowStuff.wireFrames[0].updateLocation({10, 0, 0});
                    break;
                // Cube toggle rotations
                case 88: // X
                    windowStuff.wireFrames[0].toggleRotation(rotateX);
                    break;
                case 89: // Y
                    windowStuff.wireFrames[0].toggleRotation(rotateY);
                    break;
                case 90: // Z
                    windowStuff.wireFrames[0].toggleRotation(rotateZ);
                    break;
                default:
                    std::cout << wParam << '\n';
            }
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

    resetWindowBuffer(&windowStuff.windowBuffer, &windowStuff.bitmapInfo, hwnd);

    if (hwnd == nullptr) {
        MessageBox(nullptr, "Window Creation Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    Playback::replay(hwnd, windowStuff.windowBuffer, lpCmdLine);

    windowStuff.wireFrames.emplace_back(coord{580, 280, 500}, 100, 100, 100);
    // "Game" Loop
    while (windowStuff.running) {
        if (PeekMessage(&Msg, hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        } else {
            onIdle(windowWidth, windowHeight, windowStuff.windowBuffer);
            SendMessage(hwnd, WM_PAINT, 0, 0);
        }
    }

    return Msg.wParam;
}