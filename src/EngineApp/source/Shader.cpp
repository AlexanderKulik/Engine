#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <cassert>
#include <iostream>

#include "Shader.h"

Shader::Shader(ID3D11Device* dev, const std::wstring& shaderName)
{
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob;

	// vertex shader 
	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	auto&& result = D3DCompileFromFile(shaderName.c_str(), 0, 0, "VShader", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errors.GetAddressOf());
	assert(SUCCEEDED(result));

	if (FAILED(result) && errors)
	{
		const char* errorMsg = reinterpret_cast<const char*>(errors->GetBufferPointer());
		std::cout << "Failed to compile vertex shader '" << shaderName.c_str() << "'!. Error: " << errorMsg << std::endl;
	}

	// encapsulate both shaders into shader objects
	result = dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, m_vertexShader.GetAddressOf());
	assert(SUCCEEDED(result));

	// pixel shader
	result = D3DCompileFromFile(shaderName.c_str(), 0, 0, "PShader", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errors.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(result));

	if (FAILED(result) && errors)
	{
		const char* errorMsg = reinterpret_cast<const char*>(errors->GetBufferPointer());
		std::cout << "Failed to compile pixel shader '" << shaderName.c_str() << "'!. Error: " << errorMsg << std::endl;
	}

	// encapsulate both shaders into shader objects
	result = dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, m_pixelShader.GetAddressOf());
	assert(SUCCEEDED(result));

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMFLOAT3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = dev->CreateInputLayout(inputElementDesc, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
	assert(SUCCEEDED(result));
}

void Shader::Bind(ID3D11DeviceContext* devcon)
{
	devcon->IASetInputLayout(m_inputLayout.Get());

	devcon->VSSetShader(m_vertexShader.Get(), 0, 0);
	devcon->PSSetShader(m_pixelShader.Get(), 0, 0);
}