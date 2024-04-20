#pragma once

class CDepthBuffer
{
public:
	CDepthBuffer(int width, int height, DXGI_FORMAT format);
	~CDepthBuffer();

	ID3D12Resource* GetResource() { return mDepthStencilResource.Get(); }
	DXGI_FORMAT GetFormat() { return mFormat; }
	void TransitionTo(std::vector<CD3DX12_RESOURCE_BARRIER>& barriers, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter);

	DescriptorHandle GetDSV()
	{
		return mDescriptorDSV;
	}

	DescriptorHandle& GetSRV()
	{
		return mDescriptorSRV;
	}

	const int GetWidth() { return mWidth; }
	const int GetHeight() { return mHeight; }

private:

	int mWidth, mHeight;
	DXGI_FORMAT mFormat;
	D3D12_RESOURCE_STATES mCurrentResourceState;

	DescriptorHandle mDescriptorDSV;
	DescriptorHandle mDescriptorSRV;
	ComPtr<ID3D12Resource> mDepthStencilResource;
};
