#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;

class Texture
{
public:
	static std::shared_ptr<Texture>						CreateTexture(ID3D11Device* dev, const std::wstring& textureName);
	static void											ClearUnreferenced();

public:
	Texture(ID3D11Device* dev, const std::wstring& textureName);

	void												Bind(ID3D11DeviceContext* devcon);

private:
	using TextureCache = std::unordered_map<std::wstring, std::shared_ptr<Texture>>;
	static TextureCache									s_textureCache;

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_texture;
};