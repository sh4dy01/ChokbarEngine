#pragma once 

class CHOKBAR_API GeometryGenerator
{
	public:
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const XMFLOAT3& position,
			const XMFLOAT3& normal,
			const XMFLOAT3& tangent,
			const XMFLOAT2& uv
		);
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v
		);
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT> Indices32;

		std::vector<UINT16>& GetIndices16();

	private:
		std::vector<UINT16> m_Indices16;
	};

	MeshData CreatePyramid(float width, float height, float depth);
};

struct CHOKBAR_API Transform
{
	XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 Scale = { 1.0f, 1.0f, 1.0f };
};