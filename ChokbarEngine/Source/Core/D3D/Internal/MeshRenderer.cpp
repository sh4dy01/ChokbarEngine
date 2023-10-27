#include "Chokbar.h"

#include "Engine/ECS/Components/Component.h"
#include "D3DMesh.h"

#include "Material.h"
#include "ShaderBase.h"

#include "MeshRenderer.h"

MeshRenderer::MeshRenderer()
{
	Mesh = nullptr;
	Mat = nullptr;
}

MeshRenderer::~MeshRenderer()
{
	Mesh = nullptr;
	Mat = nullptr;
}

void MeshRenderer::BindMaterial(Material* mat)
{
	Mat = mat;
	ObjectCBIndex = Mat->GetShader()->Bind()->GetCreatedIndex();
}