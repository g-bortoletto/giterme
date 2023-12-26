#include "giterme_log.h"
#include "giterme_main.h"
#include "win_renderer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

RendererState RendererInit(HWND window)
{
    RendererState result = {};
    HRESULT hr = 0;

    // Create device, context and swapchain
    {
        D3D_FEATURE_LEVEL d3dFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        DXGI_SWAP_CHAIN_DESC swapChainDesc =
        {
            .BufferDesc =
            {
                .Width = 0,
                .Height = 0,
                .RefreshRate =
                {
                    .Numerator = 60,
                    .Denominator = 1,
                },
                .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            },
            .SampleDesc = { .Count = 1 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = window,
            .Windowed = 1,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        #ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            d3dFeatureLevels,
            1,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &result.swapChain,
            &result.device,
            0,
            &result.context);
        if (FAILED(hr))
        {
            LogError("D3D11CreateDeviceAndSwapChain() failed.");
        }
        else
        {
            LogInfo("Created Device, Context and SwapChain.\n"
                "  + DEVICE:    0x%p\n"
                "  + CONTEXT:   0x%p\n"
                "  + SWAPCHAIN: 0x%p",
                result.device, result.context, result.swapChain);
        }
    }

    // Set up debug layer to break on D3D11 errors
    #ifdef _DEBUG
    ID3D11Debug *d3dDebug = nullptr;
    hr = result.device->QueryInterface(IID_PPV_ARGS(&d3dDebug));
    if (SUCCEEDED(hr))
    {
        LogInfo("Created d3dDebug layer.\n"
            "  + D3DDEBUGLAYER: 0x%p", d3dDebug);
    }
    else
    {
        LogError("Failed to create d3dDebug layer.");
    }
    if (d3dDebug)
    {
        ID3D11InfoQueue *d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue))))
        {
            LogInfo("Created debug info queue.\n"
                "  + DEBUGINFOQUEUE: 0x%p", d3dInfoQueue);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            d3dInfoQueue->Release();
            d3dInfoQueue = nullptr;
            LogInfo("Released debug info queue.\n"
                "  - DEBUGINFOQUEUE: 0x%p", d3dInfoQueue);
        }
        else
        {
            LogError("Failed to create debug info queue.");
        }
        d3dDebug->Release();
        d3dDebug = nullptr;
        LogInfo("Released the d3d debug layer.\n"
            "  - D3DDEBUGLAYER: 0x%p", d3dDebug);
    }
    #endif

    // Init render target view
    {
        ID3D11Texture2D *backBuffer = nullptr;
        if (FAILED(result.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
        {
            LogError("Could not get backBuffer to create RenderTargetView.");
        }
        else
        {
            LogInfo("Acquired backbuffer.\n"
                "  + BACKBUFFER: 0x%p",
                backBuffer);
        }
        hr = result.device->CreateRenderTargetView(backBuffer, 0, &result.renderTargetView);
        if (SUCCEEDED(hr))
        {
            LogInfo("Created render target view.\n"
                "  + RENDERTARGETVIEW: 0x%p",
                result.renderTargetView);
        }
        else
        {
            LogError("Could not create render target view.");
        }
    }

    // Vertex shader stuff
    {
        ID3DBlob *vertexShaderBlob = nullptr;
        const char vertexShaderSource[] =
            "struct VS_Input\
            {\
                float2 pos : POS;\
                float4 color : COL;\
            };\
            struct PS_Input\
            {\
                float4 position : SV_POSITION;\
                float4 color : COL;\
            };\
            PS_Input vs_main(VS_Input input)\
            {\
                PS_Input output;\
                output.position = float4(input.pos, 0.0f, 1.0f);\
                output.color = input.color;    \
                return output;\
            }";
        ID3DBlob *compileErrorsBlob = nullptr;
        if (D3DCompile(
                vertexShaderSource,
                strlen(vertexShaderSource),
                nullptr,
                nullptr,
                nullptr,
                "vs_main",
                "vs_5_0",
                0,
                0,
                &vertexShaderBlob,
                &compileErrorsBlob)
            != S_OK)
        {
            LogError("Could not compile vertex shader.\nError message: %ls", (wchar_t *)compileErrorsBlob->GetBufferPointer());
            Assert(false);
        }
        else
        {
            LogInfo("Compiled vertex shader.\n"
                "  + BLOB: 0x%p",
                vertexShaderBlob);
        }
        hr = result.device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            &result.vertexShader);
        if (SUCCEEDED(hr))
        {
            LogInfo("Created vertex shader.\n"
                "  + VERTEX_SHADER: 0x%p",
                result.vertexShader);
        }
        else
        {
            LogError("Could not create vertex shader.");
        }

        // Create input layout
        {
            D3D11_INPUT_ELEMENT_DESC localLayout[] =
            {
                { "POS", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COL", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(Vertex, col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            hr = result.device->CreateInputLayout(
                localLayout,
                2,
                vertexShaderBlob->GetBufferPointer(),
                vertexShaderBlob->GetBufferSize(),
                &result.inputLayout);
            if (SUCCEEDED(hr))
            {
                LogInfo("Created input layout.\n"
                    "  + INPUT_LAYOUT: 0x%p",
                    result.inputLayout);
            }
            else
            {
                LogError("Could not create input layout.");
            }
        }

        vertexShaderBlob->Release();
        vertexShaderBlob = nullptr;
        LogInfo("Released vertexShaderBlob.\n"
            "  - VERTEX_SHADER_BLOB: 0x%p",
            vertexShaderBlob);
        if (compileErrorsBlob)
        {
            compileErrorsBlob->Release();
            compileErrorsBlob = nullptr;
            LogInfo("Released compileErrorsBlob.\n"
                "  - VERTEX_SHADER_BLOB: 0x%p",
                compileErrorsBlob);
        }
    }

    // Pixel shader stuff
    {
        ID3DBlob *pixelShaderBlob = nullptr;
        ID3DBlob *compileErrorsBlob = nullptr;
        const char pixelShaderSource[] =
            "struct PS_Input\
            {\
                float4 position : SV_POSITION;\
                float4 color : COL;\
            };\
            float4 ps_main(PS_Input input) : SV_TARGET\
            {\
                return input.color;\
            }";
        if (D3DCompile(
                pixelShaderSource,
                strlen(pixelShaderSource),
                nullptr,
                nullptr,
                nullptr,
                "ps_main",
                "ps_5_0",
                0,
                0,
                &pixelShaderBlob,
                &compileErrorsBlob)
            != S_OK)
        {
            LogError("Could not compile pixel shader.\nError message: %ls", (wchar_t *)compileErrorsBlob->GetBufferPointer());
            Assert(false);
        }
        else
        {
            LogInfo("Compiled pixel shader.\n"
                "  + BLOB: 0x%p",
                pixelShaderBlob);
        }
        hr = result.device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            &result.pixelShader);
        if (SUCCEEDED(hr))
        {
            LogInfo("Created pixel shader.\n"
                "  + PIXEL_SHADER: 0x%p",
                result.pixelShader);
        }
        else
        {
            LogError("Could not create pixel shader.");
        }

        pixelShaderBlob->Release();
        pixelShaderBlob = nullptr;
        LogInfo("Released pixelShaderBlob.\n"
            "  - PIXEL_SHADER_BLOB: 0x%p",
            pixelShaderBlob);
        if (compileErrorsBlob)
        {
            compileErrorsBlob->Release();
            compileErrorsBlob = nullptr;
            LogInfo("Released compileErrorsBlob.\n"
                "  - VERTEX_SHADER_BLOB: 0x%p",
                compileErrorsBlob);
        }
    }

    LogInfo("INIT RESULT:\n"
        "  + DEVICE:             0x%p\n"
        "  + CONTEXT:            0x%p\n"
        "  + SWAPCHAIN:          0x%p\n"
        "  + INPUT_LAYOUT:       0x%p\n"
        "  + VERTEX_SHADER:      0x%p\n"
        "  + PIXEL_SHADER:       0x%p\n"
        "  + RENDER_TARGET_VIEW: 0x%p\n"
        "  + VERTEX_BUFFER:      0x%p",
        result.device,
        result.context,
        result.swapChain,
        result.inputLayout,
        result.vertexShader,
        result.pixelShader,
        result.renderTargetView,
        result.vertexBuffer);

    return result;
}

u32 stride = sizeof(Vertex);
u32 offset = 0;
void RendererDraw(RendererState *renderer, const RendererDrawData *drawData)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    renderer->swapChain->GetDesc(&swapChainDesc);
    D3D11_VIEWPORT viewport =
    {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width    = (float)swapChainDesc.BufferDesc.Width,
        .Height   = (float)swapChainDesc.BufferDesc.Height,
        .MinDepth = 0,
        .MaxDepth = 1
    };

    if (drawData)
    {
        if (!renderer->vertexBuffer)
        {
            D3D11_BUFFER_DESC vertexBufferDesc
            {
                .ByteWidth = (UINT)(drawData->vertexCount * sizeof(Vertex)),
                .Usage = D3D11_USAGE_DYNAMIC,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            };
            renderer->device->CreateBuffer(&vertexBufferDesc, nullptr, &renderer->vertexBuffer);
        }

        D3D11_MAPPED_SUBRESOURCE vertexData;
        renderer->context->Map(renderer->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertexData);
        Vertex *vertices = (Vertex *)vertexData.pData;
        memcpy(vertices, drawData->vertices, drawData->vertexCount * sizeof(Vertex));
        renderer->context->Unmap(renderer->vertexBuffer, 0);
    }

    renderer->context->IASetInputLayout(renderer->inputLayout);
    renderer->context->IASetVertexBuffers(0, 1, &renderer->vertexBuffer, &stride, &offset);
    renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    renderer->context->VSSetShader(renderer->vertexShader, 0, 0);
    renderer->context->PSSetShader(renderer->pixelShader, 0, 0);
    renderer->context->OMSetRenderTargets(1, &renderer->renderTargetView, 0);
    renderer->context->RSSetViewports(1, &viewport);

    renderer->context->Draw((UINT)drawData->vertexCount, 0);

    renderer->swapChain->Present(1, 0);
}

void RendererCleanup(RendererState *renderer)
{
    if (renderer)
    {
        if (renderer->device)               { renderer->device              ->Release(); renderer->device               = nullptr; }
        if (renderer->context)              { renderer->context             ->Release(); renderer->context              = nullptr; }
        if (renderer->swapChain)            { renderer->swapChain           ->Release(); renderer->swapChain            = nullptr; }
        if (renderer->inputLayout)          { renderer->inputLayout         ->Release(); renderer->inputLayout          = nullptr; }
        if (renderer->vertexShader)         { renderer->vertexShader        ->Release(); renderer->vertexShader         = nullptr; }
        if (renderer->pixelShader)          { renderer->pixelShader         ->Release(); renderer->pixelShader          = nullptr; }
        if (renderer->renderTargetView)     { renderer->renderTargetView    ->Release(); renderer->renderTargetView     = nullptr; }
        if (renderer->vertexBuffer)         { renderer->vertexBuffer        ->Release(); renderer->vertexBuffer         = nullptr; }
    }
}