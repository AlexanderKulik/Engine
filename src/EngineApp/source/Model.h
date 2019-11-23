#pragma once

#include "Math/AABB.h"

class Texture;
class Shader;
class Frustum;

class Model
{
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Quaternion = DirectX::SimpleMath::Quaternion;

public:
	struct Mesh
	{
		std::string								name;

		Microsoft::WRL::ComPtr<ID3D11Buffer>	vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	indexBuffer;

		unsigned int							vertexCount;
		unsigned int							indexCount;

		std::shared_ptr<Texture>				diffuse;
		Shader*									material{ nullptr };

		AABB									aabb;
	};

public:
	Model(ID3D11Device* dev, const std::string& path);

	void								Render(ID3D11DeviceContext* context, const Frustum& frustum);
	void								SetMaterial(size_t idx, Shader* shader);
	void								SetAllMaterials(Shader* shader);

	DirectX::XMMATRIX					GetTransform() const;
	size_t								GetMeshCount() const;
	const Mesh&							GetMesh(size_t idx) const;

	const Vector3&						GetPosition() const;
	const Quaternion&					GetRotation() const;
	const Vector3&						GetScale() const;

	void								SetPosition(const Vector3& position);
	void								SetRotation(const Quaternion& rotation);
	void								SetScale(const Vector3& scale);

private:
	Vector3								m_position;
	Quaternion							m_rotation;
	Vector3								m_scale;
	std::vector<std::unique_ptr<Mesh>>	m_meshes;
};

inline size_t Model::GetMeshCount() const
{
	return m_meshes.size();
}

inline const Model::Mesh& Model::GetMesh(size_t idx) const
{
	assert(idx < m_meshes.size());
	return *m_meshes[idx];
}

inline const DirectX::SimpleMath::Vector3& Model::GetPosition() const
{
	return m_position;
}

inline const DirectX::SimpleMath::Quaternion& Model::GetRotation() const
{
	return m_rotation;
}

inline const DirectX::SimpleMath::Vector3& Model::GetScale() const
{
	return m_scale;
}

inline void Model::SetPosition(const DirectX::SimpleMath::Vector3& position)
{
	m_position = position;
}

inline void Model::SetRotation(const DirectX::SimpleMath::Quaternion& rotation)
{
	m_rotation = rotation;
}

inline void Model::SetScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_scale = scale;
}
