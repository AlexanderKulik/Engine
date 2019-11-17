#include "pch.h"

#include <WICTextureLoader.h>
#include <assert.h>

#include "Texture.h"

Texture::TextureCache Texture::s_textureCache;

std::shared_ptr<Texture> Texture::CreateTexture(ID3D11Device* dev, const std::wstring& textureName)
{
	auto&& textureIt = s_textureCache.find(textureName);
	if (textureIt == s_textureCache.end())
	{
		auto texture = std::make_shared<Texture>(dev, L"textures/" + textureName);

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
}

Texture::Texture(ID3D11Device* dev, const std::wstring& textureName)
{
	auto&& result = DirectX::CreateWICTextureFromFile(dev, textureName.c_str(), nullptr, m_texture.GetAddressOf());
	assert(SUCCEEDED(result));

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = dev->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());
	assert(SUCCEEDED(result));
}

void Texture::Bind(ID3D11DeviceContext* devcon)
{
	devcon->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	devcon->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
}