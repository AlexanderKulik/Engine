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

class Material
{
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
	const BlendState& GetBlendState() const;

	void Reset();

	const Shader* GetShader() const;
	size_t GetHash() const;
	bool IsValid() const;

	void Bind(ID3D11Device* device, ID3D11DeviceContext* context) const;

private:
	const Shader* m_shader;
	// hash
	MaterialParamsStorage m_storage;
	Texture* m_samplers[MAX_SAMPLERS];
};

inline const Shader* Material::GetShader() const
{
	return m_shader;
}