#include "pch.h"

#include "Material.h"
#include "Shader.h"
#include "Texture.h"

Material::Material()
{
}

Material::Material(const Shader* shader)
	: m_shader(shader)
{
	if (m_shader)
	{
		auto&& constantBuffer = m_shader->m_shaderBuffers[0];
		auto&& size = constantBuffer.mSize;

		m_storage.mem = new char[size];

		memset(m_samplers, 0, sizeof(m_samplers));
	}
}

Material::Material(const Material& other)
	: m_shader(other.m_shader)
{
	if (m_storage.mem)
	{
		delete[] m_storage.mem;
	}

	if (m_shader)
	{
		auto&& constantBuffer = m_shader->m_shaderBuffers[0];
		auto&& size = constantBuffer.mSize;

		m_storage.mem = new char[size];

		memcpy(m_storage.mem, other.m_storage.mem, size);

		memcpy(m_samplers, other.m_samplers, sizeof(m_samplers));
	}
}

Material::Material(Material && other)
	: m_shader(other.m_shader)
{
	if (m_storage.mem)
	{
		delete[] m_storage.mem;
	}

	m_storage.mem = other.m_storage.mem;

	memcpy(m_samplers, other.m_samplers, sizeof(m_samplers));

	other.m_shader = nullptr;
	other.m_storage.mem = nullptr;
}

Material::~Material()
{
	if (m_storage.mem)
	{
		delete[] m_storage.mem;
	}
}

Material& Material::operator=(const Material& other)
{
	if (this != &other)
	{
		this->~Material();
		new (this) Material(other);
	}

	return *this;
}

Material& Material::operator=(Material&& other)
{
	if (this != &other)
	{
		this->~Material();
		new (this) Material(std::move(other));
	}

	return *this;
}

UniformHandle Material::FindUniformHandle(const std::string & name) const
{
	if (m_shader)
	{
		auto&& variables = m_shader->m_shaderBuffers[0].mVariables;
		auto it = std::find_if(variables.begin(), variables.end(), [&name](auto&& val)
		{
			return val.name == name;
		});

		if (it != variables.end())
		{
			return UniformHandle{ static_cast<unsigned>(std::distance(variables.begin(), it)) };
		}
	}

	return UniformHandle{ UniformHandle::Invalid };
}

UniformHandle Material::FindTextureHandle(const std::string & name) const
{
	if (m_shader)
	{
		auto&& samplers = m_shader->m_shaderSamplers;
		auto it = std::find_if(samplers.begin(), samplers.end(), [&name](auto&& val)
		{
			return val.name == name;
		});

		if (it != samplers.end())
		{
			return UniformHandle{ static_cast<unsigned>(std::distance(samplers.begin(), it)) };
		}
	}

	return UniformHandle{ UniformHandle::Invalid };
}

void Material::SetUniform(UniformHandle handle, float value)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length == sizeof(value));
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, &value, sizeof(value));
	}
}

void Material::SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector2& value)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length == sizeof(value));
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, &value.x, sizeof(value));
	}
}

void Material::SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector3& value)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length == sizeof(value));
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, &value.x, sizeof(value));
	}
}

void Material::SetUniform(UniformHandle handle, const DirectX::SimpleMath::Vector4& value)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length == sizeof(value));
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, &value.x, sizeof(value));
	}
}

void Material::SetUniform(UniformHandle handle, const DirectX::SimpleMath::Matrix& value)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length == sizeof(value));
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, &value.m, sizeof(value));
	}
}

void Material::SetUniform(UniformHandle handle, float values[], unsigned count)
{
	if (m_shader)
	{
		auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
		assert(uniformData.length <= sizeof(float) * count);
		auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
		memcpy(dst, values, sizeof(float) * count);
	}
}

void Material::SetUniform(const std::string& name, float value)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, value);
	}
}

void Material::SetUniform(const std::string& name, const DirectX::SimpleMath::Vector2& value)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, value);
	}
}

void Material::SetUniform(const std::string& name, const DirectX::SimpleMath::Vector3& value)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, value);
	}
}

void Material::SetUniform(const std::string& name, const DirectX::SimpleMath::Vector4& value)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, value);
	}
}

void Material::SetUniform(const std::string& name, const DirectX::SimpleMath::Matrix& value)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, value);
	}
}

void Material::SetUniform(const std::string& name, float values[], unsigned count)
{
	auto handle = FindUniformHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetUniform(handle, values, count);
	}
}

void Material::SetTexture(UniformHandle handle, Texture* texture)
{
	if (m_shader)
	{
		auto&& samplerData = m_shader->m_shaderSamplers[handle.idx];
		m_samplers[samplerData.slot] = texture;
	}
}

void Material::SetTexture(const std::string& name, Texture* texture)
{
	auto handle = FindTextureHandle(name);
	if (handle.idx != UniformHandle::Invalid)
	{
		SetTexture(handle, texture);
	}
}

void Material::Reset()
{
	assert(false);
}

void Material::Bind(ID3D11Device* device, ID3D11DeviceContext* context) const
{
	if (m_shader)
	{
		m_shader->Bind(context);

		auto&& constantBuffer = m_shader->m_shaderBuffers[0];
		auto&& size = constantBuffer.mSize;

		D3D11_MAPPED_SUBRESOURCE ms;
		auto result = context->Map(m_shader->m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		assert(SUCCEEDED(result));

		memcpy(ms.pData, m_storage.mem, size);

		context->Unmap(m_shader->m_constantBuffer.Get(), 0);

		for (unsigned i = 0; i < MAX_SAMPLERS; i++)
		{
			if (m_samplers[i])
			{
				m_samplers[i]->Bind(device, context, i);
			}
		}
	}
}
