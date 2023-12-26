#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define LOGGER_LEVEL_INFO
#define LOGGER_LEVEL_ERROR
#define LOGGER_IMPL
#include "giterme_log.h"

#include "giterme_main.h"
#include "win_renderer.h"

#pragma comment(lib, "user32.lib")

static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
static HWND WindowCreate(
    const wchar_t *title,
    int x      = CW_USEDEFAULT,
    int y      = CW_USEDEFAULT,
    int width  = CW_USEDEFAULT,
    int height = CW_USEDEFAULT);
static void WindowCleanup(HWND window);

typedef struct
{
    // RENDERER
    RendererState    *renderer;
    RendererDrawData *drawData;

    // INPUT
    struct { float x, y; } mouse;
} Giterme;

static Giterme giterme;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE instancePrevious, LPWSTR args, int show)
{
    LoggerInit();

    // TODO(guilherme): To calculando centro da tela na mão, depois eu vejo isso...
    HWND window = WindowCreate(L"Giterme", 320, 180, 1280, 720);
    Assert(IsWindow(window));

    RendererState renderer = RendererInit(window);
    RendererDrawData drawData =
    {
        .vertexCount = 6,
        .vertices = (Vertex *)VirtualAlloc(
            nullptr,
            6 * sizeof(Vertex),
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE),
    };
    ZeroMemory(drawData.vertices, drawData.vertexCount * sizeof(Vertex));
    //                                    X      Y             AABBGGRR
    drawData.vertices[0] = { .pos = { -0.5f, -0.5f }, .col = 0xff0000ff };
    drawData.vertices[1] = { .pos = { -0.5f,  0.5f }, .col = 0xff00ff00 };
    drawData.vertices[2] = { .pos = {  0.5f, -0.5f }, .col = 0xffff0000 };
    drawData.vertices[3] = { .pos = {  0.5f, -0.5f }, .col = 0xffff0000 };
    drawData.vertices[4] = { .pos = { -0.5f,  0.5f }, .col = 0xff00ff00 };
    drawData.vertices[5] = { .pos = {  0.5f,  0.5f }, .col = 0xff0000ff };

    giterme.renderer = &renderer;
    giterme.drawData = &drawData;

    bool quit = false;
    while (!quit)
    {
        MSG message;
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
            if (message.message == WM_QUIT) { quit = true; break; }
        }

        RendererDraw(giterme.renderer, giterme.drawData);
    }

    RendererCleanup(giterme.renderer);
    WindowCleanup(window);
}

static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = DefWindowProc(window, message, wParam, lParam);

    switch (message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            result = 0;
        } break;

        case WM_MOUSEMOVE:
        {
            giterme.mouse.x = (float)LOWORD(lParam);
            giterme.mouse.y = (float)HIWORD(lParam);
        } break;
    }

    return result;
}

static HWND WindowCreate(const wchar_t *title, int x, int y, int width, int height)
{
    HWND result = nullptr;

    WNDCLASSEXW windowClass =
    {
        .cbSize        = sizeof(windowClass),
        .lpfnWndProc   = &WindowProc,
        .hInstance     = GetModuleHandle(nullptr),
        .hIcon         = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor       = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = L"voidgitmainwindow",
    };
    RegisterClassEx(&windowClass);
    result = CreateWindowExW(
        WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP,
        windowClass.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x,
        y,
        width,
        height,
        nullptr,
        nullptr,
        windowClass.hInstance,
        nullptr);

    return result;
}

static void WindowCleanup(HWND window)
{
    if (window)
    {
        if (IsWindow(window)) { DestroyWindow(window); }
        window = 0;
    }
}