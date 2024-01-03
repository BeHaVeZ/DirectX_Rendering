#include "pch.h"
#include "Effect.h"
#include "Texture.h"



Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: m_pEffect{ LoadEffect(pDevice, assetFile) }
{
	m_pTechnique = m_pEffect->GetTechniqueByName("PointFilterTechnique");
	if (!m_pTechnique->IsValid()) 
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pShaderResourceVariable not valid!\n";
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable not valid!\n";
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable not valid!\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable not valid!\n";
	}

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
	}

	m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
	if (!m_pViewInverseVariable->IsValid())
	{
		std::wcout << L"m_pViewInverseVariable not valid!\n";
	}

	m_pWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pWorldVariable->IsValid())
	{
		std::wcout << L"m_pWorldVariable not valid!\n";
	}

}

Effect::~Effect()
{
	if (m_pEffect) m_pEffect->Release();
}

ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	return m_pTechnique;
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile
	(
		assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob
	);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			std::wstringstream ss;
			for (unsigned int i{}; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << "\n";
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << "\n";
			return nullptr;
		}
	}

	return pEffect;
}

void Effect::SetWorldViewProjectionMatrix(const Matrix& worldViewProj)
{
	m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
}

void Effect::SetWorldMatrix(const Matrix& worldMatrix)
{
	m_pWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void Effect::SetInvViewMatrix(const Matrix& invMatrix)
{
	m_pWorldVariable->SetMatrix(reinterpret_cast<const float*>(&invMatrix));
}

void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
	}
}

void Effect::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pNormalTexture->GetShaderResourceView());
	}
}

void Effect::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetShaderResourceView());
	}
}

void Effect::SetGlossingessMap(Texture* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetShaderResourceView());
	}
}

void Effect::SetFilterMode(FilterMode mode)
{
	switch (mode)
	{
	case Effect::Point:
		m_pTechnique = m_pEffect->GetTechniqueByName("PointFilterTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"PointTechnique not valid\n";
		break;
	case Effect::Linear:
		m_pTechnique = m_pEffect->GetTechniqueByName("LinearFilterTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"LinearTechnique not valid\n";
		break;
	case Effect::Anisotropic:
		m_pTechnique = m_pEffect->GetTechniqueByName("AnisotropicFilterTechnique");
		if (!m_pTechnique->IsValid()) std::wcout << L"AnisotropicTechnique not valid\n";
		break;
	}

}
