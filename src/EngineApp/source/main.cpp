#include "pch.h"

// inclu//de the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <wrl.h>
#include <math.h>

#include <Keyboard.h>
#include <Mouse.h>
#include <PrimitiveBatch.h>

#include "Camera.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Model.h"

#undef min
#undef max

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

const unsigned WIDTH = 800;
const unsigned HEIGHT = 600;

// global declarations
IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11Texture2D* m_depthStencilBuffer;
ID3D11DepthStencilView *pDepthStencilView;
ID3D11DepthStencilState* m_depthStencilState;
ID3D11Buffer* pConstantBuffer;
ID3D11Buffer* pConsantBuffer2;

std::unique_ptr<DirectX::Keyboard> g_keyboard;
std::unique_ptr<DirectX::Mouse> g_mouse;

std::unique_ptr<Camera> g_camera, g_lightCamera;
std::unique_ptr<Model> g_model;
std::unique_ptr<Shader> g_shader, g_debugPrimitiveShader;
std::unique_ptr<RenderTarget> g_shadowMap;

struct Vertex3D
{
	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Vector4 color;
};

std::unique_ptr<DirectX::PrimitiveBatch<Vertex3D>> g_debugDrawer;

struct ConstantBufferType
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 worldViewProjection;
	DirectX::XMFLOAT4 lightDir;
};

struct FogDataBuffer
{
	DirectX::XMFLOAT4 params;		// start and end in xy, zw not used
	DirectX::XMFLOAT4 color;		// color and influence in alpha
};

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void UpdateFrame(void);     // renders a single frame
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(void);    // creates the shape to render

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	//{
	//	AllocConsole();
	//
	//	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	//	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	//	FILE* hf_out = _fdopen(hCrt, "w");
	//	setvbuf(hf_out, NULL, _IONBF, 1);
	//	*stdout = *hf_out;
	//
	//	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	//	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	//	FILE* hf_in = _fdopen(hCrt, "r");
	//	setvbuf(hf_in, NULL, _IONBF, 128);
	//	*stdin = *hf_in;
	//}

	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Our First Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	// set up and initialize Direct3D
	InitD3D(hWnd);


	g_keyboard = std::make_unique<DirectX::Keyboard>();
	g_mouse = std::make_unique<DirectX::Mouse>();
	g_mouse->SetWindow(hWnd);

	// enter the main loop:

	MSG msg;

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			UpdateFrame();
			RenderFrame();
		}
	}

	// clean up DirectX and COM
	CleanD3D();

	CoUninitialize();

	return static_cast<int>(msg.wParam);
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;

	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{
	HRESULT result;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.SampleDesc.Quality = 0;                             // multisample quality level
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);


	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	///// Depth

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = WIDTH;
	depthBufferDesc.Height = HEIGHT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = dev->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	assert(SUCCEEDED(result));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	// Create the depth stencil state.
	result = dev->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	assert(SUCCEEDED(result));

	// Set the depth stencil state.
	devcon->OMSetDepthStencilState(m_depthStencilState, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = dev->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &pDepthStencilView);
	assert(SUCCEEDED(result));

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	devcon->RSSetViewports(1, &viewport);

	InitGraphics();

	const float fovy = DirectX::XMConvertToRadians(45.0f);
	const float ar = (float)WIDTH / (float)HEIGHT;

	//g_camera = std::make_unique<Camera>(Camera::Orthographic{ 20.0f }, ar, 0.5f, 500.0f);
	g_camera = std::make_unique<Camera>(Camera::Perspective{ fovy }, ar, 0.5f, 500.0f);
	g_camera->SetPosition(DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f));
	g_camera->SetRotation(DirectX::XMQuaternionIdentity());

	g_lightCamera = std::make_unique<Camera>(Camera::Orthographic{ 200.0f }, 1.0f, 0.5f, 500.0f);
	g_lightCamera->SetPosition(DirectX::XMVectorSet(84.0f, 69.0f, 105.0f, 1.0f));
	g_lightCamera->SetRotation({ 0.115494534f, -0.882180750f, 0.286937863f, 0.355084032f });
	//g_lightCamera->SetRotation(DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixLookAtLH(g_lightCamera->GetPosition(), DirectX::XMVectorZero(), DirectX::XMVectorSet(0, 1, 0, 0))));

	g_shader = std::make_unique<Shader>(dev, L"shaders/basic.hlsl");
	g_debugPrimitiveShader = std::make_unique<Shader>(dev, L"shaders/primitive.hlsl");

	g_model = std::make_unique<Model>(dev, "models/Snow covered CottageOBJ.obj");

	g_shadowMap = std::make_unique<RenderTarget>(dev, 1024, 1024, std::string(), "D24S8");

	g_debugDrawer = std::make_unique<DirectX::PrimitiveBatch<Vertex3D>>(devcon);
}


// this is the function that creates the shape to render
void InitGraphics()
{
	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(ConstantBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		auto result = dev->CreateBuffer(&matrixBufferDesc, NULL, &pConstantBuffer);
		assert(SUCCEEDED(result));
	}

	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(FogDataBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		FogDataBuffer fogData;
		fogData.color = { 1.0f, 1.0f, 1.0f, 0.2f };
		fogData.params = { 10.0f, 200.0f, 0.0f, 0.0f };

		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = &fogData;
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		auto result = dev->CreateBuffer(&bufferDesc, &subresourceData, &pConsantBuffer2);
		assert(SUCCEEDED(result));
	}
}


void UpdateFrame()
{
	auto kb = g_keyboard->GetState();
	if (kb.Escape)
	{
		PostQuitMessage(0);
	}

	const float ROTATION_GAIN = 0.01f;
	const float FOV_GAIN = DirectX::XMConvertToRadians(1.0f);
	const float ORTHO_VIEW_GAIN = 1.0f;
	static float MOVEMENT_GAIN = 1.0f;

	// update camera rotation
	{		
		
		auto mouse = g_mouse->GetState();

		if (auto scrollVal = mouse.scrollWheelValue)
		{
			auto clamp = [](auto val, auto lowerBound, auto upperBound)
			{
				return std::min(std::max(val, lowerBound), upperBound);
			};

			MOVEMENT_GAIN = clamp(MOVEMENT_GAIN + static_cast<float>(scrollVal) * 0.001f, 0.01f, 10.0f);
			g_mouse->ResetScrollWheelValue();
		}
		
		if (mouse.positionMode == DirectX::Mouse::MODE_RELATIVE)
		{
			auto&& cameraFrontVec = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), g_camera->GetRotation());
		
			const DirectX::XMFLOAT3 cameraFront =
			{
				DirectX::XMVectorGetX(cameraFrontVec),
				DirectX::XMVectorGetY(cameraFrontVec),
				DirectX::XMVectorGetZ(cameraFrontVec)
			};
		
			const float fLen = sqrtf(cameraFront.z * cameraFront.z + cameraFront.x * cameraFront.x);
		
			float cameraYawAngle = atan2f(cameraFront.x, cameraFront.z);
			float cameraPitchAngle = -atan2f(cameraFront.y, fLen);
		
			DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(float(mouse.x), float(mouse.y), 0.f) * ROTATION_GAIN;
		
			cameraPitchAngle += delta.y;
			cameraYawAngle += delta.x;
		
			// limit pitch to straight up or straight down
			// with a little fudge-factor to avoid gimbal lock
			float limit = DirectX::XM_PI / 2.0f - 0.01f;
			cameraPitchAngle = std::max(-limit, cameraPitchAngle);
			cameraPitchAngle = std::min(+limit, cameraPitchAngle);
		
			// keep longitude in sane range by wrapping
			if (cameraYawAngle > DirectX::XM_PI)
			{
				cameraYawAngle -= DirectX::XM_PI * 2.0f;
			}
			else if (cameraYawAngle < -DirectX::XM_PI)
			{
				cameraYawAngle += DirectX::XM_PI * 2.0f;
			}
			g_camera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(cameraPitchAngle, cameraYawAngle, 0.0f));
		}
		
		g_mouse->SetMode(mouse.leftButton ? DirectX::Mouse::MODE_RELATIVE : DirectX::Mouse::MODE_ABSOLUTE);
	}

	// update camera position
	{
		float deltaMoveRight = 0.0f;
		float deltaMoveFront = 0.0f;
		float deltaMoveUp = 0.0f;
		float deltaFrustum = 0.0f;

		if (kb.Up || kb.W)
		{
			deltaMoveFront += MOVEMENT_GAIN;
		}

		if (kb.Down || kb.S)
		{
			deltaMoveFront -= MOVEMENT_GAIN;
		}

		if (kb.Right || kb.D)
		{
			deltaMoveRight += MOVEMENT_GAIN;
		}

		if (kb.Left || kb.A)
		{
			deltaMoveRight -= MOVEMENT_GAIN;
		}

		if (kb.X)
		{
			deltaMoveUp += MOVEMENT_GAIN;
		}

		if (kb.Z)
		{
			deltaMoveUp -= MOVEMENT_GAIN;
		}

		if (kb.E)
		{
			deltaFrustum += g_camera->IsPerspective() ? FOV_GAIN : ORTHO_VIEW_GAIN;
		}

		if (kb.Q)
		{
			deltaFrustum -= g_camera->IsPerspective() ? FOV_GAIN : ORTHO_VIEW_GAIN;
		}

		auto&& cameraPos = g_camera->GetPosition();
		auto&& cameraFront = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), g_camera->GetRotation());
		auto&& cameraRight = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), g_camera->GetRotation());
		auto&& cameraUp = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), g_camera->GetRotation());

		auto&& offset = DirectX::XMVectorAdd(DirectX::XMVectorScale(cameraFront, deltaMoveFront), DirectX::XMVectorScale(cameraRight, deltaMoveRight)); 
		offset = DirectX::XMVectorAdd(offset, DirectX::XMVectorScale(cameraUp, deltaMoveUp));

		g_camera->SetPosition(DirectX::XMVectorAdd(cameraPos, offset));
		g_camera->SetProjectionData(g_camera->GetProjectionData() + deltaFrustum);
	}

	g_lightCamera->UpdateFrustum();
	g_camera->UpdateFrustum();

	// 

	const float k_frameTime = 60.0f / 1000.0f; //sec

	//auto newScale = g_model->GetScale();
	//
	//static bool inc = true;
	//if (inc)
	//{
	//	newScale += DirectX::SimpleMath::Vector3{ 0.1f } * k_frameTime;
	//	if (newScale.x >= 2.0f)
	//	{
	//		inc = false;
	//	}
	//}
	//else
	//{
	//	newScale -= DirectX::SimpleMath::Vector3{ 0.1f } *k_frameTime;
	//	if (newScale.x <= 1.0f)
	//	{
	//		inc = true;
	//	}
	//}
	//
	//g_model->SetScale(newScale);
}

// this is the function used to render a single frame
void RenderFrame(void)
{
	// render to shadow map
	{
		g_shadowMap->Bind(devcon);

		// Lock the constant buffer so it can be written to.
		{
			D3D11_MAPPED_SUBRESOURCE ms;
			auto result = devcon->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
			assert(SUCCEEDED(result));

			// Get a pointer to the data in the constant buffer.
			auto dataPtr = (ConstantBufferType*)ms.pData;

			auto world = g_model->GetTransform();
			auto viewProj = g_lightCamera->GetViewProjectionTransform();
			auto worldViewProj = world * viewProj;

			// Copy the matrices into the constant buffer.
			DirectX::XMStoreFloat4x4(&dataPtr->world, world);
			DirectX::XMStoreFloat4x4(&dataPtr->view, g_lightCamera->GetViewTransform());
			DirectX::XMStoreFloat4x4(&dataPtr->worldViewProjection, worldViewProj);
			DirectX::XMStoreFloat4(&dataPtr->lightDir, DirectX::XMVector3Rotate(-DirectX::SimpleMath::Vector3::UnitZ, g_lightCamera->GetRotation()));

			// Unlock the constant buffer.
			devcon->Unmap(pConstantBuffer, 0);

			// Finanly set the constant buffer in the vertex shader with the updated values.
			devcon->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		}

		g_shader->Bind(devcon);
		g_model->Render(devcon, g_lightCamera->GetFrustum());
	}

	// clear the back buffer to a deep blue
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, pDepthStencilView);

	devcon->ClearRenderTargetView(backbuffer, clearColor);
	devcon->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Lock the constant buffer so it can be written to.
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		auto result = devcon->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		assert(SUCCEEDED(result));

		// Get a pointer to the data in the constant buffer.
		auto dataPtr = (ConstantBufferType*)ms.pData;

		auto world = g_model->GetTransform();
		auto viewProj = g_camera->GetViewProjectionTransform();
		auto worldViewProj = world * viewProj;

		// Copy the matrices into the constant buffer.
		DirectX::XMStoreFloat4x4(&dataPtr->world, world);
		DirectX::XMStoreFloat4x4(&dataPtr->view, g_camera->GetViewTransform());
		DirectX::XMStoreFloat4x4(&dataPtr->worldViewProjection, worldViewProj);
		DirectX::XMStoreFloat4(&dataPtr->lightDir, DirectX::XMVector3Rotate(-DirectX::SimpleMath::Vector3::UnitZ, g_lightCamera->GetRotation()));

		// Unlock the constant buffer.
		devcon->Unmap(pConstantBuffer, 0);

		// Finanly set the constant buffer in the vertex shader with the updated values.
		devcon->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	}

	{
		devcon->PSSetConstantBuffers(1, 1, &pConsantBuffer2);
		devcon->VSSetConstantBuffers(1, 1, &pConsantBuffer2);
	}

	g_shader->Bind(devcon);
	g_model->Render(devcon, g_camera->GetFrustum());

	///////////////////////////////////////  debug layer

	g_debugPrimitiveShader->Bind(devcon);
	g_debugDrawer->Begin();

	{
		const auto meshCount = g_model->GetMeshCount();
		for (auto i = 0; i < meshCount; i++)
		{
			auto&& mesh = g_model->GetMesh(i);
			auto&& aabb = mesh.aabb;

			std::array<Vector3, 8> aabbPoints;
			aabb.GetPoints(aabbPoints.data());

			Vertex3D vtx0{ aabbPoints[0],{ 1, 0, 0, 1 } };
			Vertex3D vtx1{ aabbPoints[1],{ 1, 0, 0, 1 } };
			Vertex3D vtx2{ aabbPoints[2],{ 1, 0, 0, 1 } };
			Vertex3D vtx3{ aabbPoints[3],{ 1, 0, 0, 1 } };
			Vertex3D vtx4{ aabbPoints[4],{ 1, 0, 0, 1 } };
			Vertex3D vtx5{ aabbPoints[5],{ 1, 0, 0, 1 } };
			Vertex3D vtx6{ aabbPoints[6],{ 1, 0, 0, 1 } };
			Vertex3D vtx7{ aabbPoints[7],{ 1, 0, 0, 1 } };

			g_debugDrawer->DrawLine(vtx0, vtx1);
			g_debugDrawer->DrawLine(vtx1, vtx2);
			g_debugDrawer->DrawLine(vtx2, vtx3);
			g_debugDrawer->DrawLine(vtx3, vtx0);

			g_debugDrawer->DrawLine(vtx4, vtx5);
			g_debugDrawer->DrawLine(vtx5, vtx6);
			g_debugDrawer->DrawLine(vtx6, vtx7);
			g_debugDrawer->DrawLine(vtx7, vtx4);

			g_debugDrawer->DrawLine(vtx0, vtx4);
			g_debugDrawer->DrawLine(vtx1, vtx5);
			g_debugDrawer->DrawLine(vtx2, vtx6);
			g_debugDrawer->DrawLine(vtx3, vtx7);
		}
	}

	g_debugDrawer->End();

	// switch the back buffer and the front buffer
	swapchain->Present(1, 0);
}


// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
	Texture::ClearUnreferenced();

	g_debugDrawer.reset();
	g_debugPrimitiveShader.reset();

	g_lightCamera = nullptr;
	g_camera = nullptr;
	g_model = nullptr;
	g_shader = nullptr;

	g_shadowMap = nullptr;

	g_keyboard = nullptr;
	g_mouse = nullptr;

	// close and release all existing COM objects
	swapchain->Release();
	backbuffer->Release();
	m_depthStencilBuffer->Release();
	m_depthStencilState->Release();
	pDepthStencilView->Release();
	dev->Release();
	devcon->Release();
}