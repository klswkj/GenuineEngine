#include "stdafx.h"
#include "SunRaysPass.h"
#include "Core/DeferredRenderer.h"
#include <algorithm>

struct SunRayConstBuffer
{
	XMFLOAT2 SunPos;
};

void SunRaysPass::CreatePSO()
{
	auto device = computeCore->GetDevice();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"SunRaysPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = nullptr;
	descPipelineState.InputLayout.NumElements = 0;// _countof(inputLayout);
	descPipelineState.pRootSignature = renderer->GetRootSignature();
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = false;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.DepthClipEnable = false;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&sunRaysPSO));
}

void SunRaysPass::CreateCB()
{
	auto device = computeCore->GetDevice();
	cbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = 1024 * 32;
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer));

	cbWrapper.Initialize(constantBuffer, (sizeof(SunRayConstBuffer) + 255) & ~255);

	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = (sizeof(SunRayConstBuffer) + 255) & ~255;
	device->CreateConstantBufferView(&descBuffer, cbHeap.handleCPU(0));
}

SunRaysPass::SunRaysPass(ComputeCore* core, DeferredRenderer* renderContext)
{
	computeCore = core;
	renderer = renderContext;
	occlusionPassCS = std::make_unique<ComputeProcess>(core, L"OcclusionPassCS.cso");
	sunRaysPassCS = std::make_unique<ComputeProcess>(core, L"SunRaysCS.cso");
	compositeRaysCS = std::make_unique<ComputeProcess>(core, L"CompositeRaysCS.cso");
	CreatePSO();
	CreateCB();
}

Texture* SunRaysPass::GetOcclusionTexture(ID3D12GraphicsCommandList* commandList, Texture * depthSRV, TexturePool *texturePool)
{
	//if (occlusionTexCache.size() != 3)
	//{
	//	int index;
	//	occlusionTexCache.push_back(texturePool->Request(DXGI_FORMAT_R32G32B32A32_FLOAT, 1920, 1080, TextureTypeSRV, &index, false));
	//	ocTexIndex.push_back(index);
	//}
	//auto uav = texturePool->GetUAV(ocTexIndex[occlusionTexIndex]);
	//auto srv = texturePool->GetSRV(ocTexIndex[occlusionTexIndex]);
	//occlusionTexIndex = (occlusionTexIndex + 1) % 3;

	auto texResource = texturePool->GetNext();
	auto uavTex = texResource.UAV;// uav;
	auto srvTex = texResource.SRV;// srv;
	occlusionPassCS->SetShader(commandList);
	occlusionPassCS->SetTextureSRV(commandList, depthSRV);
	occlusionPassCS->SetTextureUAV(commandList, uavTex);
	occlusionPassCS->Dispatch(commandList, computeCore->GetRenderer()->GetWidth(), computeCore->GetRenderer()->GetHeight(), 1);
	return srvTex;
}

Texture* SunRaysPass::Apply(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, Texture* pixels, TexturePool* texturePool, Camera* camera)
{	
	auto camDir = XMLoadFloat3(&camera->GetDirection());
	
	auto viewProjT = camera->GetViewProjectionMatrix();
	auto viewProj = XMLoadFloat4x4(&viewProjT);
	auto sunDir = XMVector3Normalize(XMVectorSet(1.0f, 1.5f, 4.f, 0.f));
	float dotCamSun = XMVectorGetX(XMVector3Dot(camDir, sunDir));
	if (dotCamSun < 0.f) return pixels;

	auto vSunPos = -200.f * sunDir;
	auto eyePos = XMLoadFloat3(&camera->GetPosition());
	XMVectorSetX(vSunPos, XMVectorGetX(vSunPos) + XMVectorGetX(eyePos));
	XMVectorSetZ(vSunPos, XMVectorGetZ(vSunPos) + XMVectorGetZ(eyePos));
	auto vSunPosSS = XMVector3TransformCoord(vSunPos, viewProj);
	static const float MaxSunDist = 1.7f;
	auto sunColor = XMVectorSet(0.5f, 0.5f, 0.05f, 0.f);
	XMFLOAT2 sunPos(0.5f * XMVectorGetX(vSunPosSS) + 0.5f, -0.5f * XMVectorGetY(vSunPosSS) + 0.5f);
	float maxDist = std::max(XMVectorGetX(vSunPosSS), XMVectorGetY(vSunPosSS));
	if ((MaxSunDist - maxDist) < 0.f)
	{
		return pixels;
	}

	if (maxDist > 0.5f)
	{
		sunColor *= (MaxSunDist - maxDist);
	}

	XMFLOAT3 rayColor;
	XMStoreFloat3(&rayColor, sunColor);
	auto constb = SunRayConstBuffer{ sunPos };
	cbWrapper.CopyData(&constb, sizeof(SunRayConstBuffer), 0);

	auto lightRaysTexResource = texturePool->GetNext();
	auto outResource = texturePool->GetNext();
	auto lightRaysSRV = lightRaysTexResource.SRV;
	auto rayRTV = lightRaysTexResource.RTV;

	auto outSRV = outResource.SRV;
	auto outUAV = outResource.UAV;
	
	
	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightRaysSRV->GetTextureResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET));
	auto occlusionTex = GetOcclusionTexture(commandList, depthSRV, texturePool);
	commandList->SetPipelineState(sunRaysPSO);

	auto frame = renderer->GetFrameManager();
	auto fheapParams = renderer->GetFrameHeapParameters();

	ID3D12DescriptorHeap* heaps[] = { frame->GetDescriptorHeap() };
	ID3D12DescriptorHeap* cbheaps[] = { cbHeap.pDescriptorHeap.Get() };
	commandList->ClearRenderTargetView(rayRTV, mClearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rayRTV, true, nullptr);
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(fheapParams.Textures, occlusionTex->GetHeapIndex()));

	auto cbIndex = frame->CopyAllocate(1, cbHeap, 0);
	commandList->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frame->GetGPUHandle(cbIndex));
	renderer->DrawScreenQuad(commandList);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightRaysSRV->GetTextureResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	compositeRaysCS->SetShader(commandList);
	compositeRaysCS->SetTextureSRV(commandList, lightRaysSRV);
	compositeRaysCS->SetTextureSRVOffset(commandList, pixels);
	compositeRaysCS->SetTextureUAV(commandList, outUAV);
	compositeRaysCS->SetConstants(commandList, &rayColor, 3, 0);
	compositeRaysCS->Dispatch(commandList, computeCore->GetRenderer()->GetWidth(), computeCore->GetRenderer()->GetHeight(), 1);
	return outSRV;
}


SunRaysPass::~SunRaysPass()
{
	sunRaysPSO->Release();
	cbHeap.pDescriptorHeap->Release();
	constantBuffer->Release();
}
