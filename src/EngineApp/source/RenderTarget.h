#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

class Texture;

class RenderTarget
{
public:
	RenderTarget(ID3D11Device* dev, unsigned width, unsigned height, const std::string& rtFormat, const std::string& dsFormat);

	void												Bind(ID3D11DeviceContext* devcon);

	void												SetClearColor(const DirectX::XMFLOAT4& clearColor);
	void												SetViewport(const DirectX::SimpleMath::Rectangle& viewport);

	Texture*											GetColorTexture() const;
	Texture*											GetDepthTexture() const;

	const DirectX::XMFLOAT4&							GetClearColor() const;
	const DirectX::SimpleMath::Rectangle&				GetViewport() const;
	unsigned											GetWidth() const;
	unsigned											GetHeight() const;

private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_depthStencilView;

	std::unique_ptr<Texture>							m_colorTexture;
	std::unique_ptr<Texture>							m_depthTexture;

	DirectX::XMFLOAT4									m_clearColor;
	DirectX::SimpleMath::Rectangle						m_viewport;
};

inline void RenderTarget::SetClearColor(const DirectX::XMFLOAT4& clearColor)
{
	m_clearColor = clearColor;
}

inline void RenderTarget::SetViewport(const DirectX::SimpleMath::Rectangle& viewport)
{
	m_viewport = viewport;
}

inline Texture* RenderTarget::GetColorTexture() const
{
	return m_colorTexture.get();
}

inline Texture* RenderTarget::GetDepthTexture() const
{
	return m_depthTexture.get();
}

inline const DirectX::XMFLOAT4& RenderTarget::GetClearColor() const
{
	return m_clearColor;
}

inline const DirectX::SimpleMath::Rectangle& RenderTarget::GetViewport() const
{
	return m_viewport;
}

inline unsigned RenderTarget::GetWidth() const
{
	return 0;
}

inline unsigned RenderTarget::GetHeight() const
{
	return 0;
}
