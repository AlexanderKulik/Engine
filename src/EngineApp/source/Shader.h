#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct VertexBufferDesc;

class BlendState
{
public:
	enum BlendFactor
	{
		ONE,
		ZERO,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DST_ALPHA,
		INV_DEST_ALPHA,
		SRC_COLOR,
		INV_SRC_COLOR
	};

	enum BlendOp
	{
		ADD,
		SUBSTRACT
	};

	BlendState();
	BlendState(bool enabled, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, BlendOp op);
	BlendState(const BlendState& other);

private:
	bool m_enabled;
	BlendFactor m_srcColor, m_dstColor;
	BlendFactor m_srcAlpha, m_dstAlpha;
	BlendOp m_op;
	size_t m_hash;
};

enum class InputSemantic
{
	POSITION	= 1 << 0,
	NORMAL		= 1 << 1,
	TEXCOORD0	= 1 << 2,
	TEXCOORD1	= 1 << 3,
	TEXCOORD2	= 1 << 4,
	TEXCOORD3	= 1 << 5,
	TANGENT		= 1 << 6,
	COLOR		= 1 << 7,

	UNKNOWN = 0,
};

enum class InputType
{
	R32_UINT,
	R32_SINT,
	R32_FLOAT,

	R32G32_UINT,
	R32G32_SINT,
	R32G32_FLOAT,
	
	R32G32B32_UINT,
	R32G32B32_SINT,
	R32G32B32_FLOAT,

	R32G32B32A32_UINT,
	R32G32B32A32_SINT,
	R32G32B32A32_FLOAT,

	UNKNOWN,
};

class Shader
{
	struct ShaderInputDesc
	{

	};

public:
	Shader(ID3D11Device* dev, const std::wstring& shaderName);

	void										Bind(ID3D11DeviceContext* devcon);
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	RequestInputLayout(ID3D11Device* dev, const VertexBufferDesc& vertexBufferDesc);

private:
	using InputLayoutsCache = std::vector<std::pair<size_t, Microsoft::WRL::ComPtr<ID3D11InputLayout>>>;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob>			m_vsBytecode;
	
	InputLayoutsCache							m_inputLayouts;
};
