#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Core/d3dx12.h"
#include <dxgi1_4.h>
#include <wrl.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


class CHOKBAR_API D3DApp : public Win32::Window {

public:

	D3DApp(HINSTANCE);
	~D3DApp();
	void InitializeWindow();
	void CreateDepthStencilBuffer();

	void CheckMSAAQualitySupport();
	void CreateFenceAndDescriptor();

public:

	static D3DApp* GetInstance() { return m_pApp; }

	void Initialize() override;

private:

	void InitializeD3D12();
	void EnableDebugLayer();
	void CreateDevice();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetView();

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:

	static D3DApp *m_pApp;

	HINSTANCE	m_hAppInst;					// application instance handle

	// Set true to use 4X MSAA (4.1.8).  The default is false.
	bool		m_4xMsaaState = false;    // 4X MSAA enabled
	UINT		m_4xMsaaQuality = 0;      // quality level of 4X MSAA

	HINSTANCE m_pInstance;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_pDxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;

	Microsoft::WRL::ComPtr<ID3D12Device> m_pD3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

	static const int SWAP_CHAIN_BUFFER_COUNT = 2;
	int mCurrBackBuffer = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_pSwapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;

	D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

};
