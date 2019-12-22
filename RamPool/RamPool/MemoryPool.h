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
		//��ÿһ���ڴ�delete
		while (m_headSlot)
		{
			Slot_pointer pre = m_headSlot;
			m_headSlot = m_headSlot->next;
			operator delete(reinterpret_cast<void*>(pre));
		}
	}
	//����ռ�
	T * allocateOne()
	{
		//���е�λ���пռ��ÿ��е�λ��
		if (m_FreeHeadSlot)
		{
			Slot_pointer pre = m_FreeHeadSlot;
			m_FreeHeadSlot = m_FreeHeadSlot->next;
			return reinterpret_cast<T*>(pre);
		}
		//����һ���ڴ�
		if (m_currentSlot >= m_LaterSlot)
		{
			Char_pointer blockSize = reinterpret_cast<Char_pointer>(operator new(Block + sizeof(Slot_pointer)));

			m_MenorySize += (Block + sizeof(Slot_pointer));
			m_BlockSize++;

			reinterpret_cast<Slot_pointer>(blockSize)->next = m_headSlot;//�����ڴ���ڱ�ͷ
			m_headSlot = reinterpret_cast<Slot_pointer>(blockSize);

			m_currentSlot = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));//����ָ����һ���ָ������ڴ�
			m_LaterSlot = reinterpret_cast<Slot_pointer>(blockSize + Block + sizeof(Slot_pointer) - sizeof(Slot_)+1);//ָ�����һ���ڴ�Ŀ�ͷλ��
		}

		return reinterpret_cast<T*>(m_currentSlot++);
	}

	/*��̬����ռ�,ע�⣺���䳬��2���ռ���ڿ����洴��ռ��4�ֽڵĿռ��������ָ�룬
	����ռ䲻�ᱻ���գ����Զ�̬������÷����ռ��ʹ�ö�̬
	*/
	T * allocate(size_t size = 1)
	{
		std::unique_lock<std::mutex> lock{ this->m_lock };

		//����һ���ռ�
		if (size == 1)
			return allocateOne();

		Slot_pointer pReSult = nullptr;
		/*�ȼ����������Ŀ�ռ乻�����������û��յĿռ䣬��Ϊ���տռ䲻������*/
		int canUseSize = reinterpret_cast<int>(m_LaterSlot) + sizeof(Slot_) - 1 - reinterpret_cast<int>(m_currentSlot);

		int applySize = sizeof(T) * size + sizeof(T*);//�����������ʱ���˸�ָ�룬�����ڴ�Ҫ�Ӹ�ָ��Ĵ�С
		if (applySize <= canUseSize) //�ռ��㹻,��ʣ��ռ�����ȥ
		{
			pReSult = m_currentSlot;
			m_currentSlot = reinterpret_cast<Slot_pointer>(reinterpret_cast<Char_pointer>(m_currentSlot) + applySize);
			return reinterpret_cast<T*>(pReSult);
		}

		/*�ռ䲻����̬������С,������һ��ʣ��Ŀռ�ʹ������Ϊ�ռ�����Ҫ������
		������һ��������ǰ�ƹ��´�ʹ��*/
		Char_pointer blockSize = reinterpret_cast<Char_pointer>(operator new(applySize + sizeof(Slot_pointer)));
		m_MenorySize += (applySize + sizeof(Slot_pointer));
		m_BlockSize++;
		if (!m_headSlot)//Ŀǰû��һ���ڴ����
		{
			reinterpret_cast<Slot_pointer>(blockSize)->next = m_headSlot;
			m_headSlot = reinterpret_cast<Slot_pointer>(blockSize);
			m_currentSlot = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));
			m_LaterSlot = reinterpret_cast<Slot_pointer>(blockSize + Block + sizeof(Slot_pointer) - sizeof(Slot_) + 1);
			pReSult = m_currentSlot;
			m_currentSlot = m_LaterSlot;//��һ���ڴ����Ƕ�̬���䣬������һ���ڴ�������
		}
		else
		{
			//�������һ�鶯̬�ڴ�����ֱ꣬����ͷ�����ƶ�
			Slot_pointer currentSlot = nullptr;
			Slot_pointer next = m_headSlot->next;
			currentSlot = reinterpret_cast<Slot_pointer>(blockSize);
			currentSlot->next = next;
			m_headSlot->next = currentSlot;
			pReSult = reinterpret_cast<Slot_pointer>(blockSize + sizeof(Slot_pointer));//����ָ����һ���ָ������ڴ�
		}
		return reinterpret_cast<T*>(pReSult);
	}

	//ʹ�ÿռ�
	void construct(T * p, size_t size = 1)
	{
		//_SCL_SECURE_ALWAYS_VALIDATE(size != 0);
		if (size == 1)
			new (p)T();
		else
			new (p)T[size]();
	}

	//����һ������
	void destroy(T * p)
	{
		p->~T();
	}

	//����һ���ռ�
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

	Slot_pointer m_FreeHeadSlot;//���еĿռ�ͷ��λ��
	Slot_pointer m_headSlot;//ָ���ͷλ��
	Slot_pointer m_currentSlot;//��ǰ��ָ���λ��
	Slot_pointer m_LaterSlot;//ָ�����һ��Ԫ�صĿ�ʼλ��

	size_t m_MenorySize;
	size_t m_BlockSize;

	// ͬ��
	std::mutex m_lock;
	static_assert(BlockSize > 0, "BlockSize can not zero");
};