#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;

struct TextureDimension
{
	enum
	{
		Texture2D,
		Texture3D,
		TextureCube,

		Unknown
	};
};

struct TextureFormat
{
	enum
	{
		R8,
		RGB888,
		RGBA8888,

		Unknown
	};
};

enum class TextureFilter
{
	POINT,
	BILINEAR,
	TRILINEAR,
};

enum class WrapMode
{
	CLAMP_TO_EDGE,
	REPEAT,
	MIRROR,
	BORDER,
};

struct SamplerStateDesc
{
	TextureFilter filter = TextureFilter::BILINEAR;
	WrapMode wrapU = WrapMode::CLAMP_TO_EDGE;
	WrapMode wrapV = WrapMode::CLAMP_TO_EDGE;
	WrapMode wrapW = WrapMode::CLAMP_TO_EDGE;
	unsigned aniso = 1;
};

class Texture
{
	

public:
	static std::shared_ptr<Texture>						CreateTexture(ID3D11Device* dev, const std::wstring& textureName);
	static void											ClearUnreferenced();

public:
	Texture(ID3D11Device* dev, const std::wstring& textureName);
	Texture(ID3D11Device* dev, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView);

	void												Bind(ID3D11Device* device, ID3D11DeviceContext* context, unsigned bindSlot);

	void												SetAnisoLevel(unsigned anisoLevel);
	void												SetTextureFilter(TextureFilter filter);
	void												SetWrapMode(WrapMode wrapModeUVW);
	void												SetWrapModeU(WrapMode wrapModeU);
	void												SetWrapModeV(WrapMode wrapModeV);
	void												SetWrapModeW(WrapMode wrapModeW);

	unsigned											GetAnisoLevel(unsigned anisoLevel) const;
	TextureFilter										GetTextureFilter(TextureFilter filter) const;
	WrapMode											GetWrapModeU(WrapMode wrapModeU) const;
	WrapMode											GetWrapModeV(WrapMode wrapModeV) const;
	WrapMode											GetWrapModeW(WrapMode wrapModeW) const;

	TextureDimension									GetTextureDimension() const;
	TextureFormat										GetTextureFormat() const;
	unsigned											GetWidth() const;
	unsigned											GetHeight() const;

private:
	using TextureCache = std::unordered_map<std::wstring, std::shared_ptr<Texture>>;
	using SamplerStatesCache = std::vector < std::pair<SamplerStateDesc, Microsoft::WRL::ComPtr<ID3D11SamplerState>>>;

	static TextureCache									s_textureCache;
	static SamplerStatesCache							s_samplerStatesCache;

private:
	//Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_texture;
	SamplerStateDesc									m_samplerState;
};