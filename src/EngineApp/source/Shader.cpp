#include "pch.h"

#include <d3dcompiler.h>
#include <cassert>
#include <iostream>

#include "Shader.h"

HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;
	if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	inputLayoutDesc.reserve(shaderDesc.InputParameters);

	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	return pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);
}

HRESULT CreateConstantBufferReflection(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;
	if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	//
	//Find all constant buffers
	//
	for (unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		unsigned int register_index = 0;
		ID3D11ShaderReflectionConstantBuffer* buffer = NULL;
		buffer = pVertexShaderReflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bdesc;
		buffer->GetDesc(&bdesc);

		for (unsigned int k = 0; k < shaderDesc.BoundResources; ++k)
		{
			D3D11_SHADER_INPUT_BIND_DESC ibdesc;
			pVertexShaderReflection->GetResourceBindingDesc(k, &ibdesc);

			if (!strcmp(ibdesc.Name, bdesc.Name))
				register_index = ibdesc.BindPoint;
		}

		//
		//Add constant buffer
		//
		//oConstantShaderBuffer* shaderbuffer = new ConstantShaderBuffer(register_index, Engine::String.ConvertToWideStr(bdesc.Name), buffer, &bdesc);
		//mShaderBuffers.push_back(shaderbuffer);
	}

		//
		//Populate constant buffer with variables
		//
	//for (unsigned int j = 0; j < desc->Variables; ++j)
	//{
	//	ID3D11ShaderReflectionVariable* variable = NULL;
	//	variable = buffer->GetVariableByIndex(j);
	//
	//	D3D11_SHADER_VARIABLE_DESC vdesc;
	//	variable->GetDesc(&vdesc);
	//
	//	ShaderVariable* shadervariable = new ShaderVariable();
	//	shadervariable->name = Engine::String.ConvertToWideStr(vdesc.Name);
	//	shadervariable->length = vdesc.Size;
	//	shadervariable->offset = vdesc.StartOffset;
	//	mSize += vdesc.Size;
	//	mVariables.push_back(shadervariable);
	//}

	return S_OK;
}

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

	//// create the input layout object
	//D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMFLOAT3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//
	//result = dev->CreateInputLayout(inputElementDesc, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
	//assert(SUCCEEDED(result));

	result = CreateInputLayoutDescFromVertexShaderSignature(vsBlob.Get(), dev, m_inputLayout.GetAddressOf());
	assert(SUCCEEDED(result));
}

void Shader::Bind(ID3D11DeviceContext* devcon)
{
	devcon->IASetInputLayout(m_inputLayout.Get());

	devcon->VSSetShader(m_vertexShader.Get(), 0, 0);
	devcon->PSSetShader(m_pixelShader.Get(), 0, 0);
}