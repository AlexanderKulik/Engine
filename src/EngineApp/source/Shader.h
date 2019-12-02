#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11ShaderReflectionConstantBuffer;

struct _D3D11_SHADER_BUFFER_DESC;

struct VertexBufferDesc;
class Material;

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
	friend class Material;

	struct ConstantShaderBuffer
	{
		struct ShaderVariable
		{
			std::string name;
			unsigned length;
			unsigned offset;
		};

		ConstantShaderBuffer(unsigned slot, const std::string& name, ID3D11ShaderReflectionConstantBuffer* buffer, const _D3D11_SHADER_BUFFER_DESC& bdesc);

		std::string mName;
		unsigned mSlot;
		unsigned mSize = 0;
		std::vector<ShaderVariable> mVariables;
	};


public:
	Shader(ID3D11Device* dev, const std::wstring& shaderName);

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	RequestInputLayout(ID3D11Device* dev, const VertexBufferDesc& vertexBufferDesc);

private:
	void										Bind(ID3D11DeviceContext* devcon) const;
	HRESULT										CreateConstantBufferReflection(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice);

private:
	using InputLayoutsCache = std::vector<std::pair<size_t, Microsoft::WRL::ComPtr<ID3D11InputLayout>>>;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob>			m_vsBytecode;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
	
	InputLayoutsCache							m_inputLayouts;
	std::vector<ConstantShaderBuffer>			m_shaderBuffers;
};
