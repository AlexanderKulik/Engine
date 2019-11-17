#include "pch.h"

#include "Model.h"
#include "Texture.h"

#pragma warning( push )
#pragma warning( disable : 4244 )
#include "OBJ_Loader.h"
#pragma warning( pop ) 

Model::Model(ID3D11Device* dev, const std::string& path)
{
	objl::Loader loader;

	bool loadout = loader.LoadFile(path);
	if (loadout)
	{
		for (auto&& meshData : loader.LoadedMeshes)
		{
			auto&& mesh = std::make_unique<Mesh>();
			mesh->name = meshData.MeshName;

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

void Model::Render(ID3D11DeviceContext* context)
{
	for (auto&& mesh : m_meshes)
	{
		if (mesh->diffuse)
		{
			mesh->diffuse->Bind(context);
		}

		// select which vertex buffer to display
		UINT stride = sizeof(objl::Vertex);
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