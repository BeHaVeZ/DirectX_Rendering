#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include <memory>
#include "Vector3.h"


struct Vector2;

class Texture
{
public:
	//Texture(SDL_Surface* pSurface);

	///Just for my reference (different textures directX) below parameters
	//! https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-type
	Texture(ID3D11Device* pDevice, const std::string& path);
	~Texture();

	//static std::unique_ptr<Texture> LoadFromFile(const std::string& path);
	//dae::ColorRGB Sample(const Vector2& uv) const;
	//dae::Vector3 SampleNormal(const Vector2& uv) const;


	ID3D11Texture2D* GetResource() const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;
private:

	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };


	ID3D11Texture2D* m_pResource{};
	ID3D11ShaderResourceView* m_pShaderResourceView{};
};
