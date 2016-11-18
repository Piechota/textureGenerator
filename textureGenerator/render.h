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
	ID3D12PipelineState* m_texturePSO;

	ID3D12DescriptorHeap* m_renderTargetHeap;
	ID3D12Resource* m_renderTargetRes;

	UINT m_fenceValue;
	HANDLE m_fenceEvent;

private:
	void CheckResult(HRESULT result);

public:
	void Init();
	void Release();
};

extern CRender GRender;
