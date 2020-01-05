#include "pch.h"

// inclu//de the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
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
#include "Material.h"

#undef min
#undef max

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Matrix;

const unsigned msaaSamples = 4;
const unsigned WIDTH = 800;
const unsigned HEIGHT = 600;

namespace debug
{
	bool g_drawSubmeshesBoundingBox = false;
	bool g_drawShadowmapCameraFrustum = false;
	bool g_drawGrid = true;
	bool g_animateModel = false;
}

// global declarations
IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11DepthStencilView *pDepthStencilView;

Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;

std::unique_ptr<DirectX::Keyboard> g_keyboard;
std::unique_ptr<DirectX::Mouse> g_mouse;

std::unique_ptr<Camera> g_camera, g_lightCamera;
std::unique_ptr<Model> g_model;
std::unique_ptr<Shader> g_shader, g_debugPrimitiveShader, g_shadowMapShader;
std::unique_ptr<RenderTarget> g_shadowMap;

Material basicMaterial, shadowMapMaterial, primitiveMaterial;

struct Vertex3D
{
	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Vector4 color;
};

std::unique_ptr<DirectX::PrimitiveBatch<Vertex3D>> g_debugDrawer;

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void UpdateFrame(void);     // renders a single frame
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory

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
	//	int hCrt = _open_osfhandle(reinterpret_cast<intptr_t>(handle_out), _O_TEXT);
	//	FILE* hf_out = _fdopen(hCrt, "w");
	//	setvbuf(hf_out, NULL, _IONBF, 1);
	//	*stdout = *hf_out;
	//
	//	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	//	hCrt = _open_osfhandle(reinterpret_cast<intptr_t>(handle_in), _O_TEXT);
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
		50,
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
	scd.SampleDesc.Count = msaaSamples;                     // how many multisamples
	scd.SampleDesc.Quality = 0;                             // multisample quality level
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	result = D3D11CreateDeviceAndSwapChain(NULL,
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
	assert(SUCCEEDED(result));

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	result = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	assert(SUCCEEDED(result));

	// use the back buffer address to create the render target
	result = dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	assert(SUCCEEDED(result));
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
	depthBufferDesc.SampleDesc.Count = msaaSamples;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	result = dev->CreateTexture2D(&depthBufferDesc, NULL, depthStencilBuffer.GetAddressOf());
	assert(SUCCEEDED(result));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = msaaSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = dev->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, &pDepthStencilView);
	assert(SUCCEEDED(result));

	depthStencilBuffer = nullptr;

	//// set scissor rect
	//{
	//	D3D11_RECT scissorRect;
	//	scissorRect.left = 0;
	//	scissorRect.top = 0;
	//	scissorRect.right = WIDTH;
	//	scissorRect.bottom = HEIGHT;
	//	devcon->RSSetScissorRects(1, &scissorRect);
	//}

	{
		CD3D11_RASTERIZER_DESC rasterizerState(D3D11_DEFAULT);
		//rasterizerState.CullMode = D3D11_CULL_BACK;
		//rasterizerState.FillMode = D3D11_FILL_SOLID;
		//rasterizerState.CullMode = D3D11_CULL_FRONT;
		//rasterizerState.FrontCounterClockwise = true;
		//rasterizerState.DepthBias = false;
		//rasterizerState.DepthBiasClamp = 0;
		//rasterizerState.SlopeScaledDepthBias = 0;
		//rasterizerState.DepthClipEnable = true;
		//rasterizerState.ScissorEnable = true;
		//rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = msaaSamples > 1;
		//rasterizerState.ForcedSampleCount = 0;
		result = dev->CreateRasterizerState(&rasterizerState, m_rasterState.GetAddressOf());
		assert(SUCCEEDED(result));
	}

	{
		const float fovy = DirectX::XMConvertToRadians(45.0f);
		const float ar = (float)WIDTH / (float)HEIGHT;

		//g_camera = std::make_unique<Camera>(Camera::Orthographic{ 20.0f }, ar, 0.5f, 500.0f);
		g_camera = std::make_unique<Camera>(Camera::Perspective{ fovy }, ar, 0.5f, 5000.0f);
		g_camera->SetPosition({ 0.0f, 0.0f, -150.0f });
		g_camera->SetRotation(DirectX::SimpleMath::Quaternion::Identity);
	}

	{
		g_lightCamera = std::make_unique<Camera>(Camera::Orthographic{ 200.0f }, 1.0f, 0.5f, 500.0f);
		g_lightCamera->SetPosition({ 150.f, 70.0f, 80 });

		Matrix rotMatrix = Matrix::CreateLookAt(-g_lightCamera->GetPosition(), Vector3::Zero, Vector3::Up);
		rotMatrix = rotMatrix.Transpose();
		const auto quat = Quaternion::CreateFromRotationMatrix(rotMatrix);
		g_lightCamera->SetRotation(quat);
	}

	g_shader = std::make_unique<Shader>(dev, L"shaders/basic.hlsl");
	g_shadowMapShader = std::make_unique<Shader>(dev, L"shaders/shadowmap.hlsl");
	g_debugPrimitiveShader = std::make_unique<Shader>(dev, L"shaders/primitive.hlsl");

	//g_model = std::make_unique<Model>(dev, "models/247_House 15_obj.obj");
	//g_model = std::make_unique<Model>(dev, "models/Futuristic Apartment.obj");
	//g_model = std::make_unique<Model>(dev, "models/luxury house interior.obj");
	g_model = std::make_unique<Model>(dev, "models/Snow covered CottageOBJ.obj");
	//g_model = std::make_unique<Model>(dev, "models/orb.obj");
	//g_model = std::make_unique<Model>(dev, "models/cube.obj");
	
	g_shadowMap = std::make_unique<RenderTarget>(dev, 1024, 1024, std::string(), "D24S8");

	g_debugDrawer = std::make_unique<DirectX::PrimitiveBatch<Vertex3D>>(devcon);

	//
	basicMaterial = Material(g_shader.get());
	shadowMapMaterial = Material(g_shadowMapShader.get());
	primitiveMaterial = Material(g_debugPrimitiveShader.get());

	auto diffuse = Texture::CreateTexture(dev, L"textures/tmp/AussenWand_C.jpg");

	basicMaterial.SetUniform("fogParams", { 10.0f, 200.0f, 0.0f, 0.0f });
	basicMaterial.SetUniform("fogColor", { 1.0f, 1.0f, 1.0f, 0.2f });
	basicMaterial.SetUniform("lightDir", Vector3::Transform(Vector3::Forward, g_lightCamera->GetRotation()));
	basicMaterial.SetTexture("shaderTexture", diffuse.get());
	basicMaterial.SetTexture("shadowMapSampler", g_shadowMap->GetDepthTexture());

	primitiveMaterial.SetBlendState({ true, BlendFactor::SRC_ALPHA, BlendFactor::INV_SRC_ALPHA, BlendOp::ADD });
	primitiveMaterial.SetDepthTest(true);
	primitiveMaterial.SetDepthWrite(false);

	{
		auto&& mdlAabb = g_model->GetLocalAABB();
		auto&& aabbDiag = mdlAabb.GetMaxPoint() - mdlAabb.GetMinPoint();
		auto&& camPos = g_model->GetPosition() + aabbDiag;
		Matrix rotMatrix = Matrix::CreateLookAt(-camPos, g_model->GetPosition(), Vector3::Up);
		rotMatrix = rotMatrix.Transpose();
		const auto quat = Quaternion::CreateFromRotationMatrix(rotMatrix);
		g_camera->SetPosition(camPos);
		g_camera->SetRotation(quat);
	}

	{
		auto&& mdlAabb = g_model->GetLocalAABB();
		auto&& view = Matrix::CreateLookAt(-g_lightCamera->GetPosition(), Vector3::Zero, Vector3::Up);

		std::array<Vector3, 8> points;
		mdlAabb.GetPoints(points.data());

		AABB mdlAabbVS;
		for (size_t i = 0; i < points.size(); i++)
		{
			auto pointVS = Vector3::Transform(points[i], view);
			mdlAabbVS.AddPoint(pointVS);
		}

		auto extents = mdlAabbVS.GetExtents();

		view = view.Invert();

		std::array<Vector3, 8> pointsWS;
		mdlAabbVS.GetPoints(pointsWS.data());
		for (size_t i = 0; i < pointsWS.size(); i++)
		{
			pointsWS[i] = Vector3::Transform(pointsWS[i], view);
		}

		auto cameraPos = pointsWS[0] + view.Right() * extents.x * 0.5f + view.Up() * extents.y * 0.5f;

		g_lightCamera->SetProjectionData(extents.y);
		g_lightCamera->SetAspectRatio(extents.x / extents.y);
		g_lightCamera->SetNear(0.0f);
		g_lightCamera->SetFar(extents.z);
		g_lightCamera->SetPosition(cameraPos);

		basicMaterial.SetUniform("shadowMapMatrix", g_lightCamera->GetViewProjectionTransform().Transpose());

		//g_camera = std::make_unique<Camera>(Camera::Orthographic{ extents.y }, g_lightCamera->GetAspectRatio(), 0.0f, extents.z);
		//g_camera->SetPosition(g_lightCamera->GetPosition());
		//g_camera->SetRotation(g_lightCamera->GetRotation());
	}

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
	result = dev->QueryInterface(d3dDebug.GetAddressOf());

	if (SUCCEEDED(result))
	{
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		result = d3dDebug.As(&d3dInfoQueue);

		if (SUCCEEDED(result))
		{
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET
			};

			D3D11_INFO_QUEUE_FILTER filter;
			ZeroMemory(&filter, sizeof(filter));

			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;

			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif
}


void UpdateFrame()
{
	auto kb = g_keyboard->GetState();
	if (kb.Escape)
	{
		PostQuitMessage(0);
	}

	if (kb.LeftControl && kb.D1)
	{
		debug::g_drawGrid = !debug::g_drawGrid;
	}
	if (kb.LeftControl && kb.D2)
	{
		debug::g_drawSubmeshesBoundingBox = !debug::g_drawSubmeshesBoundingBox;
	}
	if (kb.LeftControl && kb.D3)
	{
		debug::g_drawShadowmapCameraFrustum = !debug::g_drawShadowmapCameraFrustum;
	}
	if (kb.LeftControl && kb.D4)
	{
		debug::g_animateModel = !debug::g_animateModel;
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
		auto&& cameraRight = Vector3::Transform(Vector3::UnitX, g_camera->GetRotation());
		auto&& cameraUp = Vector3::Transform(Vector3::UnitY, g_camera->GetRotation());
		auto&& cameraFront = Vector3::Transform(Vector3::UnitZ, g_camera->GetRotation());

		auto&& offset = cameraFront * deltaMoveFront + cameraRight * deltaMoveRight + cameraUp * deltaMoveUp;
		g_camera->SetPosition(cameraPos + offset);

		g_camera->SetProjectionData(g_camera->GetProjectionData() + deltaFrustum);
	}

	g_lightCamera->UpdateFrustum();
	g_camera->UpdateFrustum();

	// 

	const float k_frameTime = 60.0f / 1000.0f; //sec

	// apply some animations to model
	if (debug::g_animateModel)
	{
		g_model->SetRotation(g_model->GetRotation() * Quaternion::CreateFromAxisAngle(Vector3::Up, k_frameTime * 0.1f));

		auto newScale = g_model->GetScale();
		
		static bool inc = true;
		if (inc)
		{
			newScale += Vector3{ 0.1f } * k_frameTime;
			if (newScale.x >= 2.0f)
			{
				inc = false;
			}
		}
		else
		{
			newScale -= Vector3{ 0.1f } * k_frameTime;
			if (newScale.x <= 1.0f)
			{
				inc = true;
			}
		}
		
		g_model->SetScale(newScale);
	}

	g_model->UpdateBoundingVolumes();
}

// this is the function used to render a single frame
void RenderFrame(void)
{
	// set raster state
	devcon->RSSetState(m_rasterState.Get());

	// render to shadow map
	{
		shadowMapMaterial.SetUniform("worldViewProj", (g_model->GetTransform() * g_lightCamera->GetViewProjectionTransform()).Transpose());

		g_shadowMap->Bind(devcon);
		g_model->Render(dev, devcon, g_lightCamera->GetFrustum(), shadowMapMaterial);
	}

	// clear the back buffer to a deep blue
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, pDepthStencilView);

	// Set the viewport
	{
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = WIDTH;
		viewport.Height = HEIGHT;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		devcon->RSSetViewports(1, &viewport);
	}

	devcon->ClearRenderTargetView(backbuffer, clearColor);
	devcon->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	{

		const Matrix world = g_model->GetTransform();
		const Matrix viewProj = g_camera->GetViewProjectionTransform();
		const Matrix worldViewProj = world * viewProj;

		basicMaterial.SetUniform("world", world.Transpose());
		basicMaterial.SetUniform("view", g_camera->GetViewTransform().Transpose());
		basicMaterial.SetUniform("worldViewProj", worldViewProj.Transpose());

		g_model->Render(dev, devcon, g_camera->GetFrustum(), basicMaterial);

	}
	/////////////////////////////////////////  debug layer

	{
		primitiveMaterial.SetUniform("worldViewProj", (g_camera->GetViewProjectionTransform()).Transpose());
	}

	VertexBufferDesc vertexBufferDesc;
	vertexBufferDesc.AddInput(InputSemantic::POSITION, InputType::R32G32B32_FLOAT);
	vertexBufferDesc.AddInput(InputSemantic::COLOR, InputType::R32G32B32A32_FLOAT);

	devcon->IASetInputLayout(g_debugPrimitiveShader->RequestInputLayout(dev, vertexBufferDesc).Get());
	primitiveMaterial.Bind(dev, devcon);
	g_debugDrawer->Begin();

	// render grid
	if (debug::g_drawGrid)
	{
		const float k_gridSize = 1000.0f;
		const float k_cellSize = 50;

		// render X axis
		{
			Vertex3D vtx0{ { -k_gridSize, 0.0f, 0.0f }, { 0.5, 0, 0, 0.5 } };
			Vertex3D vtx1{ {  k_gridSize, 0.0f, 0.0f }, { 1, 0, 0, 0.5 } };
			g_debugDrawer->DrawLine(vtx0, vtx1);
		}

		// render Z axis
		{
			Vertex3D vtx0{ { 0.0f, 0.0f, -k_gridSize }, { 0, 0, 0.5, 0.5 } };
			Vertex3D vtx1{ { 0.0f, 0.0f,  k_gridSize }, { 0, 0, 1, 0.5 } };
			g_debugDrawer->DrawLine(vtx0, vtx1);
		}

		const float cellCount = std::ceilf(k_gridSize / k_cellSize);
		for (float i = 1; i <= cellCount; i++)
		{
			Vertex3D vtx0{ { -k_gridSize, 0.0f, i * k_cellSize },{ 0.5, 0.5, 0.5, 0.5 } };
			Vertex3D vtx1{ {  k_gridSize, 0.0f, i * k_cellSize },{ 0.5, 0.5, 0.5, 0.5 } };
			g_debugDrawer->DrawLine(vtx0, vtx1);

			Vertex3D vtx2{ { -k_gridSize, 0.0f, -i * k_cellSize },{ 0.5, 0.5, 0.5, 0.5 } };
			Vertex3D vtx3{ {  k_gridSize, 0.0f, -i * k_cellSize },{ 0.5, 0.5, 0.5, 0.5 } };
			g_debugDrawer->DrawLine(vtx2, vtx3);
		}

		for (float i = 1; i <= cellCount; i++)
		{
			Vertex3D vtx0{ { i * k_cellSize, 0.0f, -k_gridSize },{ 0.5, 0.5, 0.5, 0.5 } };
			Vertex3D vtx1{ { i * k_cellSize, 0.0f,  k_gridSize },{ 0.5, 0.5, 0.5, 0.5 } };
			g_debugDrawer->DrawLine(vtx0, vtx1);

			Vertex3D vtx2{ { -i * k_cellSize, 0.0f, -k_gridSize },{ 0.5, 0.5, 0.5, 0.5 } };
			Vertex3D vtx3{ { -i * k_cellSize, 0.0f,  k_gridSize },{ 0.5, 0.5, 0.5, 0.5 } };
			g_debugDrawer->DrawLine(vtx2, vtx3);
		}
	}

	// render bounding boxes
	if (debug::g_drawSubmeshesBoundingBox)
	{
		const auto meshCount = g_model->GetMeshCount();
		for (auto i = 0; i < meshCount; i++)
		{
			auto&& mesh = g_model->GetMesh(i);
			auto&& aabb = mesh.worldAabb;

			std::array<Vector3, 8> aabbPoints;
			aabb.GetPoints(aabbPoints.data());

			const Vector4 clr = { 1, 1, 1, 1.0f };

			Vertex3D vtx0{ aabbPoints[0],clr };
			Vertex3D vtx1{ aabbPoints[1],clr };
			Vertex3D vtx2{ aabbPoints[2],clr };
			Vertex3D vtx3{ aabbPoints[3],clr };
			Vertex3D vtx4{ aabbPoints[4],clr };
			Vertex3D vtx5{ aabbPoints[5],clr };
			Vertex3D vtx6{ aabbPoints[6],clr };
			Vertex3D vtx7{ aabbPoints[7],clr };

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

	// render shadow map frustum
	if (debug::g_drawShadowmapCameraFrustum)
	{
		const Vector4 clr = { 1, 0, 1, 1.0f };
		auto frustumPoints = g_lightCamera->GetFrustum().GetFrumstumPoints();
	
		Vertex3D vtx0{ frustumPoints[0],clr };
		Vertex3D vtx1{ frustumPoints[1],clr };
		Vertex3D vtx2{ frustumPoints[2],clr };
		Vertex3D vtx3{ frustumPoints[3],clr };
		Vertex3D vtx4{ frustumPoints[4],clr };
		Vertex3D vtx5{ frustumPoints[5],clr };
		Vertex3D vtx6{ frustumPoints[6],clr };
		Vertex3D vtx7{ frustumPoints[7],clr };
	
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

		g_debugDrawer->DrawLine(vtx0, vtx2);
		g_debugDrawer->DrawLine(vtx1, vtx3);
	}

	g_debugDrawer->End();

	// switch the back buffer and the front buffer
	swapchain->Present(1, 0);
}


// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
	m_rasterState.Reset();

	Material::CleanCaches();
	Texture::ClearUnreferenced();

	g_debugDrawer.reset();
	g_debugPrimitiveShader.reset();

	g_lightCamera = nullptr;
	g_camera = nullptr;
	g_model = nullptr;
	g_shader = nullptr;

	g_shadowMapShader.reset();
	g_shadowMap = nullptr;

	g_keyboard = nullptr;
	g_mouse = nullptr;

	// close and release all existing COM objects
	swapchain->Release();
	backbuffer->Release();
	pDepthStencilView->Release();
	dev->Release();
	devcon->Release();
}