#include "render.h"

#include <d3dcompiler.h>
#include <windows.h>
#include <comdef.h>
#include <tchar.h>

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)

char const* vsShader =
"struct VSOutput"
"{"
"	float4 position : SV_POSITION;"
"	float2 uv : TEXCOORD;"
"};"
"VSOutput vsFullScreen(const uint vID : SV_VERTEXID)"
"{"
"	VSOutput result;"
"	const float4 ids = float4(int2(vID >> 1, vID & 1), 0.f, 1.f);"
"	result.position = mad(ids, float4(4.f, 4.f, 1.f, 1.f), float4(-1.f, -1.f, 0.f, 0.f));"
"	result.uv = mad(ids.xy, float2(2.f, -2.f), float2(0.f, 1.f));"
"	return result;"
"}";

char const* psShader =
"struct PSInput"
"{"
"	float4 position : SV_POSITION;"
"	float2 uv : TEXCOORD;"
"};"
"float4 psMain(PSInput input) : SV_TARGET"
"{"
"	return float4(1.f, 1.f, 1.f, 1.f);"
"}";

void CheckResult(HRESULT result)
{
	if (FAILED(result))
	{
		_com_error err(result);
		::OutputDebugString(err.ErrorMessage());
		throw;
	}
}
void CheckResult(HRESULT result, ID3DBlob* errorBlob )
{
	if (FAILED(result))
	{
		_com_error err(result);
		::OutputDebugString(err.ErrorMessage());
		::OutputDebugStringA("\n");
		::OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		throw;
	}
	if (errorBlob && 0 < errorBlob->GetBufferSize())
	{
		::OutputDebugString((LPCWSTR)errorBlob->GetBufferPointer());
	}
}

void CRender::Init()
{
#ifdef _DEBUG
	CheckResult(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)));
	m_debugController->EnableDebugLayer();
#endif

	m_viewport.MaxDepth = 1.f;
	m_viewport.MinDepth = 0.f;
	m_viewport.Width = 600.f;
	m_viewport.Height = 600.f;
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;

	m_scissorRect.right = 600;
	m_scissorRect.bottom = 600;
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;

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

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	CheckResult(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_copyCQ)));
	CheckResult(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_copyCA)));
	CheckResult(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copyCA, nullptr, IID_PPV_ARGS(&m_copyCL)));

	m_textureCL->Close();
	m_copyCL->Close();

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

	CheckResult(m_device->CreateCommittedResource(&renderTargetHeapProp, D3D12_HEAP_FLAG_NONE, &renderTargetResDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_renderTargetRes)));
	m_device->CreateRenderTargetView(m_renderTargetRes, nullptr, m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());

	UINT64 bufferSize;
	UINT numRows;
	UINT64 rowSize;
	m_device->GetCopyableFootprints(&renderTargetResDesc, 0, 1, 0, &m_renderTargetFootprint, &numRows, &rowSize, &bufferSize);

	D3D12_HEAP_PROPERTIES const cpuAccessHeapProp =
	{
		D3D12_HEAP_TYPE_READBACK,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0, 0
	};
	D3D12_RESOURCE_DESC const cpuAccessResDesc =
	{
		D3D12_RESOURCE_DIMENSION_BUFFER,
		0,
		bufferSize,
		1,
		1,
		1,
		DXGI_FORMAT_UNKNOWN,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE
	};
	CheckResult(m_device->CreateCommittedResource(&cpuAccessHeapProp, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &cpuAccessResDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_cpuAccessRes)));

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
	CheckResult(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob), errorBlob);
	CheckResult(m_device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	ID3DBlob* vsShaderBlob;
	ID3DBlob* psShaderBlob;

	CheckResult( D3DCompile(vsShader, strlen(vsShader), NULL, nullptr, nullptr, "vsFullScreen", "vs_5_0", 0, 0, &vsShaderBlob, &errorBlob), errorBlob );
	CheckResult( D3DCompile(psShader, strlen(psShader), NULL, nullptr, nullptr, "psMain", "ps_5_0", 0, 0, &psShaderBlob, &errorBlob), errorBlob );

	D3D12_RASTERIZER_DESC rasterizerState =
	{
		/*FillMode*/					D3D12_FILL_MODE_SOLID
		/*CullMode*/					,D3D12_CULL_MODE_BACK
		/*FrontCounterClockwise*/		,FALSE
		/*DepthBias*/					,D3D12_DEFAULT_DEPTH_BIAS
		/*DepthBiasClamp*/				,D3D12_DEFAULT_DEPTH_BIAS_CLAMP
		/*SlopeScaledDepthBias*/		,D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS
		/*DepthClipEnable*/				,FALSE
		/*MultisampleEnable*/			,FALSE
		/*AntialiasedLineEnable*/		,FALSE
		/*ForcedSampleCount*/			,0
		/*ConservativeRaster*/			,D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	static D3D12_BLEND_DESC const blendState =
	{
		/*AlphaToCoverageEnable*/					FALSE
		/*IndependentBlendEnable*/					,FALSE
		/*RenderTarget[0].BlendEnable*/				,FALSE
		/*RenderTarget[0].LogicOpEnable*/			,FALSE
		/*RenderTarget[0].SrcBlend*/				,D3D12_BLEND_ONE
		/*RenderTarget[0].DestBlend*/				,D3D12_BLEND_ZERO
		/*RenderTarget[0].BlendOp*/					,D3D12_BLEND_OP_ADD
		/*RenderTarget[0].SrcBlendAlpha*/			,D3D12_BLEND_ONE
		/*RenderTarget[0].DestBlendAlpha*/			,D3D12_BLEND_ZERO
		/*RenderTarget[0].BlendOpAlpha*/			,D3D12_BLEND_OP_ADD
		/*RenderTarget[0].LogicOp*/					,D3D12_LOGIC_OP_NOOP
		/*RenderTarget[0].RenderTargetWriteMask*/	,D3D12_COLOR_WRITE_ENABLE_ALL
		/*RenderTarget[1].BlendEnable*/				,FALSE
		/*RenderTarget[1].LogicOpEnable*/			,FALSE
		/*RenderTarget[1].SrcBlend*/				,D3D12_BLEND_ONE
		/*RenderTarget[1].DestBlend*/				,D3D12_BLEND_ZERO
		/*RenderTarget[1].BlendOp*/					,D3D12_BLEND_OP_ADD
		/*RenderTarget[1].SrcBlendAlpha*/			,D3D12_BLEND_ONE
		/*RenderTarget[1].DestBlendAlpha*/			,D3D12_BLEND_ZERO
		/*RenderTarget[1].BlendOpAlpha*/			,D3D12_BLEND_OP_ADD
		/*RenderTarget[1].LogicOp*/					,D3D12_LOGIC_OP_NOOP
		/*RenderTarget[1].RenderTargetWriteMask*/	,D3D12_COLOR_WRITE_ENABLE_ALL
	};
	static D3D12_DEPTH_STENCIL_DESC const depthStencilState =
	{
		/*DepthEnable*/							FALSE
		/*DepthWriteMask*/						,D3D12_DEPTH_WRITE_MASK_ALL
		/*DepthFunc*/							,D3D12_COMPARISON_FUNC_LESS
		/*StencilEnable*/						,FALSE
		/*StencilReadMask*/						,D3D12_DEFAULT_STENCIL_READ_MASK
		/*StencilWriteMask*/					,D3D12_DEFAULT_STENCIL_WRITE_MASK
		/*FrontFace.StencilFailOp*/				,{ D3D12_STENCIL_OP_REPLACE
		/*FrontFace.StencilDepthFailOp*/		,D3D12_STENCIL_OP_REPLACE
		/*FrontFace.StencilPassOp*/				,D3D12_STENCIL_OP_REPLACE
		/*FrontFace.StencilFunc*/				,D3D12_COMPARISON_FUNC_ALWAYS }
		/*BackFace.StencilFailOp*/				,{ D3D12_STENCIL_OP_REPLACE
		/*BackFace.StencilDepthFailOp*/			,D3D12_STENCIL_OP_REPLACE
		/*BackFace.StencilPassOp*/				,D3D12_STENCIL_OP_REPLACE
		/*BackFace.StencilFunc*/				,D3D12_COMPARISON_FUNC_ALWAYS }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS.BytecodeLength = vsShaderBlob->GetBufferSize();
	psoDesc.VS.pShaderBytecode = vsShaderBlob->GetBufferPointer();
	psoDesc.PS.BytecodeLength = psShaderBlob->GetBufferSize();
	psoDesc.PS.pShaderBytecode = psShaderBlob->GetBufferPointer();
	
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	CheckResult(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_texturePSO)));

	rootSignatureBlob->Release();
	vsShaderBlob->Release();
	psShaderBlob->Release();
	if ( errorBlob )
	{
		errorBlob->Release();
	}
}

void CRender::Release()
{
	m_cpuAccessRes->Unmap(0, nullptr);
	m_cpuAccessRes->Release();
	m_renderTargetRes->Release();
	m_renderTargetHeap->Release();
	m_texturePSO->Release();
	m_rootSignature->Release();
	m_textureCL->Release();
	m_textureCA->Release();
	m_textureCQ->Release();
	m_copyCL->Release();
	m_copyCA->Release();
	m_copyCQ->Release();
	m_fence->Release();
	m_device->Release();

#ifdef _DEBUG
	m_debugController->Release();
#endif // _DEBUG
}

void CRender::GenerateImage()
{
	D3D12_RESOURCE_BARRIER renderTargetBarrier = {};
	renderTargetBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderTargetBarrier.Transition.pResource = m_renderTargetRes;
	renderTargetBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	renderTargetBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderTargetBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_textureCA->Reset();
	m_textureCL->Reset(m_textureCA, m_texturePSO);
	m_textureCL->ResourceBarrier(1, &renderTargetBarrier);
	m_textureCL->OMSetRenderTargets(1, &m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	m_textureCL->RSSetViewports(1, &m_viewport);
	m_textureCL->RSSetScissorRects(1, &m_scissorRect);
	m_textureCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_textureCL->SetGraphicsRootSignature(m_rootSignature);
	m_textureCL->DrawInstanced(3, 1, 0, 0);

	renderTargetBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	renderTargetBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_textureCL->ResourceBarrier(1, &renderTargetBarrier);

	m_textureCL->Close();

	m_textureCQ->ExecuteCommandLists(1, (ID3D12CommandList**)&m_textureCL);

	m_copyCA->Reset();
	m_copyCL->Reset(m_copyCA, nullptr);

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = m_cpuAccessRes;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint = m_renderTargetFootprint;

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = m_renderTargetRes;
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	m_copyCL->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	m_copyCL->Close();

	CheckResult(m_textureCQ->Signal(m_fence, m_fenceValue));
	CheckResult(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
	++m_fenceValue;
	WaitForSingleObject(m_fenceEvent, INFINITE);

	m_copyCQ->ExecuteCommandLists(1, (ID3D12CommandList**)&m_copyCL);

	CheckResult(m_copyCQ->Signal(m_fence, m_fenceValue));
	CheckResult(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
	++m_fenceValue;
	WaitForSingleObject(m_fenceEvent, INFINITE);

	CheckResult(m_cpuAccessRes->Map(0, nullptr, (void**)&m_renderTargetData));
}

void* CRender::GetRenderTargetData()
{
	return m_renderTargetData;
}

CRender GRender;