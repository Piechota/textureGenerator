#pragma once

#pragma comment( lib, "d3d12" )
#pragma comment( lib, "dxgi" )
#pragma comment( lib, "d3dcompiler" )

#include <d3d12.h>
#include <dxgi1_4.h>

class CRender
{
private:

#ifdef _DEBUG
	ID3D12Debug* m_debugController;
#endif
	ID3D12Fence* m_fence;
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_textureCL;
	ID3D12CommandAllocator*	m_textureCA;
	ID3D12CommandQueue* m_textureCQ;
	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_texturePSO;

	ID3D12DescriptorHeap* m_renderTargetHeap;
	ID3D12Resource* m_renderTargetRes;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_renderTargetFootprint;
	UINT64 m_bufferSize;
	UINT m_numRows;
	UINT64 m_rowSize;
	
	UINT m_fenceValue;
	HANDLE m_fenceEvent;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	BYTE* m_renderTargetData;

public:
	void Init();
	void Release();
	void GenerateImage();
	void* GetRenderTargetData();
};

extern CRender GRender;
