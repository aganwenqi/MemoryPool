#include <iostream>
#include <string>
#include <set>
#include <ctime>
#include <thread>
#include"MemoryPool.h"
#include"AllocateManager.h"
using namespace std;
//��̬����ʱ��num1��ʾ������num2��ʾÿ���С
#define num1 1000
#define num2 10000  
class Test
{
public:
	int a;
	~Test()
	{
		//cout << a << " ";
	}
};

void TestByCjj()
{
	clock_t start;
	start = clock();
	Test * p[num1][num2];
	Test * t;
	AllocateManager<Test, allocator<Test>> pool;
	start = clock();
	int count = 0;
	//���ڴ������ռ䲢���������
	for (int i = 0; i < num1; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			t = pool.allocateJJ(1);
			t->a = count++;
			p[i][j] = t;
		}
	}
	//���ݶ�����ڴ���ͷŲ����ոÿռ�
	for (int i = 0; i < num1; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			t = p[i][j];
			pool.destroy(t);
		}
	}
	std::cout << "C++ Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << endl;
}

void TestByOne()
{
	clock_t start;
	start = clock();
	Test * p[num1][num2];
	Test * t;
	AllocateManager<Test, MemoryPool<Test, 1024>> memoryPool;
	start = clock();
	int count = 0;
	for (int i = 0; i < num1; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			t = memoryPool.allocate(1);
			t->a = count++;
			p[i][j] = t;
		}
	}
	for (int i = 0; i < num1; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			t = p[i][j];
			memoryPool.destroy(t);
		}
	}
	std::cout << "MemoryPool One Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC);
	std::cout << "  �ڴ��������" << memoryPool.getBlockSize();
	std::cout << "  �ڴ�����(byte)��" << memoryPool.getMenorySize() << std::endl;
}
void TestByBlock()
{
	clock_t start;
	start = clock();
	Test * p[num1][num2];
	Test * t;
	AllocateManager<Test, MemoryPool<Test, 1024>> memoryPool;
	start = clock();
	int count = 0;
	for (int i = 0; i < num1; i++)
	{
		t = memoryPool.allocate(num2);
		for (int j = 0; j < num2; j++)
		{
			t->a = count++;
			p[i][j] = t++;
		}
	}
	for (int i = 0; i < num1; i++)
	{
		for (int j = 0; j < num2; j++)
		{
			Test * t = p[i][j];
			memoryPool.destroy(t);
		}
	}
	std::cout << "MemoryPool Block Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC);
	std::cout << "  �ڴ��������" << memoryPool.getBlockSize();
	std::cout << "  �ڴ�����(byte)��" << memoryPool.getMenorySize() << std::endl;
}

int main()
{
	TestByCjj();
	TestByOne();
	TestByBlock();
	
	return 0;
}