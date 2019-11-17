#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

enum InputSemantic
{
	POSITION	= 1 << 0,
	NORMAL		= 1 << 1,
	TEXCOORD0	= 1 << 2,
	TEXCOORD1	= 1 << 3,
	COLOR		= 1 << 4,
	TANGENT		= 1 << 5,

	COUNT = 6
};

class Shader
{
public:
	Shader(ID3D11Device* dev, const std::wstring& shaderName);

	void										Bind(ID3D11DeviceContext* devcon);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
};
