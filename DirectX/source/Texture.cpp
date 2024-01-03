#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <algorithm>


    Texture::Texture(ID3D11Device* pDevice, const std::string& path)
    {
        //Do this resource/resource view creation when you load the texture, so only once!
        SDL_Surface* pSurface = IMG_Load(path.c_str());

        //!Texture
        const DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = pSurface->w;
        desc.Height = pSurface->h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        //!Init SDL_Surface
        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = pSurface->pixels;
        initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
        initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

        HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

        //!ShaderResourceView
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
        SRVDesc.Format = format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;

        hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);

        //After pushing the data from the SDL_Surface to the resource, the SDL_Surface is no longer needed in memory, thus free it using SDL_FreeSurface.
        SDL_FreeSurface(pSurface);
    }

    Texture::~Texture()
    {
        if (m_pSurface)
        {
            SDL_FreeSurface(m_pSurface);
            m_pSurface = nullptr;
        }
    }

    //std::unique_ptr<Texture> Texture::LoadFromFile(const std::string& path)
    //{
    //    SDL_Surface* pLoadedSurface = IMG_Load(path.c_str());
    //    if (!pLoadedSurface)
    //    {
    //        return nullptr;
    //    }
    //    return std::make_unique<Texture>(pLoadedSurface);
    //}

    //ColorRGB Texture::Sample(const Vector2& uv) const
    //{
    //    if (!m_pSurface or !m_pSurfacePixels)
    //    {
    //        return ColorRGB{};
    //    }
    //    const float clampedU = std::clamp(uv.x, 0.f, 1.f);
    //    const float clampedV = std::clamp(uv.y, 0.f, 1.f);
    //    const Uint32 px{ static_cast<Uint32>(clampedU * m_pSurface->w) };
    //    const Uint32 py{ static_cast<Uint32>(clampedV * m_pSurface->h) };
    //    const Uint32 pixelIndex{ static_cast<Uint32>(px + (py * m_pSurface->w)) };

    //    Uint8 r, g, b;
    //    SDL_GetRGB(m_pSurfacePixels[pixelIndex], m_pSurface->format, &r, &g, &b);

    //    const float invResize = 1 / 255.f;
    //    return ColorRGB{ r * invResize , g * invResize, b * invResize };
    //}

    //Vector3 Texture::SampleNormal(const Vector2& uv) const
    //{
    //    ColorRGB sampleColor = Sample(uv);

    //    Vector3 normal{};
    //    normal.x = 2 * sampleColor.r - 1;
    //    normal.y = 2 * sampleColor.g - 1;
    //    normal.z = 2 * sampleColor.b - 1;

    //    return normal.Normalized();
    //}
    ID3D11Texture2D* Texture::GetResource() const
    {
        return m_pResource;
    }
    ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
    {
        return m_pShaderResourceView;
    }