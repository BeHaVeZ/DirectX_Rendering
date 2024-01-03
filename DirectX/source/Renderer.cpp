#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Utils.h"


Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}
	m_Camera.Initialize(45.f, { 0.f,0.f,-132.827f }, static_cast<float>(m_Width) / m_Height);
	// Create some date for our mesh
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	Utils::ParseOBJ("Resources/CS_AK.obj", vertices, indices);
	m_pMesh = new Mesh{ m_pDevice, vertices, indices };
}

Renderer::~Renderer()
{
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
		delete m_pMesh;
	}
}

void Renderer::Update(const Timer* pTimer)
{
	m_Camera.Update(pTimer);

	if (!m_DisableMeshRotation) // Check if mesh rotation is enabled
	{
		m_pMesh->Rotate(Vector3::UnitY, m_RotationSpeed * TO_RADIANS * pTimer->GetElapsed());
	}
	m_pMesh->UpdateViewMatrices(m_Camera.GetWorldViewProjection(), m_Camera.GetInvMatrix());

	HandleFilterModeChange();
	HandleInspectModeToggle();
	HandleMeshRotationToggle();

	if (m_InspectMode == false)
	{
		return;
	}
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	RotateObjectWithMouse(mouseX, mouseY, m_RotationSpeed * TO_RADIANS * pTimer->GetElapsed());
}


void Renderer::Render() const
{
	if (!m_IsInitialized)
		return;

	//1. clear RTV and DSV
	constexpr float color[4] = { 0.1f,0.1f,0.1f,1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

	//2. Set pipeline + invoke draw call

	m_pMesh->Render(m_pDeviceContext);

	//3. present backbuffer (swap)
	m_pSwapChain->Present(0, 0);

}

HRESULT Renderer::InitializeDirectX()
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) or defined(_DEBUG)
	createDeviceFlags != D3D11_CREATE_DEVICE_DEBUG;
#endif // defined(DEBUG) or defined(_DEBUG)

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

	if (FAILED(result))
		return result;

	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(result))
		return result;

	//create swapchain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.BufferDesc.Width = m_Width;
	swap_chain_desc.BufferDesc.Height = m_Height;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 1;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 144;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;


	// Get the handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sys_wm_info{};
	SDL_VERSION(&sys_wm_info.version);
	SDL_GetWindowWMInfo(m_pWindow, &sys_wm_info);
	swap_chain_desc.OutputWindow = sys_wm_info.info.win.window;

	// Create SwapChain
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swap_chain_desc, &m_pSwapChain);
	if (FAILED(result))
		return result;


	//depthbuffer
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;



	//view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;


	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;



	// 4. Create RenderTarget (RT) & RenderTargetView (RTV)

	// Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	// View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;


	// 5. Bind RTV & DSV to Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 6. Set the viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);
	return result;


	return S_FALSE;
}

void Renderer::HandleFilterModeChange()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	static int currentTechniqueIndex = 0;
	static bool prevF2State = false;

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		if (!prevF2State)
		{
			currentTechniqueIndex = (currentTechniqueIndex + 1) % 3;
			switch (currentTechniqueIndex)
			{
			case 0:
				m_pMesh->SetFilterTechnique(Effect::FilterMode::Point);
				std::wcout << "POINT MODE:\n";
				break;
			case 1:
				m_pMesh->SetFilterTechnique(Effect::FilterMode::Linear);
				std::wcout << "LINEAR MODE:\n";
				break;
			case 2:
				m_pMesh->SetFilterTechnique(Effect::FilterMode::Anisotropic);
				std::wcout << "ANISOTROPIC MODE:\n";
				break;
			}
		}
		prevF2State = true;
	}
	else
	{
		prevF2State = false;
	}
}

void Renderer::HandleInspectModeToggle()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	static bool prevF4State = false;

	m_InspectMode = !m_InspectMode;
	if (pKeyboardState[SDL_SCANCODE_F4])
	{
		if (!prevF4State)
		{
			m_Camera.SetInspectMode();
			m_DisableMeshRotation = !m_DisableMeshRotation;
		}
		prevF4State = true;
	}
	else
	{
		prevF4State = false;
	}
}
void Renderer::HandleMeshRotationToggle()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	static bool prevF5State = false;

	if (pKeyboardState[SDL_SCANCODE_F5])
	{
		if (!prevF5State)
		{
			m_DisableMeshRotation = !m_DisableMeshRotation; // Toggle mesh rotation state
		}
		prevF5State = true;
	}
	else
	{
		prevF5State = false;
	}
}

void Renderer::RotateObjectWithMouse(int mouseX, int mouseY, float rotationSpeed)
{
	// Rotate the object based on mouse movement
	// Implement your object rotation logic here using the mouseX and mouseY values
	// This could involve adjusting the rotation of the rendered model
	// Example: m_pMesh->RotateObject(mouseX, mouseY, rotationSpeed);

	static int prevMouseX = mouseX;
	static int prevMouseY = mouseY;

	if (leftMouseButtonHeld)
	{
		int deltaX = mouseX - prevMouseX;
		int deltaY = mouseY - prevMouseY;

		m_pMesh->Rotate(Vector3((float)deltaY, (float)deltaX, 0.0f), rotationSpeed);

		prevMouseX = mouseX;
		prevMouseY = mouseY;
	}

}
