#include "Chokbar.h"

#include "Engine/ECS/Components/Component.h"
#include "D3D/Shaders/Material.h"
#include "D3D/Shaders/ShaderBase.h"

#include "MeshRenderer.h"

using namespace DirectX;

MeshRenderer::MeshRenderer() : IRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::Render(ID3D12GraphicsCommandList* cmdList)
{
	if (!IsEnabled() || !Mat || !Mesh) return;

	auto shader = Mat->GetShader();
	shader->BeginDraw(cmdList);

	shader->Draw(cmdList, this);

	shader->EndDraw(cmdList);

}

void MeshRenderer::Update(float dt)
{
	if (!IsEnabled() || !Mat || !Mesh) return;

	if (transform->IsDirty())
		transform->UpdateParentedWorldMatrix();

	Mat->GetShader()->UpdateObjectCB(transform->GetParentedWorldMatrix(), ObjectCBIndex);
}

