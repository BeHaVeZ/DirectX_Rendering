#pragma once

#include "Effect.h"
#include "Math.h"
#include "Vector3.h"
#include "DataTypes.h"

class Effect;
class Matrix;
class Texture;


//struct Vertex_PosCol final
//{
//	Vector3 position;
//	Vector2 uv;
//};


class Mesh final
{

public:
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh(Mesh&& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;
	~Mesh();

	void Render(ID3D11DeviceContext* pDeviceContext) const;

	//void SetWorldViewProjectionMatrix(const dae::Matrix& matrix);

	void SetFilterTechnique(Effect::FilterMode mode);

	/// <summary>
	/// To rotate around the X-axis, you'd call Rotate(Vector3::UnitX, angle).
	/// For the Y - axis, it'd be Rotate(Vector3::UnitY, angle).
	/// And for the Z - axis, it'd be Rotate(Vector3::UnitZ, angle).
	/// </summary>
	/// <param name="Vector3:">X,Y,Z</param>
	/// <param name="angle:">angle which the mesh is rotated by</param>
	void Rotate(const Vector3& axis,float angle);
	void UpdateViewMatrices(const Matrix& viewProjectionMatrix, const Matrix& inverseViewMatrix);
private:

	std::unique_ptr<Effect> m_pEffect{};
	std::unique_ptr<Texture> m_pDiffuseTexture{};
	std::unique_ptr<Texture> m_pNormalTexture{};
	std::unique_ptr<Texture> m_pSpecularTexture{};
	std::unique_ptr<Texture> m_pGlossinessTexture{};

	ID3D11InputLayout* m_pInputLayout{};

	uint32_t m_NumIndices{};

	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};

	Matrix m_TranslationMatrix{ Vector3::UnitX,Vector3::UnitY,Vector3::UnitZ,Vector3::Zero };
	Matrix m_RotationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_ScaleMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
};