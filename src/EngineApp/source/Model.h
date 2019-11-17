#pragma once

#include "Math/AABB.h"

class Texture;
class Shader;

class Model
{
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

	void								Render(ID3D11DeviceContext* context);
	void								SetMaterial(size_t idx, Shader* shader);
	void								SetAllMaterials(Shader* shader);

	size_t								GetMeshCount() const;

private:
	DirectX::XMVECTOR					m_position;
	DirectX::XMVECTOR					m_rotation;
	DirectX::XMVECTOR					m_scale;
	std::vector<std::unique_ptr<Mesh>>	m_meshes;
};

inline size_t Model::GetMeshCount() const
{
	return m_meshes.size();
}