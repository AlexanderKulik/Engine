#include "pch.h"

#include "Model.h"
#include "Texture.h"
#include "Frustum.h"

#pragma warning( push )
#pragma warning( disable : 4244 )
#include "OBJ_Loader.h"
#pragma warning( pop )

#include <boost/container_hash/hash.hpp>

unsigned GetSizeFromInputType(InputType inputType)
{
	switch (inputType)
	{
	case InputType::R32G32B32A32_FLOAT:
		return sizeof(float) * 4;

	case InputType::R32G32B32_FLOAT:
		return sizeof(float) * 3;

	case InputType::R32G32_FLOAT:
		return sizeof(float) * 2;

	default:
		assert(false);
		return 0;
	}
}

Model::Model(ID3D11Device* dev, const std::string& path)
	: m_position(DirectX::XMVectorZero())
	, m_rotation(DirectX::XMQuaternionIdentity())
	, m_scale(DirectX::XMVectorSet(1, 1, 1, 1))
{
	objl::Loader loader;

	bool loadout = loader.LoadFile(path);
	if (loadout)
	{
		for (auto&& meshData : loader.LoadedMeshes)
		{
			auto&& mesh = std::make_unique<Mesh>();
			mesh->name = meshData.MeshName;
			
			AABB meshAabb;
			for (auto&& data : meshData.Vertices)
			{
				meshAabb.AddPoint(DirectX::SimpleMath::Vector3{ &data.Position.X });
			}
			mesh->localAabb = meshAabb;

			mesh->vertexBufferDesc.AddInput(InputSemantic::POSITION, InputType::R32G32B32_FLOAT);
			mesh->vertexBufferDesc.AddInput(InputSemantic::NORMAL, InputType::R32G32B32_FLOAT);
			mesh->vertexBufferDesc.AddInput(InputSemantic::TEXCOORD0, InputType::R32G32_FLOAT);

			// create the vertex buffer
			{
				D3D11_BUFFER_DESC vertexBufferDesc;
				ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

				vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(objl::Vertex) * meshData.Vertices.size());
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexData;
				vertexData.pSysMem = meshData.Vertices.data();
				vertexData.SysMemPitch = 0;
				vertexData.SysMemSlicePitch = 0;

				auto&& result = dev->CreateBuffer(&vertexBufferDesc, &vertexData, mesh->vertexBuffer.GetAddressOf());
				assert(SUCCEEDED(result));

				mesh->vertexCount = static_cast<unsigned>(meshData.Vertices.size());
			}


			// create index buffer
			{
				D3D11_BUFFER_DESC indexBufferDesc;
				ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = static_cast<UINT>(meshData.Indices.size() * sizeof(unsigned int));
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA indexData;
				indexData.pSysMem = meshData.Indices.data();
				indexData.SysMemPitch = 0;
				indexData.SysMemSlicePitch = 0;

				auto&& result = dev->CreateBuffer(&indexBufferDesc, &indexData, mesh->indexBuffer.GetAddressOf());
				assert(SUCCEEDED(result));

				mesh->indexCount = static_cast<unsigned>(meshData.Indices.size());
			}

			auto&& diffuse = meshData.MeshMaterial.map_Kd;
			if (!diffuse.empty())
			{
				mesh->diffuse = Texture::CreateTexture(dev, std::wstring(diffuse.begin(), diffuse.end()));
			}
			else
			{
				mesh->diffuse = nullptr;
			}

			m_meshes.push_back(std::move(mesh));
		}
	}
}

void Model::Render(ID3D11Device* dev, ID3D11DeviceContext* context, const Frustum& frustum, Shader* shader)
{
	for (auto&& mesh : m_meshes)
	{
		auto&& cullStatus = frustum.CullAABB(mesh->worldAabb);
		if (cullStatus == CullResult::OUTSIDE)
		{
			continue;
		}

		if (mesh->diffuse)
		{
			mesh->diffuse->Bind(context);
		}

		context->IASetInputLayout( shader->RequestInputLayout(dev, mesh->vertexBufferDesc).Get() );

		// select which vertex buffer to display
		UINT stride = mesh->vertexBufferDesc.stride;
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// select which primtive type we are using
		context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// draw the vertex buffer to the back buffer
		context->DrawIndexed(mesh->indexCount, 0, 0);
	}
}

void Model::SetMaterial(size_t idx, Shader* shader)
{
	assert(idx < m_meshes.size());
	m_meshes[idx]->material = shader;
}

void Model::SetAllMaterials(Shader* shader)
{
	for (size_t idx = 0, sz = m_meshes.size(); idx < sz; idx++)
	{
		SetMaterial(idx, shader);
	}
}

void Model::UpdateBoundingVolumes()
{
	auto&& transform = GetTransform();
	for (auto&& mesh : m_meshes)
	{
		std::array<Vector3, 8> localAabbPoints;
		mesh->localAabb.GetPoints(localAabbPoints.data());

		mesh->worldAabb = AABB();
		for (auto&& p : localAabbPoints)
		{
			mesh->worldAabb.AddPoint(Vector3::Transform(p, transform));
		}
	}
}

Model::Matrix Model::GetTransform() const
{
	return DirectX::XMMatrixAffineTransformation(m_scale, DirectX::XMVectorZero(), m_rotation, m_position);
}

void VertexBufferDesc::AddInput(InputSemantic semantic, InputType type)
{
	VertexBufferInput input;
	input.semantic = semantic;
	input.type = type;
	input.offset = stride;
	inputs.push_back(input);

	stride += GetSizeFromInputType(type);

	boost::hash_combine(hash, semantic);
	boost::hash_combine(hash, type);
}