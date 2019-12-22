#pragma once
#include <set>

template<typename T, typename Alloc = std::allocator<T>>
class AllocateManager
{
private:

	typedef typename Alloc::template rebind<T>::other other_;
	other_ m_allocate;//创建一个内存池管理器

public:
	//MemoryPool申请空间
	T * allocate(size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		T * node = m_allocate.allocate(size);
		m_allocate.construct(node, size);
		return node;
	}
	//Allocator申请空间
	T * allocateJJ(size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		T * node = m_allocate.allocate(size);
		m_allocate.construct(node);
		return node;
	}
	//释放并回收空间
	void destroy(T * node, size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		for (int i = 0; i < size; i++)
		{
			m_allocate.destroy(node);
			m_allocate.deallocate(node,1);
			node++;
		}
	}
	//获得当前内存池的大小
	const size_t getMenorySize()
	{
		return m_allocate.getMenorySize();
	}
	//获得当前内存池的块数
	const size_t getBlockSize()
	{
		return m_allocate.getBlockSize();
	}
};