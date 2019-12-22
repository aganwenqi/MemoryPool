#pragma once
#include <mutex>

template<typename T, int BlockSize = 6, int Block = sizeof(T) * BlockSize>
class MemoryPool
{
public:
	template<typename F>
	struct rebind
	{
		typedef MemoryPool<F, BlockSize> other;
	};
	MemoryPool()
	{
		m_FreeHeadSlot = nullptr;
		m_headSlot = nullptr;
		m_currentSlot = nullptr;
		m_LaterSlot = nullptr;
		m_MenorySize = 0;
		m_BlockSize = 0;
	}
	~MemoryPool()
	{
		//将每一块内存delete
		while (m_headSlot)
		{
			Slot_pointer pre = m_headSlot;
			m_headSlot = m_headSlot->next;
			operator delete(reinterpret_cast<void*>(pre));
		}
	}
	//申请空间
	T * allocateOne()
	{
		//空闲的位置有空间用空闲的位置
		if (m_FreeHeadSlot)
		{
			Slot_pointer pre = m_FreeHeadSlot;
			m_FreeHeadSlot = m_FreeHeadSlot->next;
			return reinterpret_cast<T*>(pre);
		}
		//申请一块内存
		if (m_currentSlot >= m_LaterSlot)
		{
			Char_pointer blockSize = reinterpret_cast<Char_pointer>(operator new(Block + sizeof(Slot_pointer)));

			m_MenorySize += (Block + sizeof(Slot_pointer));
			m_BlockSize++;

			reinterpret_cast<Slot_pointer>(blockSize)->next = m_headSlot;//将新内存放在表头
			m_headSlot = reinterpret_cast<Slot_pointer>(blockSize);

			m_currentSlot = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));//跳过指向下一块的指针这段内存
			m_LaterSlot = reinterpret_cast<Slot_pointer>(blockSize + Block + sizeof(Slot_pointer) - sizeof(Slot_)+1);//指向最后一个内存的开头位置
		}

		return reinterpret_cast<T*>(m_currentSlot++);
	}

	/*动态分配空间,注意：分配超过2个空间会在块里面创建占用4字节的空间存放数组的指针，
	这个空间不会被回收，所以动态分配最好分配大空间才使用动态
	*/
	T * allocate(size_t size = 1)
	{
		std::unique_lock<std::mutex> lock{ this->m_lock };

		//申请一个空间
		if (size == 1)
			return allocateOne();

		Slot_pointer pReSult = nullptr;
		/*先计算最后申请的块空间够不够，不适用回收的空间，因为回收空间不是连续*/
		int canUseSize = reinterpret_cast<int>(m_LaterSlot) + sizeof(Slot_) - 1 - reinterpret_cast<int>(m_currentSlot);

		int applySize = sizeof(T) * size + sizeof(T*);//创建数组对象时多了个指针，所以内存要加个指针的大小
		if (applySize <= canUseSize) //空间足够,把剩余空间分配出去
		{
			pReSult = m_currentSlot;
			m_currentSlot = reinterpret_cast<Slot_pointer>(reinterpret_cast<Char_pointer>(m_currentSlot) + applySize);
			return reinterpret_cast<T*>(pReSult);
		}

		/*空间不够动态分配块大小,不把上一块剩余的空间使用是因为空间是需要连续，
		所以上一块会继续往前推供下次使用*/
		Char_pointer blockSize = reinterpret_cast<Char_pointer>(operator new(applySize + sizeof(Slot_pointer)));
		m_MenorySize += (applySize + sizeof(Slot_pointer));
		m_BlockSize++;
		if (!m_headSlot)//目前没有一块内存情况
		{
			reinterpret_cast<Slot_pointer>(blockSize)->next = m_headSlot;
			m_headSlot = reinterpret_cast<Slot_pointer>(blockSize);
			m_currentSlot = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));
			m_LaterSlot = reinterpret_cast<Slot_pointer>(blockSize + Block + sizeof(Slot_pointer) - sizeof(Slot_) + 1);
			pReSult = m_currentSlot;
			m_currentSlot = m_LaterSlot;//第一块内存且是动态分配，所以这一块内存是满的
		}
		else
		{
			//这个申请一块动态内存就用完，直接往头后面移动
			Slot_pointer currentSlot = nullptr;
			Slot_pointer next = m_headSlot->next;
			currentSlot = reinterpret_cast<Slot_pointer>(blockSize);
			currentSlot->next = next;
			m_headSlot->next = currentSlot;
			pReSult = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));//跳过指向下一块的指针这段内存
		}
		return reinterpret_cast<T*>(pReSult);
	}

	//使用空间
	void construct(T * p, size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		if (size == 1)
			new (p)T();
		else
			new (p)T[size]();
	}

	//析构一个对象
	void destroy(T * p)
	{
		p->~T();
	}

	//回收一个空间
	void deallocate(T * p, size_t count = 1)
	{
		std::unique_lock<std::mutex> lock{ this->m_lock };

		reinterpret_cast<Slot_pointer>(p)->next = m_FreeHeadSlot;
		m_FreeHeadSlot = reinterpret_cast<Slot_pointer>(p);
	}

	const size_t getMenorySize()
	{
		return m_MenorySize;
	}
	const size_t getBlockSize()
	{
		return m_BlockSize;
	}
private:
	union Slot_
	{
		T _data;
		Slot_ * next;
	};
	typedef Slot_* Slot_pointer;
	typedef char*  Char_pointer;

	Slot_pointer m_FreeHeadSlot;//空闲的空间头部位置
	Slot_pointer m_headSlot;//指向的头位置
	Slot_pointer m_currentSlot;//当前所指向的位置
	Slot_pointer m_LaterSlot;//指向最后一个元素的开始位置

	size_t m_MenorySize;
	size_t m_BlockSize;

	// 同步
	std::mutex m_lock;
	static_assert(BlockSize > 0, "BlockSize can not zero");
};