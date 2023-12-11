#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>


using namespace DirectX;

// Define a Colors class with predefined colors
//class Colors {
//public:
//    static const FLOAT CornflowerBlue[4];
//};
//
//// Initialize the CornflowerBlue color
//const FLOAT Colors::CornflowerBlue[4] = { 0.392f, 0.584f, 0.929f, 1.0f };


// Window variables
HWND hwnd = nullptr;
const int windowWidth = 800;
const int windowHeight = 600;

// Direct3D variables
IDXGISwapChain* swapChain = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

// Vertex data
struct Vertex {
    XMFLOAT3 Position;
};

// Vertex buffer
ID3D11Buffer* vertexBuffer = nullptr;

// Box position
float boxY = 0.0f;

// Function to initialize Direct3D
bool InitializeDirect3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 2, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &deviceContext);

    if (device == nullptr || deviceContext == nullptr || swapChain == nullptr) {
        return false;
    }

    // Create a render target view
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    // Set the viewport
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 1.0f };
    deviceContext->RSSetViewports(1, &viewport);

    return true;
}

// Function to create the vertex buffer
bool CreateVertexBuffer() {
    Vertex vertices[] = {
        { XMFLOAT3(-0.1f, -0.1f, 0.0f) },
        { XMFLOAT3(0.1f, -0.1f, 0.0f) },
        { XMFLOAT3(0.1f, 0.1f, 0.0f) },
        { XMFLOAT3(-0.1f, 0.1f, 0.0f) },
    };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    if (FAILED(device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer))) {
        return false;
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    return true;
}

// Function to render the frame
void RenderFrame() {
    // Clear the back buffer
    deviceContext->ClearRenderTargetView(renderTargetView, Colors::CornflowerBlue);

    // Update box position (simple falling animation)
    boxY += 0.001f;

    // Create world matrix and set it (In a complete program, you would set shaders, view, projection matrices, etc.)

    XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, boxY, 0.0f);

    // Set the world matrix
    // (In a complete program, you would also need to set shaders, view, projection matrices, etc.)

    deviceContext->UpdateSubresource(vertexBuffer, 0, nullptr, &worldMatrix, 0, 0);

    // Draw the box
    deviceContext->Draw(4, 0);

    // Present the frame
    swapChain->Present(1, 0);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"DirectX11WindowClass";
    RegisterClassEx(&wc);

    // Create the window
    hwnd = CreateWindow(L"DirectX11WindowClass", L"DirectX 11 Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    if (!InitializeDirect3D(hwnd) || !CreateVertexBuffer()) {
        MessageBox(hwnd, L"Initialization failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);

    // Main message loop
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            RenderFrame();
        }
    }

    // Release resources
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    if (swapChain) {
        swapChain->Release();
        swapChain = nullptr;
    }
    if (deviceContext) {
        deviceContext->Release();
        deviceContext = nullptr;
    }
    if (device) {
        device->Release();
        device = nullptr;
    }

    return static_cast<int>(msg.wParam);
}
