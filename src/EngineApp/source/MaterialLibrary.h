#pragma once

#include "Material.h"

struct MaterialHandle
{
	static const unsigned Invalid = (unsigned)-1;

	unsigned idx;
};

class MaterialLibrary
{
	static const unsigned MAX_MATERIALS = 512;

public:
	MaterialHandle Create(const std::string& name);
	MaterialHandle Clone(MaterialHandle handle);
	MaterialHandle FindByName(const std::string& name) const;
	void Destroy(MaterialHandle handle);
	void DestroyAll();

private:
	Material m_materials[MAX_MATERIALS];
	std::unordered_map<std::string, MaterialHandle> m_nameToHandle;
};