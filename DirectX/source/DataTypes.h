#pragma once
#include "Math.h"
#include "vector"


struct Vertex
{
	Vector3 position{};
	///ColorRGB color{colors::White};
	Vector2 uv{}; //W2
	Vector3 normal{}; //W4
	Vector3 tangent{}; //W4
	///Vector3 viewDirection{}; //W4
};

struct Vertex_Out
{
	Vector4 position{};
	ColorRGB color{ colors::White };
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector3 viewDirection{};
};

enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};
