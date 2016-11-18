#include "render.h"

#include <d3dcompiler.h>
#include <windows.h>
#include <comdef.h>

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)

void CRender::CheckResult(HRESULT result)
{
	if (FAILED(result))
	{
		_com_error err(result);
		::OutputDebugString(err.ErrorMessage());
		throw;
	}
}

void CRender::Init()
{
#ifdef _DEBUG
	CheckResult(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)));
	m_debugController->EnableDebugLayer();
#endif

	CheckResult(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
	m_fenceValue = 0;
	CheckResult(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	++m_fenceValue;
	m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (!m_fenceEvent)
	{
		throw;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	CheckResult(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_textureCQ)));
	CheckResult(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_textureCA)));
	CheckResult(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_textureCA, nullptr, IID_PPV_ARGS(&m_textureCL)));

	D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {};
	renderTargetHeapDesc.NumDescriptors = 1;
	renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	CheckResult(m_device->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)));

	D3D12_HEAP_PROPERTIES const renderTargetHeapProp =
	{
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0,
		0, 0
	};
	D3D12_RESOURCE_DESC const renderTargetResDesc = 
	{
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		600,
		600,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	};
	CheckResult(m_device->CreateCommittedResource(&renderTargetHeapProp, D3D12_HEAP_FLAG_NONE, &renderTargetResDesc, D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_renderTargetRes)));
	m_device->CreateRenderTargetView(m_renderTargetRes, nullptr, m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
}

void CRender::Release()
{
	m_renderTargetRes->Release();
	m_renderTargetHeap->Release();
	m_textureCL->Release();
	m_textureCA->Release();
	m_textureCQ->Release();
	m_fence->Release();
	m_device->Release();
#ifdef _DEBUG
	m_debugController->Release();
#endif // _DEBUG
}

CRender GRender;