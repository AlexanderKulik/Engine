#pragma once

#include "BlendState.h"

class Shader;
class Texture;

const unsigned MAX_SAMPLERS = 16;

struct UniformHandle
{
	static const unsigned Invalid = (unsigned)-1;

	unsigned idx;
};

struct MaterialParamsStorage
{
	void* mem = nullptr;
};

enum class ComparsionFunc
{
	NEVER,
	LESS,
	EQUAL,
	LESS_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_EQUAL,
	ALWAYS
};

enum class StencilOp
{
	KEEP,
	ZERO,
	REPLACE,
	INCR_SAT,
	DECR_SAT,
	INVERT,
	INCR,
	DECR
};

struct DepthStencilState
{
	bool depthTest = true;
	bool depthWrite = true;

	bool stencilTest = false;
	unsigned char stencilReadMask = 0xFF;
	unsigned char stencilWriteMask = 0xFF;
	unsigned char stencilRef = 0;

	StencilOp frontFaceStencilFailOp = StencilOp::KEEP;
	StencilOp frontFaceStencilDepthFailOp = StencilOp::INCR;
	StencilOp frontFaceStencilPassOp = StencilOp::KEEP;

	StencilOp backFaceStencilFailOp = StencilOp::KEEP;
	StencilOp backFaceStencilDepthFailOp = StencilOp::DECR;
	StencilOp backFaceStencilPassOp = StencilOp::KEEP;

	ComparsionFunc frontFaceStencilFunc = ComparsionFunc::ALWAYS;
	ComparsionFunc backFaceStencilFunc = ComparsionFunc::ALWAYS;
};

class Material
{
public:
	static void CleanCaches();

public:
	Material();
	Material(const Shader* shader);
	Material(const Material& other);
	Material(Material&& other);

	~Material();

	Material& operator = (const Material& other);
	Material& operator = (Material&& other);

	UniformHandle FindUniformHandle(const std::string& name) const;
	UniformHandle FindTextureHandle(const std::string& name) const;

	void SetUniform(UniformHandle handle, float value);
	void SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector2& value);
	void SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector3& value);
	void SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector4& value);
	void SetUniform(UniformHandle handle, const DirectX::SimpleMath::Matrix& value);
	void SetUniform(UniformHandle handle, float values[], unsigned count);

	void SetUniform(const std::string& name, float value);
	void SetUniform(const std::string& name, const DirectX::SimpleMath::Vector2& value);
	void SetUniform(const std::string& name, const DirectX::SimpleMath::Vector3& value);
	void SetUniform(const std::string& name, const DirectX::SimpleMath::Vector4& value);
	void SetUniform(const std::string& name, const DirectX::SimpleMath::Matrix& value);
	void SetUniform(const std::string& name, float values[], unsigned count);

	void SetTexture(UniformHandle handle, Texture* texture);
	void SetTexture(const std::string& name, Texture* texture); 

	void SetBlendState(const BlendState& blendState);

	void SetDepthTest(bool yes);
	void SetDepthWrite(bool yes);

	void SetStencilTest(bool yes);
	void SetStencilMask(unsigned char readMask, unsigned char writeMask, unsigned char ref);
	void SetStencilFrontFace(StencilOp fail, StencilOp zfail, StencilOp pass, ComparsionFunc func);
	void SetStencilBackFace(StencilOp fail, StencilOp zfail, StencilOp pass, ComparsionFunc func);

	void Reset();

	const BlendState& GetBlendState() const;
	const Shader* GetShader() const;
	size_t GetHash() const;
	bool IsValid() const;

	void Bind(ID3D11Device* device, ID3D11DeviceContext* context) const;

private:
	static std::vector<std::pair<BlendState, Microsoft::WRL::ComPtr<ID3D11BlendState>>> s_blendStatesCache;
	static std::vector<std::pair<DepthStencilState, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>>> s_depthStencilStatesCache;

private:
	const Shader*			m_shader;
	MaterialParamsStorage	m_storage;
	Texture*				m_samplers[MAX_SAMPLERS];
	BlendState				m_blendState;
	DepthStencilState		m_depthStencilState;
	// hash
};

inline const Shader* Material::GetShader() const
{
	return m_shader;
}