#include "Chokbar.h"

#include "Engine/Resource.h"
#include "Engine/ECS/Components/Component.h"
#include "D3DMesh.h"
#include "../D3DMath.h"

#include "Material.h"
#include "ShaderBase.h"
#include "Texture.h"

#include "MeshRenderer.h"


MeshRenderer::MeshRenderer()
{
	Mesh = nullptr;
	Mat = nullptr;
}

MeshRenderer::MeshRenderer(MeshType meshType, MaterialType matType)
{
	Mesh = GeometryHandler::GetMesh(meshType);

	Material* mat = Resource::LoadMaterial(matType);
	BindMaterial(mat);
}

MeshRenderer::~MeshRenderer()
{
	OnDelete();

	Mesh = nullptr;
	Mat = nullptr;
	m_textures.clear();
}

void MeshRenderer::OnAddedComponent()
{
	Bounds.Center = transform->GetPosition();
	Bounds.Radius = transform->GetScale().x;
}

void MeshRenderer::OnDelete()
{
	Mat->GetShader()->UnBind(ObjectCBIndex);
}

void MeshRenderer::RegisterTexture(Texture* tex)
{
	m_textures.push_back(tex);
	TextSrvIndex = I(D3DApp)->UpdateTextureHeap(tex);
}

void MeshRenderer::BindMaterial(Material* mat)
{
	Mat = mat;
	ObjectCBIndex = Mat->GetShader()->Bind()->GetCreatedIndex();
}