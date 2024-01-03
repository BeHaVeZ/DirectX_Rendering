#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: m_pEffect{ std::make_unique<Effect>(pDevice, L"Resources/PosCol3D.fx") },
	m_pDiffuseTexture{ std::make_unique<Texture>(pDevice,"Resources/ak47_default.png") },
	m_pNormalTexture{ std::make_unique<Texture>(pDevice,"Resources/ak47_default_normal.png") },
	m_pSpecularTexture{ std::make_unique<Texture>(pDevice,"Resources/ak47_default_specular.png") },
	m_pGlossinessTexture{ std::make_unique<Texture>(pDevice,"Resources/ak47_default_gloss.png") }
{
	// Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//vertexDesc[1].SemanticName = "COLOR";
	//vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result{ pDevice->CreateInputLayout
		(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout
		) };
	if (FAILED(result)) return;

	// Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result)) return;

	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) return;



	// Textures
	m_pEffect->SetDiffuseMap(m_pDiffuseTexture.get());
	m_pEffect->SetNormalMap(m_pNormalTexture.get());
	m_pEffect->SetSpecularMap(m_pSpecularTexture.get());
	m_pEffect->SetGlossingessMap(m_pGlossinessTexture.get());

}

Mesh::~Mesh()
{
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();

	if (m_pInputLayout) m_pInputLayout->Release();
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
	// 1. Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. Set input layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// 3. Set vertex buffer
	constexpr UINT stride{ sizeof(Vertex) };
	constexpr UINT offset{};
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 4. Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 5. Draw
	D3DX11_TECHNIQUE_DESC techniqueDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techniqueDesc);
	for (UINT p{}; p < techniqueDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

//void Mesh::SetWorldViewProjectionMatrix(const dae::Matrix& matrix) 
//{
//	if (m_pEffect)
//		m_pEffect->SetWorldViewProjectionMatrix(matrix);
//}

void Mesh::SetFilterTechnique(Effect::FilterMode mode)
{
	m_pEffect->SetFilterMode(mode);
}
void Mesh::Rotate(const Vector3& axis, float angle)
{
	Matrix rotationMatrix;

	if (axis == Vector3::UnitX)
		rotationMatrix = Matrix::CreateRotationX(angle);
	else if (axis == Vector3::UnitY)
		rotationMatrix = Matrix::CreateRotationY(angle);
	else if (axis == Vector3::UnitZ)
		rotationMatrix = Matrix::CreateRotationZ(angle);

	m_RotationMatrix = rotationMatrix * m_RotationMatrix;
}

void Mesh::UpdateViewMatrices(const Matrix& viewProjectionMatrix, const Matrix& inverseViewMatrix)
{
	Matrix world{ m_ScaleMatrix * m_RotationMatrix * m_TranslationMatrix };
	m_pEffect->SetWorldViewProjectionMatrix(world * viewProjectionMatrix);
	m_pEffect->SetInvViewMatrix(inverseViewMatrix);
	m_pEffect->SetWorldMatrix(world);
}
