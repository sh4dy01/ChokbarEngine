#pragma once 

#include "D3DMesh.h"

class GeometryHandler
{
public:

	static void CreateAllMeshes();
	static void DestroyAllMeshes();

	static D3DMesh* GetMesh(MeshType type) { return m_Meshes[type]; }

private:

	static D3DMesh* CreateCube();

	static D3DMesh* CreateUVSphere();
	static D3DMesh* CreateGeoSphere();
	static void Subdivide(std::vector<Vertex>& vertices, std::vector<UINT>& indices);
	static Vertex MidPoint(const Vertex& v0, const Vertex& v1);

	static D3DMesh* CreatePyramid();

	static D3DMesh* CreateSquare();

private:

	static D3DMesh* m_Meshes[4];
};