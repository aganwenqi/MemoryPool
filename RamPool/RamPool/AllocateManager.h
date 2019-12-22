#pragma once
#include <set>

template<typename T, typename Alloc = std::allocator<T>>
class AllocateManager
{
private:

	typedef typename Alloc::template rebind<T>::other other_;
	other_ m_allocate;//����һ���ڴ�ع�����

public:
	//MemoryPool����ռ�
	T * allocate(size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		T * node = m_allocate.allocate(size);
		m_allocate.construct(node, size);
		return node;
	}
	//Allocator����ռ�
	T * allocateJJ(size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		T * node = m_allocate.allocate(size);
		m_allocate.construct(node);
		return node;
	}
	//�ͷŲ����տռ�
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
	//��õ�ǰ�ڴ�صĴ�С
	const size_t getMenorySize()
	{
		return m_allocate.getMenorySize();
	}
	//��õ�ǰ�ڴ�صĿ���
	const size_t getBlockSize()
	{
		return m_allocate.getBlockSize();
	}
};