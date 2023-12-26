#pragma once

typedef struct
{
    struct ID3D11Device           *device;
    struct ID3D11DeviceContext    *context;
    struct IDXGISwapChain         *swapChain;
    struct ID3D11InputLayout      *inputLayout;
    struct ID3D11VertexShader     *vertexShader;
    struct ID3D11PixelShader      *pixelShader;
    struct ID3D11RenderTargetView *renderTargetView;
    struct ID3D11Buffer           *vertexBuffer;
} RendererState;

typedef struct
{
    struct { float x, y; } pos;
    u32 col;
} Vertex;

typedef struct
{
    u32 vertexCount;
    Vertex *vertices;
} RendererDrawData;

RendererState RendererInit(HWND window);
void RendererDraw(RendererState *renderer, const RendererDrawData *drawData);
void RendererCleanup(RendererState *renderer);