#include "pch.h"

#include <WICTextureLoader.h>
#include <assert.h>

#include "Texture.h"

D3D11_FILTER g_filterTable[] =
{
	D3D11_ENCODE_BASIC_FILTER(D3D11_FILTER_TYPE_POINT, D3D11_FILTER_TYPE_POINT, D3D11_FILTER_TYPE_POINT, D3D11_FILTER_REDUCTION_TYPE_STANDARD),
	D3D11_ENCODE_BASIC_FILTER(D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_POINT, D3D11_FILTER_REDUCTION_TYPE_STANDARD),
	D3D11_ENCODE_BASIC_FILTER(D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_TYPE_LINEAR, D3D11_FILTER_REDUCTION_TYPE_STANDARD),
};

D3D11_TEXTURE_ADDRESS_MODE g_wrapTable[] =
{
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_BORDER,
};

bool operator == (const SamplerStateDesc& lhs, const SamplerStateDesc& rhs)
{
	return lhs.filter == rhs.filter
		&& lhs.wrapU == rhs.wrapU
		&& lhs.wrapV == rhs.wrapV
		&& lhs.wrapW == rhs.wrapW
		&& lhs.aniso == rhs.aniso;
}

/////////////////////////////////////////////////////

Texture::TextureCache Texture::s_textureCache;
Texture::SamplerStatesCache Texture::s_samplerStatesCache;

std::shared_ptr<Texture> Texture::CreateTexture(ID3D11Device* dev, const std::wstring& textureName)
{
	auto&& textureIt = s_textureCache.find(textureName);
	if (textureIt == s_textureCache.end())
	{
		auto texture = std::make_shared<Texture>(dev, textureName);

		s_textureCache.emplace(textureName, texture);

		return texture;
	}
	else
	{
		return textureIt->second;
	}
}

void Texture::ClearUnreferenced()
{
	s_textureCache.clear();
	s_samplerStatesCache.clear();
}

/////////////////////////////////////////////////////

Texture::Texture(ID3D11Device* dev, const std::wstring& textureName)
{
	auto&& result = DirectX::CreateWICTextureFromFile(dev, textureName.c_str(), nullptr, m_texture.GetAddressOf());
	assert(SUCCEEDED(result));
}

Texture::Texture(ID3D11Device * dev, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView)
	: m_texture(shaderResourceView)
{}

void Texture::Bind(ID3D11Device* device, ID3D11DeviceContext* context, unsigned bindSlot)
{
	auto samplerStateIt = std::find_if(s_samplerStatesCache.begin(), s_samplerStatesCache.end(), [this](auto&& val)
	{
		return val.first == m_samplerState;
	});

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	if (samplerStateIt == s_samplerStatesCache.end())
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		samplerDesc.Filter = m_samplerState.aniso > 1 ? D3D11_FILTER::D3D11_FILTER_MAXIMUM_ANISOTROPIC : g_filterTable[static_cast<unsigned>(m_samplerState.filter)];
		samplerDesc.AddressU = g_wrapTable[static_cast<unsigned>(m_samplerState.wrapU)];
		samplerDesc.AddressV = g_wrapTable[static_cast<unsigned>(m_samplerState.wrapV)];
		samplerDesc.AddressW = g_wrapTable[static_cast<unsigned>(m_samplerState.wrapW)];
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = m_samplerState.aniso;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		auto&& result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		assert(SUCCEEDED(result));

		s_samplerStatesCache.emplace_back(m_samplerState, samplerState);
	}
	else
	{
		samplerState = samplerStateIt->second;
	}

	context->PSSetSamplers(bindSlot, 1, samplerState.GetAddressOf());
	context->PSSetShaderResources(bindSlot, 1, m_texture.GetAddressOf());
}

void Texture::SetAnisoLevel(unsigned anisoLevel)
{
	assert(anisoLevel <= 16);
	m_samplerState.aniso = anisoLevel;
}

void Texture::SetTextureFilter(TextureFilter filter)
{
	m_samplerState.filter = filter;
}

void Texture::SetWrapMode(WrapMode wrapModeUVW)
{
	m_samplerState.wrapU = m_samplerState.wrapV = m_samplerState.wrapW = wrapModeUVW;
}

void Texture::SetWrapModeU(WrapMode wrapModeU)
{
	m_samplerState.wrapU = wrapModeU;
}

void Texture::SetWrapModeV(WrapMode wrapModeV)
{
	m_samplerState.wrapV = wrapModeV;
}

void Texture::SetWrapModeW(WrapMode wrapModeW)
{
	m_samplerState.wrapW = wrapModeW;
}