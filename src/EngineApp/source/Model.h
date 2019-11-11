#pragma once

#include <memory>
#include <vector>
#include <string>
#include <wrl.h>
#include <DirectXMath.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
class Texture;

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
	};

public:
	Model(ID3D11Device* dev, const std::string& path);

	void								Render(ID3D11DeviceContext* context);

private:
	DirectX::XMVECTOR					m_position;
	DirectX::XMVECTOR					m_rotation;
	DirectX::XMVECTOR					m_scale;
	std::vector<std::unique_ptr<Mesh>>	m_meshes;
};