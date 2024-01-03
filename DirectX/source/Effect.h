#pragma once
#include "Texture.h"

class Matrix;
class Texture;

class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	Effect(const Effect& other) = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect(Effect&& other) = delete;
	Effect& operator=(Effect&& other) = delete;
	~Effect();
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	void SetWorldViewProjectionMatrix(const Matrix& worldViewProj);
	void SetWorldMatrix(const Matrix& worldMatrix);
	void SetInvViewMatrix(const Matrix& invMatrix);
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	void SetDiffuseMap(Texture* pDiffuseTexture);
	void SetNormalMap(Texture* pNormalTexture);
	void SetSpecularMap(Texture* pSpecularTexture);
	void SetGlossingessMap(Texture* pGlossinessTexture);



	enum FilterMode {
		Point,Linear,Anisotropic
	};
	FilterMode m_CurrentFilterMode;
	void SetFilterMode(FilterMode mode);

private:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// D3DX11MatrixVariables
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
	ID3DX11EffectMatrixVariable* m_pViewInverseVariable{};
	ID3DX11EffectMatrixVariable* m_pWorldVariable{};
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// D3DX11ResourceVariables
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};