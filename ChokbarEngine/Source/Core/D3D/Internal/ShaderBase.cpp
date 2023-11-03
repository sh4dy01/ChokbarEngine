#include "Chokbar.h"

#include "Core/DebugUtils.h"
#include "D3DMesh.h"
#include "Texture.h"
#include "ParticleRenderer.h"
#include "Core/D3D/D3DUtils.h"

#include "ShaderBase.h"

using namespace DirectX;

#pragma region SHADER BASE
ShaderBase::ShaderBase(ID3D12Device* device, ID3D12DescriptorHeap* cbvHeap, UINT cbvDescriptorSize, std::wstring& filepath)
	: m_generalDevice(device), m_cbvDescriptorSize(cbvDescriptorSize), m_filepath(filepath)
{
	m_passCB = nullptr;

	m_rootSignature = nullptr;
	m_pipelineState = nullptr;

	m_vsByteCode = nullptr;
	m_psByteCode = nullptr;

	m_MainCamera = nullptr;
}

ShaderBase::~ShaderBase()
{
	m_generalDevice = nullptr;

	RELPTR(m_pipelineState);
	RELPTR(m_rootSignature);

	RELPTR(m_vsByteCode);
	RELPTR(m_psByteCode);

	m_objectCBs.clear();
	m_passCB = nullptr;

	m_MainCamera = nullptr;
}

void ShaderBase::Init()
{
	CreatePassCB();

	CompileShader(nullptr, "vs_main", "vs_5_1", &m_vsByteCode);
	CompileShader(nullptr, "ps_main", "ps_5_1", &m_psByteCode);

	m_MainCamera = Chokbar::Engine::GetMainCamera()->GetCameraComponent();
}

void ShaderBase::SetInputLayout(VertexType vertexType)
{
	m_inputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	m_inputLayout.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(XMFLOAT3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	m_inputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT4) + sizeof(XMFLOAT3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	m_inputLayout.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(XMFLOAT4) + sizeof(XMFLOAT3) * 2, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	m_inputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(XMFLOAT4) + sizeof(XMFLOAT3) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}

void ShaderBase::UnBind(UINT index)
{
	m_objectCBs.erase(m_objectCBs.begin() + index);
	m_freeIndices.push(index);
}

ShaderBase* ShaderBase::Bind()
{
	AddObjectCB();
	return this;
}

void ShaderBase::AddObjectCB() { m_objectCBs.emplace_back(new UploadBuffer<ObjConstants>(m_generalDevice, 1, true)); }

void ShaderBase::UpdateObjectCB(DirectX::XMFLOAT4X4* itemWorldMatrix, UINT cbIndex)
{
	if (cbIndex >= m_objectCBs.size())
		AddObjectCB();

	ObjConstants objConstants;
	objConstants.World = *itemWorldMatrix;
	m_objectCBs[cbIndex]->CopyData(0, &objConstants);
}

void ShaderBase::CreatePassCB() { m_passCB = new UploadBuffer<PassConstants>(m_generalDevice, 1, true); }

void ShaderBase::UpdatePassCB(const float dt, const float totalTime)
{
	const XMMATRIX camView = m_MainCamera->GetView();
	const XMMATRIX camProj = m_MainCamera->GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(camView, camProj);
	// XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camView), camView);
	// XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camProj), camProj);
	// XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	PassConstants mainPassCB;
	XMStoreFloat4x4(&mainPassCB.View, XMMatrixTranspose(camView));
	// XMStoreFloat4x4(&mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mainPassCB.Proj, XMMatrixTranspose(camProj));
	// XMStoreFloat4x4(&mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	// XMStoreFloat4x4(&mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	mainPassCB.EyePosW = m_MainCamera->transform->GetPosition();
	// mainPassCB.RenderTargetSize = XMFLOAT2(m_bufferWidth, m_bufferHeight);
	// mainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_bufferWidth, 1.0f / m_bufferHeight);
	// mainPassCB.NearZ = m_camera.NearZ;
	// mainPassCB.FarZ = m_camera.FarZ;
	mainPassCB.TotalTime = totalTime;
	mainPassCB.DeltaTime = dt;

	m_passCB->CopyData(0, &mainPassCB);
}

void ShaderBase::CompileShader(const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target, ID3DBlob** uploader)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ID3DBlob* errors = nullptr;

	hr = D3DCompileFromFile(m_filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, uploader, &errors);

	if (errors != nullptr)
	{
		MessageBoxA(0, (char*)errors->GetBufferPointer(), 0, 0);
		errors->Release();
	}
	ThrowIfFailed(hr);
}
#pragma endregion 

#pragma region SHADER SIMPLE
ShaderSimple::ShaderSimple(ID3D12Device* device, ID3D12DescriptorHeap* cbvHeap, UINT cbvDescriptorSize, std::wstring& filepath)
	: ShaderBase(device, cbvHeap, cbvDescriptorSize, filepath)
{
}

ShaderSimple::~ShaderSimple()
{
}

void ShaderSimple::Init()
{
	ShaderBase::Init();
}

void ShaderSimple::CreatePsoAndRootSignature(VertexType vertexType, DXGI_FORMAT& rtvFormat, DXGI_FORMAT& dsvFormat)
{
	SetInputLayout(vertexType);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(slotRootParameter), slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* serializedRootSignature = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &errorBlob);
	ThrowIfFailed(hr);
	m_generalDevice->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	ThrowIfFailed(hr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = { reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()), m_vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()), m_psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = dsvFormat;

	hr = m_generalDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	ThrowIfFailed(hr);

	RELPTR(serializedRootSignature);
	RELPTR(errorBlob);
}

void ShaderSimple::BeginDraw(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(m_rootSignature);

	cmdList->SetGraphicsRootConstantBufferView(1, m_passCB->GetResource()->GetGPUVirtualAddress());

	cmdList->SetPipelineState(m_pipelineState);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShaderSimple::Draw(MeshRenderer* drawnMeshR)
{
	ID3D12GraphicsCommandList* cmdList = I(D3DApp)->GetCommandList();
	if (drawnMeshR->ObjectCBIndex >= m_objectCBs.size())
		AddObjectCB();

	assert(drawnMeshR->ObjectCBIndex <= m_objectCBs.size());

	cmdList->IASetVertexBuffers(0, 1, &drawnMeshR->Mesh->VertexBufferView());
	cmdList->IASetIndexBuffer(&drawnMeshR->Mesh->IndexBufferView());

	cmdList->SetGraphicsRootConstantBufferView(0, m_objectCBs[drawnMeshR->ObjectCBIndex]->GetResource()->GetGPUVirtualAddress());

	cmdList->DrawIndexedInstanced(drawnMeshR->Mesh->GetIndexCount(), 1, 0, 0, 0);
}

void ShaderSimple::EndDraw(ID3D12GraphicsCommandList* cmdList)
{

}
#pragma endregion

#pragma region SHADER TEXTURE
ShaderTexture::ShaderTexture(ID3D12Device* device, ID3D12DescriptorHeap* cbvHeap, UINT cbvDescriptorSize, std::wstring& filepath)
	: ShaderBase(device, cbvHeap, cbvDescriptorSize, filepath)
{
}

ShaderTexture::~ShaderTexture()
{
}

void ShaderTexture::Init()
{
	ShaderBase::Init();
}

void ShaderTexture::CreatePsoAndRootSignature(VertexType vertexType, DXGI_FORMAT& rtvFormat, DXGI_FORMAT& dsvFormat)
{
	SetInputLayout(vertexType);

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);

	auto samplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(slotRootParameter), slotRootParameter, 6, samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* serializedRootSignature = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &errorBlob);
	ThrowIfFailed(hr);
	hr = m_generalDevice->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	ThrowIfFailed(hr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = { reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()), m_vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()), m_psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = dsvFormat;

	hr = m_generalDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	ThrowIfFailed(hr);

	RELPTR(serializedRootSignature);
	RELPTR(errorBlob);
}

void ShaderTexture::BeginDraw(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(m_rootSignature);

	cmdList->SetGraphicsRootConstantBufferView(2, m_passCB->GetResource()->GetGPUVirtualAddress());

	cmdList->SetPipelineState(m_pipelineState);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShaderTexture::Draw(MeshRenderer* drawnMeshR)
{
	assert(drawnMeshR->GetTexture(0)->HeapIndex >= 0);
	ID3D12GraphicsCommandList* cmdList = I(D3DApp)->GetCommandList();

	if (drawnMeshR->ObjectCBIndex >= m_objectCBs.size())
		AddObjectCB();

	assert(drawnMeshR->ObjectCBIndex <= m_objectCBs.size());

	cmdList->IASetVertexBuffers(0, 1, &drawnMeshR->Mesh->VertexBufferView());
	cmdList->IASetIndexBuffer(&drawnMeshR->Mesh->IndexBufferView());

	auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(I(D3DApp)->GetCbvHeap()->GetGPUDescriptorHandleForHeapStart());
	cbvHandle.Offset(drawnMeshR->GetTexture(0)->HeapIndex, m_cbvDescriptorSize);

	cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
	cmdList->SetGraphicsRootConstantBufferView(1, m_objectCBs[drawnMeshR->ObjectCBIndex]->GetResource()->GetGPUVirtualAddress());


	cmdList->DrawIndexedInstanced(drawnMeshR->Mesh->GetIndexCount(), 1, 0, 0, 0);
}

void ShaderTexture::EndDraw(ID3D12GraphicsCommandList* cmdList)
{
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> ShaderTexture::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
		0.0f, // mipLODBias
		8); // maxAnisotropy

	return { pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp };
}
#pragma endregion

#pragma region SHADER PARTICLE
ShaderParticle::ShaderParticle(ID3D12Device* device, ID3D12DescriptorHeap* cbvHeap, UINT cbvDescriptorSize, std::wstring& filepath)
	: ShaderBase(device, cbvHeap, cbvDescriptorSize, filepath)
{
}

ShaderParticle::~ShaderParticle()
{
}

void ShaderParticle::Init()
{
	ShaderBase::Init();

	m_particleInstanceDataBuffer = new UploadBuffer<InstanceData>(m_generalDevice, MAX_PARTICLE_COUNT, false);
}

void ShaderParticle::CreatePsoAndRootSignature(VertexType vertexType, DXGI_FORMAT& rtvFormat, DXGI_FORMAT& dsvFormat)
{
	SetInputLayout(vertexType);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(slotRootParameter), slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* serializedRootSignature = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &errorBlob);
	ThrowIfFailed(hr);
	m_generalDevice->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	ThrowIfFailed(hr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = { reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()), m_vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()), m_psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rtvFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = dsvFormat;

	hr = m_generalDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	ThrowIfFailed(hr);

	RELPTR(serializedRootSignature);
	RELPTR(errorBlob);
}

void ShaderParticle::BeginDraw(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(m_rootSignature);

	cmdList->SetGraphicsRootConstantBufferView(0, m_passCB->GetResource()->GetGPUVirtualAddress());

	cmdList->SetPipelineState(m_pipelineState);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShaderParticle::Draw(MeshRenderer* drawnMeshR)
{
	ParticleRenderer* pr = (ParticleRenderer*)drawnMeshR;
	assert(pr);
	DrawAsParticle(pr);
}

void ShaderParticle::DrawAsParticle(ParticleRenderer* drawnMeshR)
{
	ID3D12GraphicsCommandList* cmdList = I(D3DApp)->GetCommandList();
	if (drawnMeshR->ObjectCBIndex >= m_objectCBs.size())
		AddObjectCB();

	assert(drawnMeshR->ObjectCBIndex <= m_objectCBs.size());

	cmdList->IASetVertexBuffers(0, 1, &drawnMeshR->Mesh->VertexBufferView());
	cmdList->IASetIndexBuffer(&drawnMeshR->Mesh->IndexBufferView());

	auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(I(D3DApp)->GetCbvHeap()->GetGPUDescriptorHandleForHeapStart());
	cbvHandle.Offset(drawnMeshR->ObjectCBIndex, m_cbvDescriptorSize);

	//cmdList->SetGraphicsRootConstantBufferView(0, m_objectCBs[drawnMeshR->ObjectCBIndex]->GetResource()->GetGPUVirtualAddress());

	cmdList->DrawIndexedInstanced(drawnMeshR->Mesh->GetIndexCount(), drawnMeshR->GetParticleCount(), 0, 0, 0);
}

void ShaderParticle::EndDraw(ID3D12GraphicsCommandList* cmdList)
{

}

void ShaderParticle::UpdateParticleInstanceDataBuffer(int startIndex, const void* data)
{
	m_particleInstanceDataBuffer->CopyData(startIndex, data);
}
#pragma endregion