#include "pch.h"

#include "Material.h"
#include "Shader.h"
#include "Texture.h"

bool operator == (const BlendState& lhs, const BlendState& rhs)
{
	return lhs.enabled == rhs.enabled
		&& lhs.srcColor == rhs.srcColor
		&& lhs.dstColor == rhs.dstColor
		&& lhs.srcAlpha == rhs.srcAlpha
		&& lhs.dstAlpha == rhs.dstAlpha
		&& lhs.op == rhs.op;
}

bool operator == (const DepthStencilState& lhs, const DepthStencilState& rhs)
{
	return  lhs.depthTest == rhs.depthTest
		&& lhs.depthWrite == rhs.depthWrite
		&& lhs.stencilTest == rhs.stencilTest
		&& lhs.stencilReadMask == rhs.stencilReadMask
		&& lhs.stencilWriteMask == rhs.stencilWriteMask
		&& lhs.frontFaceStencilFailOp == rhs.frontFaceStencilFailOp
		&& lhs.frontFaceStencilDepthFailOp == rhs.frontFaceStencilDepthFailOp
		&& lhs.frontFaceStencilPassOp == rhs.frontFaceStencilPassOp
		&& lhs.backFaceStencilFailOp == rhs.backFaceStencilFailOp
		&& lhs.backFaceStencilDepthFailOp == rhs.backFaceStencilDepthFailOp
		&& lhs.backFaceStencilPassOp == rhs.backFaceStencilPassOp
		&& lhs.frontFaceStencilFunc == rhs.frontFaceStencilFunc
		&& lhs.backFaceStencilFunc == rhs.backFaceStencilFunc;
}

std::vector<std::pair<BlendState, Microsoft::WRL::ComPtr<ID3D11BlendState>>> Material::s_blendStatesCache;
std::vector<std::pair<DepthStencilState, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>>> Material::s_depthStencilStatesCache;

void Material::CleanCaches()
{
	s_blendStatesCache.clear();
	s_depthStencilStatesCache.clear();
}

D3D11_BLEND g_blendTable[] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
};

D3D11_BLEND_OP g_blendOpTable[] =
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
};

D3D11_COMPARISON_FUNC g_comparsionFuncTable[] = 
{
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_ALWAYS,
};

D3D11_STENCIL_OP g_stencilOpTable[] = 
{
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_ZERO,
	D3D11_STENCIL_OP_REPLACE,
	D3D11_STENCIL_OP_INCR_SAT,
	D3D11_STENCIL_OP_DECR_SAT,
	D3D11_STENCIL_OP_INVERT,
	D3D11_STENCIL_OP_INCR,
	D3D11_STENCIL_OP_DECR,
};

/////////////////////////////////

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

void Material::SetBlendState(const BlendState & blendState)
{
	m_blendState = blendState;
}

void Material::SetDepthTest(bool yes)
{
	m_depthStencilState.depthTest = yes;
}

void Material::SetDepthWrite(bool yes)
{
	m_depthStencilState.depthWrite = yes;
}

void Material::SetStencilTest(bool yes)
{
	m_depthStencilState.stencilTest = yes;
}

void Material::SetStencilMask(unsigned char readMask, unsigned char writeMask, unsigned char ref)
{
	m_depthStencilState.stencilReadMask = readMask;
	m_depthStencilState.stencilWriteMask = writeMask;
	m_depthStencilState.stencilRef = ref;
}

void Material::SetStencilFrontFace(StencilOp fail, StencilOp zfail, StencilOp pass, ComparsionFunc func)
{
	m_depthStencilState.frontFaceStencilFailOp = fail;
	m_depthStencilState.frontFaceStencilDepthFailOp = zfail;
	m_depthStencilState.frontFaceStencilPassOp = pass;
	m_depthStencilState.frontFaceStencilFunc = func;
}

void Material::SetStencilBackFace(StencilOp fail, StencilOp zfail, StencilOp pass, ComparsionFunc func)
{
	m_depthStencilState.backFaceStencilFailOp = fail;
	m_depthStencilState.backFaceStencilDepthFailOp = zfail;
	m_depthStencilState.backFaceStencilPassOp = pass;
	m_depthStencilState.backFaceStencilFunc = func;
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

		// bind blend state
		{
			auto blendStateIt = std::find_if(s_blendStatesCache.begin(), s_blendStatesCache.end(), [this](auto&& val)
			{
				return val.first == m_blendState;
			});

			Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
			if (blendStateIt == s_blendStatesCache.end())
			{
				CD3D11_BLEND_DESC blendDesc{ D3D11_DEFAULT };

				const D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc =
				{
					m_blendState.enabled,

					g_blendTable[static_cast<unsigned>(m_blendState.srcColor)],
					g_blendTable[static_cast<unsigned>(m_blendState.dstColor)],
					g_blendOpTable[static_cast<unsigned>(m_blendState.op)],

					g_blendTable[static_cast<unsigned>(m_blendState.srcAlpha)],
					g_blendTable[static_cast<unsigned>(m_blendState.dstAlpha)],
					g_blendOpTable[static_cast<unsigned>(m_blendState.op)],

					D3D11_COLOR_WRITE_ENABLE_ALL,
				};

				for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
					blendDesc.RenderTarget[i] = renderTargetBlendDesc;

				result = device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
				assert(SUCCEEDED(result));

				s_blendStatesCache.emplace_back(m_blendState, blendState);
			}
			else
			{
				blendState = blendStateIt->second;
			}

			context->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);
		}

		// bind depth stencil state
		{
			auto depthStencilStateIt = std::find_if(s_depthStencilStatesCache.begin(), s_depthStencilStatesCache.end(), [this](auto&& val)
			{
				return val.first == m_depthStencilState;
			});

			Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
			if (depthStencilStateIt == s_depthStencilStatesCache.end())
			{
				D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

				// Initialize the description of the stencil state.
				ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

				// Set up the description of the stencil state.
				depthStencilDesc.DepthEnable = m_depthStencilState.depthTest;
				depthStencilDesc.DepthWriteMask = m_depthStencilState.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
				depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

				depthStencilDesc.StencilEnable = m_depthStencilState.stencilTest;
				depthStencilDesc.StencilReadMask = m_depthStencilState.stencilReadMask;
				depthStencilDesc.StencilWriteMask = m_depthStencilState.stencilWriteMask;

				// Stencil operations if pixel is front-facing.
				depthStencilDesc.FrontFace.StencilFailOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.frontFaceStencilFailOp)];
				depthStencilDesc.FrontFace.StencilDepthFailOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.frontFaceStencilDepthFailOp)];
				depthStencilDesc.FrontFace.StencilPassOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.frontFaceStencilPassOp)];

				depthStencilDesc.FrontFace.StencilFunc = g_comparsionFuncTable[static_cast<unsigned>(m_depthStencilState.frontFaceStencilFunc)];

				// Stencil operations if pixel is back-facing.
				depthStencilDesc.BackFace.StencilFailOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.backFaceStencilFailOp)];
				depthStencilDesc.BackFace.StencilDepthFailOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.backFaceStencilDepthFailOp)];
				depthStencilDesc.BackFace.StencilPassOp = g_stencilOpTable[static_cast<unsigned>(m_depthStencilState.backFaceStencilPassOp)];

				depthStencilDesc.BackFace.StencilFunc = g_comparsionFuncTable[static_cast<unsigned>(m_depthStencilState.backFaceStencilFunc)];

				// Create the depth stencil state.
				result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
				assert(SUCCEEDED(result));

				s_depthStencilStatesCache.emplace_back(m_depthStencilState, depthStencilState);
			}
			else
			{
				depthStencilState = depthStencilStateIt->second;
			}

			context->OMSetDepthStencilState(depthStencilState.Get(), m_depthStencilState.stencilRef);
		}
	}
}
