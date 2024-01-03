#pragma once
#include "Camera.h"


struct SDL_Window;
struct SDL_Surface;
class Mesh;

class Renderer final
{
public:
	Renderer(SDL_Window* pWindow);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	void Update(const Timer* pTimer);
	void Render() const;

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};

	bool m_IsInitialized{ false };
	bool leftMouseButtonHeld = false;


	//DIRECTX
	HRESULT InitializeDirectX();
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;

	Camera m_Camera;
	Mesh* m_pMesh;

	//...
	bool m_DisableMeshRotation{ false };
	bool m_InspectMode{ false };
	const float m_RotationSpeed{ 45.f };


	void HandleFilterModeChange();
	void HandleInspectModeToggle();
	void HandleMeshRotationToggle();
	void RotateObjectWithMouse(int mouseX, int mouseY, float rotationSpeed);


};