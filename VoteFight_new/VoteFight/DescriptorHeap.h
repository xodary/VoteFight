// This code is borrowed from FeaxRenderer https://github.com/KostasAAA/FeaxRenderer

#pragma once

#include "pch.h"

class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		mCPUHandle.ptr = NULL;
		mGPUHandle.ptr = NULL;
		mHeapIndex = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() { return mCPUHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() { return mGPUHandle; }
	UINT GetHeapIndex() { return mHeapIndex; }

	void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { mCPUHandle = cpuHandle; }
	void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mGPUHandle = gpuHandle; }
	void SetHeapIndex(UINT heapIndex) { mHeapIndex = heapIndex; }

	bool IsValid() { return mCPUHandle.ptr != NULL; }
	bool IsReferencedByShader() { return mGPUHandle.ptr != NULL; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE mCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE mGPUHandle;
	UINT mHeapIndex;
};

class DescriptorHeap
{
public:
	DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool isReferencedByShader = false);
	virtual ~DescriptorHeap();

	ID3D12DescriptorHeap* GetHeap() { return mDescriptorHeap.Get(); }
	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return mHeapType; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetHeapCPUStart() { return mDescriptorHeapCPUStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetHeapGPUStart() { return mDescriptorHeapGPUStart; }
	UINT GetMaxNoofDescriptors() { return mMaxNumDescriptors; }
	UINT GetDescriptorSize() { return mDescriptorSize; }

	void AddToHandle(ID3D12Device* device, DescriptorHandle& destCPUHandle, DescriptorHandle& sourceCPUHandle)
	{
		device->CopyDescriptorsSimple(1, destCPUHandle.GetCPUHandle(), sourceCPUHandle.GetCPUHandle(), mHeapType);
		destCPUHandle.GetCPUHandle().ptr += mDescriptorSize;
	}

	void AddToHandle(ID3D12Device* device, DescriptorHandle& destCPUHandle, D3D12_CPU_DESCRIPTOR_HANDLE& sourceCPUHandle)
	{
		device->CopyDescriptorsSimple(1, destCPUHandle.GetCPUHandle(), sourceCPUHandle, mHeapType);
		destCPUHandle.GetCPUHandle().ptr += mDescriptorSize;
	}

protected:
	ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
	D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHeapCPUStart;
	D3D12_GPU_DESCRIPTOR_HANDLE mDescriptorHeapGPUStart;
	UINT mMaxNumDescriptors;
	UINT mDescriptorSize;
	bool mIsReferencedByShader;
};

class CPUDescriptorHeap : public DescriptorHeap
{
public:
	CPUDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors);
	~CPUDescriptorHeap() final;

	DescriptorHandle GetNewHandle();
	void FreeHandle(DescriptorHandle handle);

private:
	std::vector<UINT> mFreeDescriptors;
	UINT mCurrentDescriptorIndex;
	UINT mActiveHandleCount;
};

class GPUDescriptorHeap : public DescriptorHeap
{
public:
	GPUDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors);
	~GPUDescriptorHeap() final {};

	void Reset();
	DescriptorHandle GetHandleBlock(UINT count);

private:
	UINT mCurrentDescriptorIndex;
};

class DescriptorHeapManager
{
public:
	DescriptorHeapManager();
	~DescriptorHeapManager();

	DescriptorHandle CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	GPUDescriptorHeap* GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
	{
		return mGPUDescriptorHeaps[heapType];
	}

private:
	CPUDescriptorHeap* mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	GPUDescriptorHeap* mGPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};
