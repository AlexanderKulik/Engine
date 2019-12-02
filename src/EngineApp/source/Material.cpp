#include "pch.h"

#include "Material.h"
#include "Shader.h"

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
	auto&& uniformData = m_shader->m_shaderBuffers[0].mVariables[handle.idx];
	assert(uniformData.length <= sizeof(float) * count);
	auto&& dst = static_cast<char*>(m_storage.mem) + uniformData.offset;
	memcpy(dst, values, sizeof(float) * count);
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

void Material::Reset()
{
	assert(false);
}

void Material::Bind(ID3D11DeviceContext* devcon) const
{
	if (m_shader)
	{
		m_shader->Bind(devcon);

		auto&& constantBuffer = m_shader->m_shaderBuffers[0];
		auto&& size = constantBuffer.mSize;

		D3D11_MAPPED_SUBRESOURCE ms;
		auto result = devcon->Map(m_shader->m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		assert(SUCCEEDED(result));

		memcpy(ms.pData, m_storage.mem, size);

		// Unlock the constant buffer.
		devcon->Unmap(m_shader->m_constantBuffer.Get(), 0);
	}
}
