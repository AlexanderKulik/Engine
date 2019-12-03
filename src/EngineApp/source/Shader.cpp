#include "pch.h"

#include <d3dcompiler.h>
#include <cassert>
#include <iostream>
#include <boost/container_hash/hash.hpp>

#include "Shader.h"
#include "Model.h"

InputSemantic ConvertSemanticNameToEnum(const char* semanticName, unsigned semanticSlot)
{
	if (!strcmp(semanticName, "POSITION"))
	{
		return InputSemantic::POSITION;
	}
	else if (!strcmp(semanticName, "NORMAL"))
	{
		return InputSemantic::NORMAL;
	}
	else if (!strcmp(semanticName, "TEXCOORD"))
	{
		return static_cast<InputSemantic>(static_cast<unsigned>(InputSemantic::TEXCOORD0) + semanticSlot);
	}
	//else if (!strcmp(semanticName, "TEXCOORD0"))
	//{
	//	return InputSemantic::TEXCOORD0;
	//}
	//else if (!strcmp(semanticName, "TEXCOORD1"))
	//{
	//	return InputSemantic::TEXCOORD1;
	//}
	//else if (!strcmp(semanticName, "TEXCOORD2"))
	//{
	//	return InputSemantic::TEXCOORD2;
	//}
	//else if (!strcmp(semanticName, "TEXCOORD3"))
	//{
	//	return InputSemantic::TEXCOORD3;
	//}
	else if (!strcmp(semanticName, "TANGENT"))
	{
		return InputSemantic::TANGENT;
	}
	else if (!strcmp(semanticName, "COLOR"))
	{
		return InputSemantic::COLOR;
	}

	return InputSemantic::UNKNOWN;
}

DXGI_FORMAT ConvertInputTypeToDxgiFormat(InputType inputType)
{
	switch (inputType)
	{
	case InputType::R32_UINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32_UINT;

	case InputType::R32_SINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32_SINT;

	case InputType::R32_FLOAT:
		return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;

	case InputType::R32G32_UINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT;

	case InputType::R32G32_SINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT;

	case InputType::R32G32_FLOAT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;

	case InputType::R32G32B32_UINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT;

	case InputType::R32G32B32_SINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT;

	case InputType::R32G32B32_FLOAT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;

	case InputType::R32G32B32A32_UINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT;

	case InputType::R32G32B32A32_SINT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_SINT;

	case InputType::R32G32B32A32_FLOAT:
		return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;

	default:
		assert(false && "Unknown Input type!");
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}
}

//////////////////////////////////////////////////////////////

Shader::ConstantShaderBuffer::ConstantShaderBuffer(unsigned slot,
	const std::string& name,
	ID3D11ShaderReflectionConstantBuffer* buffer,
	const D3D11_SHADER_BUFFER_DESC& bdesc)
	: mName(name)
	, mSlot(slot)
{
	// Populate constant buffer with variables
	for (unsigned int j = 0; j < bdesc.Variables; ++j)
	{
		ID3D11ShaderReflectionVariable* variable = NULL;
		variable = buffer->GetVariableByIndex(j);

		D3D11_SHADER_VARIABLE_DESC vdesc;
		variable->GetDesc(&vdesc);

		ShaderVariable shadervariable;
		shadervariable.name = vdesc.Name;
		shadervariable.length = vdesc.Size;
		shadervariable.offset = vdesc.StartOffset;
		//mSize += vdesc.Size;
		mVariables.push_back(shadervariable);
	}

	mSize = bdesc.Size;
	//assert(mSize == bdesc.Size);
}

//////////////////////////////////////////////////////////////

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

HRESULT Shader::CreateConstantBufferReflection(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice)
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

	//Find all constant buffers
	for (unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		unsigned int registerIndex = 0;
		ID3D11ShaderReflectionConstantBuffer* buffer = NULL;
		buffer = pVertexShaderReflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bdesc;
		buffer->GetDesc(&bdesc);

		for (unsigned int k = 0; k < shaderDesc.BoundResources; ++k)
		{
			D3D11_SHADER_INPUT_BIND_DESC ibdesc;
			pVertexShaderReflection->GetResourceBindingDesc(k, &ibdesc);

			if (!strcmp(ibdesc.Name, bdesc.Name))
			{
				registerIndex = ibdesc.BindPoint;
				break;
			}
		}

		m_shaderBuffers.emplace_back(registerIndex, bdesc.Name, buffer, bdesc);
	}

	return S_OK;
}

HRESULT Shader::CreateSamplersReflection(ID3DBlob * pShaderBlob, ID3D11Device * pD3DDevice)
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

	for (unsigned int i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC shaderInputDesc;
		HRESULT result = pVertexShaderReflection->GetResourceBindingDesc(i, &shaderInputDesc);
		assert(SUCCEEDED(result));

		if (shaderInputDesc.Type == D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE)
		{
			m_shaderSamplers.push_back({ shaderInputDesc.Name, shaderInputDesc.BindPoint });
		}
	}

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
	//
	//result = CreateInputLayoutDescFromVertexShaderSignature(vsBlob.Get(), dev, m_inputLayout.GetAddressOf());
	//assert(SUCCEEDED(result));
	//
	//BlendState blendState1(false, BlendState::SRC_ALPHA, BlendState::INV_SRC_ALPHA, BlendState::SRC_ALPHA, BlendState::INV_SRC_ALPHA, BlendState::ADD);
	//BlendState blendState2(true, BlendState::SRC_ALPHA, BlendState::INV_SRC_ALPHA, BlendState::SRC_ALPHA, BlendState::INV_SRC_ALPHA, BlendState::ADD);

	result = CreateConstantBufferReflection(vsBlob.Get(), dev);
	assert(SUCCEEDED(result));

	assert(m_shaderBuffers.size() == 1);

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));

	constantBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = m_shaderBuffers[0].mSize;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	result = dev->CreateBuffer(&constantBufferDesc, nullptr, m_constantBuffer.GetAddressOf());
	assert(SUCCEEDED(result));

	result = CreateSamplersReflection(psBlob.Get(), dev);
	assert(SUCCEEDED(result));

	m_vsBytecode = vsBlob;
}

void Shader::Bind(ID3D11DeviceContext* devcon) const
{
	devcon->VSSetShader(m_vertexShader.Get(), 0, 0);
	devcon->PSSetShader(m_pixelShader.Get(), 0, 0);

	devcon->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	devcon->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
}

Microsoft::WRL::ComPtr<ID3D11InputLayout> Shader::RequestInputLayout(ID3D11Device* dev, const VertexBufferDesc& vertexBufferDesc)
{
	auto it = std::find_if(m_inputLayouts.begin(), m_inputLayouts.end(), [&vertexBufferDesc](auto&& val)
	{
		return val.first == vertexBufferDesc.hash;
	});

	if (it != m_inputLayouts.end())
	{
		return it->second;
	}

	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;
	if (FAILED(D3DReflect(m_vsBytecode->GetBufferPointer(), m_vsBytecode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return nullptr;
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

		auto&& semantic = ConvertSemanticNameToEnum(paramDesc.SemanticName, paramDesc.SemanticIndex);
		assert(semantic != InputSemantic::UNKNOWN && "Shader has unknown input sematics %s!" && paramDesc.SemanticName);
		
		if (semantic == InputSemantic::UNKNOWN)
		{
			continue;
		}

		auto it = std::find_if(vertexBufferDesc.inputs.begin(), vertexBufferDesc.inputs.end(), [semantic](auto&& val)
		{
			return val.semantic == semantic;
		});
		assert(it != vertexBufferDesc.inputs.end() && "Geometry doesn't have requested by shader inputs!");

		if (it == vertexBufferDesc.inputs.end())
		{
			continue;
		}

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.Format = ConvertInputTypeToDxgiFormat(it->type);
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = it->offset;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	HRESULT result = dev->CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), m_vsBytecode->GetBufferPointer(), m_vsBytecode->GetBufferSize(), inputLayout.GetAddressOf());
	assert(SUCCEEDED(result));

	m_inputLayouts.emplace_back(vertexBufferDesc.hash, inputLayout);

	return inputLayout;
}
