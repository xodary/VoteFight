#pragma once
#include "DescriptorHeap.h"

class CBuffer
{
public:
	struct DescriptorType
	{
		enum Enum
		{
			SRV = 1 << 0,
			CBV = 1 << 1,
			UAV = 1 << 2,
			Structured = 1 << 3,
			Raw = 1 << 4,
		};
	};

	struct Description
	{
		UINT mNumElements;
		union
		{
			UINT mElementSize;
			UINT mSize;
		};
		UINT64 mAlignment;
		DXGI_FORMAT mFormat;
		UINT mDescriptorType;
		D3D12_RESOURCE_FLAGS mResourceFlags;
		D3D12_RESOURCE_STATES mState;
		D3D12_HEAP_TYPE mHeapType;

		Description() :
			mNumElements(1)
			, mElementSize(0)
			, mAlignment(0)
			, mDescriptorType(CBuffer::DescriptorType::SRV)
			, mFormat(DXGI_FORMAT_UNKNOWN)
			, mResourceFlags(D3D12_RESOURCE_FLAG_NONE)
			, mState(D3D12_RESOURCE_STATE_COMMON)
			, mHeapType(D3D12_HEAP_TYPE_DEFAULT)
		{}
	};

	CBuffer(Description& description, LPCWSTR name = nullptr, unsigned char* data = nullptr);
	CBuffer() {}
	virtual ~CBuffer();

	ID3D12Resource* GetResource() { return mBuffer.Get(); }

	DescriptorHandle& GetSRV() { return mDescriptorSRV; }
	DescriptorHandle& GetCBV() { return mDescriptorCBV; }

	unsigned char* Map()
	{
		if (mCBVMappedData == nullptr && mDescription.mDescriptorType & DescriptorType::CBV)
		{
			CD3DX12_RANGE readRange(0, 0);
			DX::ThrowIfFailed(mBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mCBVMappedData)));
		}

		return mCBVMappedData;
	}

private:
	Description mDescription;

	UINT mBufferSize;
	unsigned char* mData;
	unsigned char* mCBVMappedData;

	ComPtr<ID3D12Resource> mBuffer;
	ComPtr<ID3D12Resource> mBufferUpload;

	DescriptorHandle mDescriptorCBV;
	DescriptorHandle mDescriptorSRV;

	void CreateResources(DescriptorHeapManager* descriptorManager);
};

