// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "OBJ_Loader.h"

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

const unsigned WIDTH = 800;
const unsigned HEIGHT = 600;

class Camera;
class Model;
class Shader;

// global declarations
IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11Texture2D* m_depthStencilBuffer;
ID3D11DepthStencilView *pDepthStencilView;
ID3D11DepthStencilState* m_depthStencilState;
//ID3D11InputLayout *pLayout;            // the pointer to the input layout
//ID3D11VertexShader *pVS;               // the pointer to the vertex shader
//ID3D11PixelShader *pPS;                // the pointer to the pixel shader
//ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer
//ID3D11Buffer *pIBuffer;                // the pointer to the vertex buffer
ID3D11Buffer* pConstantBuffer;


//// a struct to define a single vertex
//struct VERTEX
//{
//	FLOAT X, Y, Z;
//	FLOAT Color[4];
//};

std::unique_ptr<Camera> g_camera;
std::unique_ptr<Model> g_model;
std::unique_ptr<Shader> g_shader;

template <typename ComObj>
class ComPtr
{
public:
	ComPtr() = default;
	ComPtr(const ComPtr&) = delete;

	~ComPtr()
	{
		if (m_comObj)
		{
			m_comObj->Release();
		}
	}

	ComPtr& operator = (const ComPtr&) = delete;

	operator bool() const
	{
		return m_comObj != nullptr;
	}

	ComObj* operator -> ()
	{
		return m_comObj;
	}

	ComObj*& GetRaw() 
	{ 
		return m_comObj; 
	}

private:
	ComObj* m_comObj{ nullptr };
};

class Camera
{
public:
	Camera(float fovy, float ar, float nearZ, float farZ)
		: m_fovy(fovy)
		, m_aspectRatio(ar)
		, m_nearZ(nearZ)
		, m_farZ(farZ)
	{}

	void SetPosition(DirectX::XMVECTOR newPos)
	{
		m_position = newPos;
	}

	void SetRotation(DirectX::XMVECTOR newRot)
	{
		m_rotation = newRot;
	}

	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetRotation() const { return m_rotation; }

	DirectX::XMMATRIX GetViewTransform() const
	{
		const DirectX::XMVECTOR zeroVector = DirectX::XMVectorZero();
		const DirectX::XMVECTOR unitVector = DirectX::XMVectorSplatOne();

		const DirectX::XMMATRIX cameraWorld = DirectX::XMMatrixAffineTransformation(unitVector, zeroVector, m_rotation, m_position);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(cameraWorld);
		return DirectX::XMMatrixInverse(&det, cameraWorld);
	}

	DirectX::XMMATRIX GetProjectionTransform() const
	{
		return DirectX::XMMatrixPerspectiveFovLH(m_fovy, m_aspectRatio, m_nearZ, m_farZ);
	}

	DirectX::XMMATRIX GetViewProjectionTransform() const
	{
		return DirectX::XMMatrixMultiply(GetViewTransform(), GetProjectionTransform());
	}

private:
	DirectX::XMVECTOR	m_position;
	DirectX::XMVECTOR	m_rotation;
	float				m_fovy;
	float				m_aspectRatio;
	float				m_nearZ;
	float				m_farZ;
};

class Shader
{
public:
	Shader(const std::wstring& shaderName)
	{
		// vertex shader
		{
			ComPtr<ID3DBlob> VSerrors;
			auto&& result = D3DCompileFromFile(shaderName.c_str(), 0, 0, "VShader", "vs_4_0", 0, 0, &m_vsBlob.GetRaw(), &VSerrors.GetRaw());
			assert(!FAILED(result));

			if (FAILED(result) && VSerrors)
			{
				const char* errorMsg = reinterpret_cast<const char*>(VSerrors->GetBufferPointer());
				std::cout << "Failed to compile vertex shader '" << shaderName.c_str() << "'!. Error: " << errorMsg << std::endl;
			}		
			
			// encapsulate both shaders into shader objects
			result = dev->CreateVertexShader(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), NULL, &m_vertexShader.GetRaw());
			assert(!FAILED(result));
		}

		// pixel shader
		{
			ComPtr<ID3DBlob> PSerrors;
			auto&& result = D3DCompileFromFile(shaderName.c_str(), 0, 0, "PShader", "ps_4_0", 0, 0, &m_psBlob.GetRaw(), &PSerrors.GetRaw());
			assert(!FAILED(result));

			if (FAILED(result) && PSerrors)
			{
				const char* errorMsg = reinterpret_cast<const char*>(PSerrors->GetBufferPointer());
				std::cout << "Failed to compile pixel shader '" << shaderName.c_str() << "'!. Error: " << errorMsg << std::endl;
			}

			// encapsulate both shaders into shader objects
			result = dev->CreatePixelShader(m_psBlob->GetBufferPointer(), m_psBlob->GetBufferSize(), NULL, &m_pixelShader.GetRaw());
			assert(!FAILED(result));
		}
	}

	void Bind(ID3D11DeviceContext* devcon)
	{
		devcon->VSSetShader(m_vertexShader.GetRaw(), 0, 0);
		devcon->PSSetShader(m_pixelShader.GetRaw(), 0, 0);
	}

	ComPtr<ID3DBlob>& GetVsBlob() { return m_vsBlob; }
	ComPtr<ID3DBlob>& GetPsBlob() { return m_psBlob; }

private:
	ComPtr<ID3D11VertexShader>	m_vertexShader;
	ComPtr<ID3D11PixelShader>	m_pixelShader;
	ComPtr<ID3DBlob>			m_vsBlob, m_psBlob;
};

class Model
{
	struct Mesh
	{
		std::string name;

		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

		unsigned int vertexCount;
		unsigned int indexCount;
	};

public:
	Model(std::istream& streamObj)
	{
		std::string line;
		//line.resize(255);

		struct Vector2
		{
			float x, y;
		};

		struct Vector3
		{
			float x, y, z;
		};

		struct FaceData
		{
			unsigned indexPos[3];
			unsigned indexNorm[3];
			unsigned indexUv[3];
		};

		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;
		std::vector<FaceData> faces;

		std::string begin;

		while (std::getline(streamObj, line))
		{
			std::stringstream lineStream(line);
			lineStream >> begin;

			if (begin == "v")
			{
				Vector3 position;
				lineStream >> position.x >> position.y >> position.z;
				positions.push_back(position);
			}
			else if (begin == "vn")
			{
				Vector3 normal;
				lineStream >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (begin == "vt")
			{
				Vector2 uv;
				lineStream >> uv.x >> uv.y;
				uvs.push_back(uv);
			}
			else if (begin == "f")
			{
				char tmp;

				FaceData faceData1;
				lineStream >> faceData1.indexPos[0] >> tmp >> faceData1.indexNorm[0] >> tmp >> faceData1.indexUv[0];
				lineStream >> faceData1.indexPos[1] >> tmp >> faceData1.indexNorm[1] >> tmp >> faceData1.indexUv[1];
				lineStream >> faceData1.indexPos[2] >> tmp >> faceData1.indexNorm[2] >> tmp >> faceData1.indexUv[2];

				FaceData faceData2;
				faceData2.indexPos[0] = faceData1.indexPos[0];
				faceData2.indexPos[1] = faceData1.indexPos[2];
				faceData2.indexNorm[0] = faceData1.indexNorm[0];
				faceData2.indexNorm[1] = faceData1.indexNorm[2];
				faceData2.indexUv[0] = faceData1.indexUv[0];
				faceData2.indexUv[1] = faceData1.indexUv[2];
				lineStream >> faceData2.indexPos[2] >> tmp >> faceData2.indexNorm[2] >> tmp >> faceData2.indexUv[2];

				faces.push_back(faceData1);
				faces.push_back(faceData2);

				//assert(faceData1.indexPos[0] >= positions.size()
				//	&& faceData1.indexPos[1] >= positions.size()
				//	&& faceData1.indexPos[2] >= positions.size()
				//	&& faceData2.indexPos[2] >= positions.size());
			}
			else if (begin == "o" || begin == "g")
			{
				auto mesh = std::make_unique<Mesh>();
				lineStream >> mesh->name;
				m_meshes.push_back(std::move(mesh));
			}

			begin.clear();
		}

	}

	Model(const std::string& path, Shader& shader)
	{
		objl::Loader loader;

		bool loadout = loader.LoadFile(path);
		if (loadout)
		{
			for (auto&& meshData : loader.LoadedMeshes)
			{
				auto&& mesh = std::make_unique<Mesh>();
				mesh->name = meshData.MeshName;

				// create the vertex buffer
				{
					D3D11_BUFFER_DESC vertexBufferDesc;
					ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

					vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					vertexBufferDesc.ByteWidth = sizeof(objl::Vertex) * meshData.Vertices.size();
					vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.CPUAccessFlags = 0;

					D3D11_SUBRESOURCE_DATA vertexData;
					vertexData.pSysMem = meshData.Vertices.data();
					vertexData.SysMemPitch = 0;
					vertexData.SysMemSlicePitch = 0;

					auto&& result = dev->CreateBuffer(&vertexBufferDesc, &vertexData, &mesh->vertexBuffer.GetRaw());
					assert(!FAILED(result));

					mesh->vertexCount = meshData.Vertices.size();
				}
						
				
				// create index buffer
				{
					D3D11_BUFFER_DESC indexBufferDesc;
					ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

					indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					indexBufferDesc.ByteWidth = meshData.Indices.size() * sizeof(unsigned int);
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					indexBufferDesc.CPUAccessFlags = 0;

					D3D11_SUBRESOURCE_DATA indexData;
					indexData.pSysMem = meshData.Indices.data();
					indexData.SysMemPitch = 0;
					indexData.SysMemSlicePitch = 0;

					auto&& result = dev->CreateBuffer(&indexBufferDesc, &indexData, &mesh->indexBuffer.GetRaw());
					assert(!FAILED(result));

					mesh->indexCount = meshData.Indices.size();
				}

				// create the input layout object
				{
					D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(objl::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(objl::Vector3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					};

					auto&& result = dev->CreateInputLayout(inputElementDesc, 3, shader.GetVsBlob()->GetBufferPointer(), shader.GetVsBlob()->GetBufferSize(), &mesh->inputLayout.GetRaw());
					assert(!FAILED(result));
				}

				m_meshes.push_back(std::move(mesh));
			}
		}
	}

	void Render(ID3D11DeviceContext* context)
	{
		for (auto&& mesh : m_meshes)
		{
			context->IASetInputLayout(mesh->inputLayout.GetRaw());

			// select which vertex buffer to display
			UINT stride = sizeof(objl::Vertex);
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &mesh->vertexBuffer.GetRaw(), &stride, &offset);
			context->IASetIndexBuffer(mesh->indexBuffer.GetRaw(), DXGI_FORMAT_R32_UINT, 0);

			// select which primtive type we are using
			context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// draw the vertex buffer to the back buffer
			context->DrawIndexed(mesh->indexCount, 0, 0);
		}
	}

private:
	DirectX::XMVECTOR					m_position;
	DirectX::XMVECTOR					m_rotation;
	DirectX::XMVECTOR					m_scale;
	std::vector<std::unique_ptr<Mesh>>	m_meshes;
};

struct ConstantBufferType
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 worldViewProjection;
	DirectX::XMFLOAT4 lightDir;
};

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(void);    // creates the shape to render
void InitPipeline(void);    // loads and prepares the shaders

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	{
		AllocConsole();

		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
		FILE* hf_out = _fdopen(hCrt, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;

		HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
		hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
		FILE* hf_in = _fdopen(hCrt, "r");
		setvbuf(hf_in, NULL, _IONBF, 128);
		*stdin = *hf_in;
	}

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

	// set up and initialize Direct3D
	InitD3D(hWnd);

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

		RenderFrame();
	}

	// clean up DirectX and COM
	CleanD3D();

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const float k_cameraMoveSpeed = 1.0f;

	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
		{
			// Process the UP ARROW key. 
			auto&& cameraPos = g_camera->GetPosition();
			auto&& cameraRight = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), g_camera->GetRotation());

			g_camera->SetPosition(DirectX::XMVectorAdd(cameraPos, DirectX::XMVectorScale(cameraRight, -k_cameraMoveSpeed)));

			break;
		}

		case VK_RIGHT:
		{
			// Process the UP ARROW key. 
			auto&& cameraPos = g_camera->GetPosition();
			auto&& cameraRight = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), g_camera->GetRotation());

			g_camera->SetPosition(DirectX::XMVectorAdd(cameraPos, DirectX::XMVectorScale(cameraRight, k_cameraMoveSpeed)));

			break;
		}

		case VK_UP:
		{
			// Process the UP ARROW key. 
			auto&& cameraPos = g_camera->GetPosition();
			auto&& cameraFront = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), g_camera->GetRotation());

			g_camera->SetPosition(DirectX::XMVectorAdd(cameraPos, DirectX::XMVectorScale(cameraFront, k_cameraMoveSpeed)));

			break;
		}

		case VK_DOWN:
		{
			// Process the UP ARROW key. 
			auto&& cameraPos = g_camera->GetPosition();
			auto&& cameraFront = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), g_camera->GetRotation());

			g_camera->SetPosition(DirectX::XMVectorAdd(cameraPos, DirectX::XMVectorScale(cameraFront, -k_cameraMoveSpeed)));

			break;
		}

		case VK_NUMPAD4:
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

			cameraYawAngle -= DirectX::XMConvertToRadians(1.0);

			g_camera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(cameraPitchAngle, cameraYawAngle, 0.0f));

			break;
		}

		case VK_NUMPAD6:
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

			cameraYawAngle += DirectX::XMConvertToRadians(1.0);

			g_camera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(cameraPitchAngle, cameraYawAngle, 0.0f));

			break;
		}

		case VK_NUMPAD8:
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

			cameraPitchAngle -= DirectX::XMConvertToRadians(1.0);

			g_camera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(cameraPitchAngle, cameraYawAngle, 0.0f));

			break;
		}

		case VK_NUMPAD2:
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

			cameraPitchAngle += DirectX::XMConvertToRadians(1.0);

			g_camera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(cameraPitchAngle, cameraYawAngle, 0.0f));

			break;
		}

		default:
			break;
		}
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
		NULL,
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
	assert(!FAILED(result));

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
	assert(!FAILED(result));

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
	assert(!FAILED(result));

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, pDepthStencilView);

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

	InitPipeline();
	InitGraphics();


	const float fovy = DirectX::XMConvertToRadians(45.0f);
	const float ar = (float)WIDTH / (float)HEIGHT;

	g_camera = std::make_unique<Camera>(fovy, ar, 0.5f, 500.0f);
	g_camera->SetPosition(DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f));
	g_camera->SetRotation(DirectX::XMQuaternionIdentity());

	g_shader = std::make_unique<Shader>(L"shaders.shader");
	g_model = std::make_unique<Model>("models/247_House 15_obj.obj", *g_shader);
	//g_model = std::make_unique<Model>("models/luxury house interior.obj", *g_shader);
}


// this is the function that creates the shape to render
void InitGraphics()
{
	//// create a triangle using the VERTEX struct
	//VERTEX OurVertices[] =
	//{
	//	{ 0.0f, 0.5f, 1.0f, {1.0f, 0.0f, 0.0f, 1.0f} },
	//	{ 0.45f, -0.5, 1.0f, {0.0f, 1.0f, 0.0f, 1.0f} },
	//	{ -0.45f, -0.5f, 1.0f, {0.0f, 0.0f, 1.0f, 1.0f} },
	//
	//	{ -0.5f, 0.5f, 0.0f,{ 1.0f, 1.0f, 1.0f, 1.0f } },
	//	{ 0.45f, -0.5, 0.0f,{ 1.0f, 1.0f, 1.0f, 1.0f } },
	//	{ -0.45f, -0.5f, 0.0f,{ 1.0f, 1.0f, 1.0f, 1.0f } },
	//};
	//
	//unsigned indices[] =
	//{
	//	0, 1, 2,
	//	3, 4, 5,
	//};
	//
	//// create the vertex buffer
	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));
	//
	//bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	//bd.ByteWidth = sizeof(VERTEX) * 6;             // size is the VERTEX struct * 3
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	//
	//dev->CreateBuffer(&bd, NULL, &pVBuffer);       // create the buffer
	//
	//// create index buffer
	//ZeroMemory(&bd, sizeof(bd));
	//
	//bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	//bd.ByteWidth = sizeof(indices);             // size is the VERTEX struct * 3
	//bd.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	//
	// // Give the subresource structure a pointer to the index data.
	//D3D11_SUBRESOURCE_DATA indexData;
	//indexData.pSysMem = indices;
	//indexData.SysMemPitch = 0;
	//indexData.SysMemSlicePitch = 0;
	//
	//dev->CreateBuffer(&bd, &indexData, &pIBuffer);       // create the buffer
	//
	//											   // copy the vertices into the buffer
	//D3D11_MAPPED_SUBRESOURCE ms;
	//devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	//memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
	//devcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer
	//
	////devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	////memcpy(ms.pData, indices, sizeof(indices));                 // copy the data
	////devcon->Unmap(pIBuffer, NULL);                                      // unmap the buffer


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
	assert(!FAILED(result));
	

	//// Lock the constant buffer so it can be written to.
	//result = devcon->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	//if (FAILED(result))
	//{
	//	//return false;
	//}
	//
	//// Get a pointer to the data in the constant buffer.
	//auto dataPtr = (ConstantBufferType*)ms.pData;
	//
	//// Copy the matrices into the constant buffer.
	//dataPtr->tintColor[0] = 0.5f;
	//dataPtr->tintColor[1] = 0.05f;
	//dataPtr->tintColor[2] = 0.05f;
	//dataPtr->tintColor[3] = 1.0f;
	//
	//// Unlock the constant buffer.
	//devcon->Unmap(pConstantBuffer, 0);

}


// this function loads and prepares the shaders
void InitPipeline()
{
	//// load and compile the two shaders
	//ID3D10Blob *VS, *PS;
	//ID3D10Blob *VSerrors, *PSerrors;
	//D3DCompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &VS, &VSerrors);
	//D3DCompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &PS, &PSerrors);
	//
	//if (!VS && VSerrors)
	//{
	//	const char* errorMsg = reinterpret_cast<const char*>(VSerrors->GetBufferPointer());
	//	assert(false && errorMsg);
	//}
	//
	//// encapsulate both shaders into shader objects
	//dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	//dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
	//
	//// set the shader objects
	//devcon->VSSetShader(pVS, 0, 0);
	//devcon->PSSetShader(pPS, 0, 0);
	//
	//// create the input layout object
	//D3D11_INPUT_ELEMENT_DESC ied[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//
	//dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
}


// this is the function used to render a single frame
void RenderFrame(void)
{
	// clear the back buffer to a deep blue
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	devcon->ClearRenderTargetView(backbuffer, clearColor);
	devcon->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Lock the constant buffer so it can be written to.
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		auto result = devcon->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		assert(!FAILED(result));

		// Get a pointer to the data in the constant buffer.
		auto dataPtr = (ConstantBufferType*)ms.pData;

		auto world = DirectX::XMMatrixIdentity();
		auto viewProj = g_camera->GetViewProjectionTransform();
		auto worldViewProj = world * viewProj;

		//std::array<DirectX::XMVECTOR, 6> testPoints =
		//{ {
		//	DirectX::XMVectorSet(5.0f, 5.0f, -4.5f, 1.0f),
		//	DirectX::XMVectorSet(5.0f, 5.0f, -5.0f, 1.0f),
		//	DirectX::XMVectorSet(5.0f, 5.0f, 0.0f, 1.0f),
		//	DirectX::XMVectorSet(5.0f, 5.0f, -5.0f + 0.5f + 500.0f, 1.0f),
		//	DirectX::XMVectorSet(5.0f, 5.0f, -2.0f, 1.0f),
		//	DirectX::XMVectorSet(5.0f, 5.0f, -3.0f, 1.0f),
		//} };
		//
		//for (size_t i = 0; i < testPoints.size(); i++)
		//{
		//	auto testPoint = testPoints[i];
		//	auto clipSpace = DirectX::XMVector4Transform(testPoint, g_camera->GetViewProjectionTransform());
		//	auto w = DirectX::XMVectorGetW(clipSpace);
		//	auto ndcSpace = DirectX::XMVectorScale(clipSpace, 1.0f / w);
		//
		//	int a = 5655;
		//}

		DirectX::XMVECTOR lightDir = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f));

		// Copy the matrices into the constant buffer.
		DirectX::XMStoreFloat4x4(&dataPtr->world, world);
		DirectX::XMStoreFloat4x4(&dataPtr->view, g_camera->GetViewTransform());
		DirectX::XMStoreFloat4x4(&dataPtr->worldViewProjection, worldViewProj);
		DirectX::XMStoreFloat4(&dataPtr->lightDir, lightDir);

		// Unlock the constant buffer.
		devcon->Unmap(pConstantBuffer, 0);

		// Finanly set the constant buffer in the vertex shader with the updated values.
		devcon->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	}

	g_shader->Bind(devcon);
	g_model->Render(devcon);

	//// 
	//devcon->IASetInputLayout(pLayout);
	//
	//// select which vertex buffer to display
	//UINT stride = sizeof(VERTEX);
	//UINT offset = 0;
	//devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	//devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
	//
	//// select which primtive type we are using
	//devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//
	//// draw the vertex buffer to the back buffer
	//devcon->DrawIndexed(3, 0, 0);
	//devcon->DrawIndexed(3, 3, 0);

	// switch the back buffer and the front buffer
	swapchain->Present(1, 0);
}


// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
	g_camera.reset();
	g_model.reset();
	g_shader.reset();

	// close and release all existing COM objects
	//pLayout->Release();
	//pVS->Release();
	//pPS->Release();
	//pIBuffer->Release();
	//pVBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	m_depthStencilBuffer->Release();
	m_depthStencilState->Release();
	pDepthStencilView->Release();
	dev->Release();
	devcon->Release();
}