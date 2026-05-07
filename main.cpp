#include <algorithm>
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
constexpr float maxSpeed = 3.0f;    // Should be refactored to WireFrame
vec3 moveVelocity{0, 0, 0};

std::string cubeFile = R"(..\extra\base-objs\cube.obj)";
std::string cylFile = R"(..\extra\base-objs\cylinder.obj)";
std::string sphereFile = R"(..\extra\base-objs\sphere.obj)";
std::string floorFile = R"(..\extra\base-objs\floor.obj)";
std::string testFile = R"(..\extra\base-objs\test.obj)";
std::string pyramidFile = R"(..\extra\base-objs\pyramid.obj)";
std::vector<WireFrame*> selected;
auto lastMove = std::chrono::steady_clock::now();

bool isSelected(const WireFrame& wireFrame) {
    return std::ranges::find(selected, &wireFrame) != selected.end();
}

/* Returns true if the key was toggled into the pressed position */
bool toggleKey(const unsigned char key) {
    if (windowStuff.keyPressed[key] && !windowStuff.keyPressedPrev[key]) {
        windowStuff.keyPressedPrev[key] = true;
        return true;
    }
    return false;
}

bool keyPressed(const unsigned char key) { return windowStuff.keyPressed[key]; }

bool keyPressedPrev(const unsigned char key) { return windowStuff.keyPressedPrev[key]; }

void keyHandling() {
    using namespace std::chrono_literals;
    if (std::chrono::steady_clock::now() - lastMove > 50ms) {
        moveVelocity.z = 0;
    }

    if (windowStuff.keyPressed[VK_ESCAPE]) {
        if (MessageBox(windowStuff.hwnd, "Quit the program?", "WARNING!", MB_YESNO) == IDYES) {
            running = false;
        }
        windowStuff.keyPressed[VK_ESCAPE] = false;
    }

    // /* Anti-Alias Toggle */
    // if (windowStuff.keyPressed['A'] && !windowStuff.keyPressedPrev['A']) {
    //     windowStuff.keyPressedPrev['A'] = true;
    //     if (ANTI_ALIAS) ANTI_ALIAS = false;
    //     else ANTI_ALIAS = true;
    // }

    /* Hitbox Toggle */
    if (toggleKey('H')) {
        if (!windowStuff.windowBuffer.showHitboxes) windowStuff.windowBuffer.showHitboxes = true;
        else windowStuff.windowBuffer.showHitboxes = false;
    }

    /* Group WireFrames */
    if (selected.size() > 1 && toggleKey('G')) {
        EnterCriticalSection(&vectorLock);

        auto parent = selected[0];
        selected.erase(selected.begin(), selected.begin() + 1);

        for (auto child : selected) {
            parent->addChild(*child);
            child->expire();
        }
        selected.clear();
        LeaveCriticalSection(&vectorLock);
    }

    if (selected.empty()) return;

    for (auto& wireFrame : selected) {
        if (keyPressed(VK_DELETE)) {
            EnterCriticalSection(&vectorLock);
            wireFrame->expire();
            LeaveCriticalSection(&vectorLock);

            selected.clear();
            continue;
        }
        // Cube movement Left/Right/Up/Down
        if (keyPressed(VK_LEFT)) {
            toggleKey(VK_LEFT);
            if (moveVelocity.x > -maxSpeed) { moveVelocity.x -= maxSpeed; }
        }
        if (keyPressed(VK_RIGHT)) {
            toggleKey(VK_RIGHT);
            if (moveVelocity.x < maxSpeed) { moveVelocity.x += maxSpeed; }
        }
        if (keyPressed(VK_UP)) {
            toggleKey(VK_UP);
            if (moveVelocity.y < maxSpeed) { moveVelocity.y += maxSpeed; }
        }
        if (keyPressed(VK_DOWN)) {
            toggleKey(VK_DOWN);
            if (moveVelocity.y > -maxSpeed) { moveVelocity.y -= maxSpeed; }
        }
        if ((!keyPressedPrev(VK_LEFT) && !keyPressedPrev(VK_RIGHT)) || (keyPressed(VK_LEFT) && keyPressed(VK_RIGHT))) {
            moveVelocity.x = 0;
        }
        if ((!keyPressedPrev(VK_UP) && !keyPressedPrev(VK_DOWN)) || (keyPressed(VK_UP) && keyPressed(VK_DOWN))) {
            moveVelocity.y = 0;
        }
        // Start the object rotating
        if (keyPressed('X') && !keyPressedPrev('X')) {  // x
            wireFrame->toggleRotation(rotateX);
        }
        if (keyPressed('Y') && !keyPressedPrev('Y')) {  // y
            wireFrame->toggleRotation(rotateY);
        }
        if (keyPressed('Z') && !keyPressedPrev('Z')) {  // z
            wireFrame->toggleRotation(rotateZ);
        }
    }
    // Toggle the keys
    if (keyPressed('X')) toggleKey('X');
    if (keyPressed('Y')) toggleKey('Y');
    if (keyPressed('Z')) toggleKey('Z');
}

void renderThreadProc() {
    // Capture initial frequency
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    // Capture initial time
    LARGE_INTEGER lastTime, currentTime;
    QueryPerformanceCounter(&lastTime);

    while (running) {

        QueryPerformanceCounter(&currentTime);
        float deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / freq.QuadPart;
        lastTime = currentTime;

        if constexpr (SHOW_FPS) std::cout << "FPS: " << 1 / deltaTime << '\n';

        EnterCriticalSection(&bufferLock);
        windowStuff.windowBuffer.clearToBlack();
        LeaveCriticalSection(&bufferLock);

        // Iterate over all WireFrames, draw to screen, rotate, and record updates
        for (WireFrame& wireFrame : windowStuff.wireFrames) {
            if (wireFrame.isExpired()) continue;

            wireFrame.rotate(deltaTime, TPS);
            if (isSelected(wireFrame)) {
                wireFrame.move(moveVelocity, deltaTime, TPS);
                // Change to blue
                windowStuff.windowBuffer.setColor(0, 0, 255);
            }
            // Draw
            EnterCriticalSection(&bufferLock);
            windowStuff.windowBuffer.drawWireframe(wireFrame);
            // Reset to green
            windowStuff.windowBuffer.setColor(0, 255, 0);
            LeaveCriticalSection(&bufferLock);
        }

        /* Remove expired wireFrames */
        EnterCriticalSection(&vectorLock);
        std::erase_if(windowStuff.wireFrames, [](const WireFrame& wireFrame) {
            return wireFrame.isExpired();
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
            return TRUE;
        }
        // Redraw window handling
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            EndPaint(hwnd, &ps);
            break;
        }
        // Menu bar handling
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case ID_FILE_NEW_CUBE: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(cubeFile);
                    windowStuff.wireFrames.back().updateLocation({0, 0, 50});
                    LeaveCriticalSection(&vectorLock);

                    break;
                }
                case ID_FILE_NEW_SPHERE: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(sphereFile);
                    windowStuff.wireFrames.back().updateLocation({0, 0, 50});
                    LeaveCriticalSection(&vectorLock);

                    break;
                }
                case ID_FILE_NEW_CYLINDER: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(cylFile);
                    windowStuff.wireFrames.back().updateLocation({0, 0, 50});
                    LeaveCriticalSection(&vectorLock);

                    break;
                }
                case ID_FILE_NEW_PYRAMID: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.emplace_back(pyramidFile);
                    windowStuff.wireFrames.back().updateLocation({0, 0, 50});
                    LeaveCriticalSection(&vectorLock);

                    break;
                }
                case ID_FILE_NEW_TEST: {
                    EnterCriticalSection(&vectorLock);
                    windowStuff.wireFrames.reserve(windowStuff.wireFrames.size() + 2);  // Reserve space

                    windowStuff.wireFrames.emplace_back(cubeFile);
                    auto& parent = windowStuff.wireFrames.back();

                    parent.updateLocation({-20, 0, 0});
                    windowStuff.wireFrames.emplace_back(cubeFile);
                    parent.addChild(windowStuff.wireFrames.back());

                    parent.updateLocation({0, 0, 100});
                    windowStuff.wireFrames.back().expire();

                    /* Stress test max number of objects */
                    // int floorWidth = 15;
                    // int floorDepth = 15;
                    // float pieceWidth = 20.0f;
                    // auto xPos = static_cast<float>(floorWidth * 0.5) - 0.5f;
                    // auto yPos = static_cast<float>(floorDepth * 0.5) - 0.5f;
                    //
                    // windowStuff.wireFrames.emplace_back(floorFile);
                    // auto& parent = windowStuff.wireFrames.back();
                    // parent.reserve(floorWidth * floorDepth);
                    //
                    // #pragma omp parallel for
                    // for (int x = static_cast<int>(-xPos); x <= static_cast<int>(xPos); x++) {
                    //     for (int y = static_cast<int>(-yPos); y <= static_cast<int>(yPos); y++) {
                    //         if (x == 0 && y == 0) continue;
                    //
                    //         WireFrame child{floorFile};
                    //         child.updateLocation({x * pieceWidth, 0, y * pieceWidth});
                    //         parent.addChild(child);
                    //     }
                    // }
                    //
                    // parent.updateLocation({0, -15.0f * floorDepth, 2 * pieceWidth * floorDepth});
                    // parent.rotate({0.989847004, 0, 0, 0.14213714}, parent.getLocation());

                    LeaveCriticalSection(&vectorLock);

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
                moveVelocity.z = -12.0f;
            } else if (delta < 0) {
                moveVelocity.z = 12.0f;
            }
            lastMove = std::chrono::steady_clock::now();
            break;
        }
        // Left click handling
        case WM_LBUTTONDOWN: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            bool noHits = true;
            for (auto& wireFrame : windowStuff.wireFrames) {
                if (wireFrame.intersects({xPos, yPos}, windowStuff.windowBuffer.raycast)) {
                    if (selected.empty() || windowStuff.keyPressed[VK_SHIFT]) {
                        selected.emplace_back(&wireFrame);
                    } else {
                        selected[0] = &wireFrame;
                    }
                    noHits = false;
                    break;
                }
            }
            if (noHits) {
                selected.clear();
            }

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
    windowStuff.windowBuffer.setColor(0, 255, 0);

    if (hwnd == nullptr) {
        MessageBox(nullptr, "Window Creation Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Playback::replay(hwnd, windowStuff.windowBuffer, lpCmdLine);

    std::thread renderThread(renderThreadProc);

    while (running) {
        if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        keyHandling();
    }

    if (renderThread.joinable()) {
        renderThread.join();
    }

    DeleteCriticalSection(&bufferLock);
    DeleteCriticalSection(&vectorLock);
    return Msg.wParam;
}