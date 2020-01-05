#include "pch.h"
#include "MaterialLibrary.h"

MaterialHandle MaterialLibrary::Create(const std::string & name)
{
	return MaterialHandle{};
}

MaterialHandle MaterialLibrary::FindByName(const std::string & name) const
{
	return MaterialHandle{};
}

void MaterialLibrary::Destroy(MaterialHandle handle)
{

}

void MaterialLibrary::DestroyAll()
{

}