#include <d3d11.h>

#include "RenderTarget.h"

RenderTarget::RenderTarget(ID3D11Device* dev, unsigned width, unsigned height, const std::string& rtFormat, const std::string& dsFormat)
	: m_clearColor(0.0f, 0.0f, 0.0f, 0.0f)
	, m_viewport(0, 0, width, height)
{
	if (!rtFormat.empty())
	{
		// create color buffer
		{
			D3D11_TEXTURE2D_DESC colorBufferDesc;
			ZeroMemory(&colorBufferDesc, sizeof(colorBufferDesc));

			colorBufferDesc.Width = width;
			colorBufferDesc.Height = height;
			colorBufferDesc.MipLevels = 1;
			colorBufferDesc.ArraySize = 1;
			colorBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
			colorBufferDesc.SampleDesc.Count = 1;
			colorBufferDesc.SampleDesc.Quality = 0;
			colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			colorBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			colorBufferDesc.CPUAccessFlags = 0;
			colorBufferDesc.MiscFlags = 0;

			auto&& result = dev->CreateTexture2D(&colorBufferDesc, NULL, m_colorBuffer.GetAddressOf());
			assert(SUCCEEDED(result));
		}

		// create render target view
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));

			renderTargetViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;

			auto&& result = dev->CreateRenderTargetView(m_colorBuffer.Get(), &renderTargetViewDesc, m_renderTargetView.GetAddressOf());
			assert(SUCCEEDED(result));
		}

		// create shader resource view
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));

			shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

			auto&& result = dev->CreateShaderResourceView(m_colorBuffer.Get(), &shaderResourceViewDesc, m_shaderResourceView.GetAddressOf());
			assert(SUCCEEDED(result));
		}
	}

	//if (!dsFormat.empty())
	//{
	//	D3D11_TEXTURE2D_DESC colorBufferDesc;
	//	// Initialize the description of the depth buffer.
	//	ZeroMemory(&colorBufferDesc, sizeof(colorBufferDesc));
	//
	//	// Set up the description of the depth buffer.
	//	colorBufferDesc.Width = WIDTH;
	//	colorBufferDesc.Height = HEIGHT;
	//	colorBufferDesc.MipLevels = 1;
	//	colorBufferDesc.ArraySize = 1;
	//	colorBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//	colorBufferDesc.SampleDesc.Count = 1;
	//	colorBufferDesc.SampleDesc.Quality = 0;
	//	colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//	colorBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//	colorBufferDesc.CPUAccessFlags = 0;
	//	colorBufferDesc.MiscFlags = 0;
	//
	//	// Create the texture for the depth buffer using the filled out description.
	//	auto&& result = dev->CreateTexture2D(&colorBufferDesc, NULL, &m_depthStencilBuffer);
	//	assert(SUCCEEDED(result));
	//}	
}

void RenderTarget::Bind(ID3D11DeviceContext* devcon)
{
	devcon->ClearRenderTargetView(m_renderTargetView.Get(), &m_clearColor.x);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = static_cast<FLOAT>(m_viewport.x);
	vp.TopLeftY = static_cast<FLOAT>(m_viewport.y);
	vp.Width = static_cast<FLOAT>(m_viewport.width);
	vp.Height = static_cast<FLOAT>(m_viewport.height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	devcon->RSSetViewports(1, &vp);

	devcon->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
}