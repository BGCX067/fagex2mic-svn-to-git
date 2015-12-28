
/************************************************************************/
/*                                                                      
	reference counts�����ԭ�Ӳ����μ�atomic_refcount.h.
	sequence numbers�����ԭ�Ӳ����μ�atomic_sequence_num.h.

	��ģ�鱩¶�����̾���һ������. ������Ҫ��ȷ����, ����Ҫ��ԭ�ӻ�(atomicity)
	���ڴ�����(memory ordering)��ϸ����; �ɶ��ԺͿ�ά���Զ��ϲ�. ���粻ʹ��
	��������Ӱ����߱���ѡ���ʱ��ſ�����Щ����. ���ֻʹ������ʽ��ȫ˵��
	�ĺ���, ���ֻ��x86ƽ̨��ʹ��, �����ļܹ�ƽ̨�ϻᵼ�±���. �������ȷ��
	��Щ, ������Ҫʹ��, ����ѡ��ʹ��Mutex.

	ֱ�Ӵ�ȡԭ�ӱ����Ǵ����. Ӧ��ʹ��NoBarrier�汾�Ĵ�ȡ����:
	NoBarrier_Store()
	NoBarrier_Load()
	��ǰ�ı���������û����ǿ��Ҫ��, �������������.
*/
/************************************************************************/
#pragma once

#include <Windows.h>
#include "fagexBaseType.h"

namespace fagex
{
	namespace subtle
	{
		//intptr_t��64λ��Ϊ64λ���ȣ�32λϵͳΪ32λ����
		typedef intptr_t AtomicWord; 

		//�ж�ԭ�ӱ����Ƿ��ϵͳҪ��ĳ���һ��
		COMPILE_ASSERT(sizeof(AtomicWord) == sizeof(LPVOID*), __atomic_is_invalid);

		// ԭ�Ӳ���:
		//     result = *ptr;
		//     if(*ptr == old_value)
		//         *ptr = new_value;
		//     return result;
		//
		// ���"*ptr"=="old_value"���滻��"new_value", ����"*ptr"��ǰ��ֵ.
		// û���ڴ�����(memory barriers).
		AtomicWord NoBarrier_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value);

		// ԭ�Ӳ���: �洢new_value��*ptr, ��������ǰ��ֵ.
		// û���ڴ�����(memory barriers).
		AtomicWord NoBarrier_AtomicExchange(volatile AtomicWord* ptr, AtomicWord new_value);

		// ԭ�Ӳ���: *ptrֵ����"increment", �������Ӻ����ֵ.
		// û���ڴ�����(memory barriers).
		AtomicWord NoBarrier_AtomicIncrement(volatile AtomicWord* ptr, AtomicWord increment);

		AtomicWord Barrier_AtomicIncrement(volatile AtomicWord* ptr, AtomicWord increment);

		// ���漸���ײ㺯�����ڿ���������(spinlocks)���ź���(mutexes)����������
		// (condition-variables)�����ϲ�ͬ���Ŀ���������. �����CompareAndSwap(),
		// ȡ������, ��ָ֤������ڴ�����. "Acquire"������֤������ڴ���ʲ���
		// ��ǰ; "Release"������֤ǰ����ڴ���ʲ����Ӻ�. "Barrier"��������
		// "Acquire"��"Release"����. MemoryBarrier()����"Barrier"���嵫��û�з���
		// �ڴ�.
		AtomicWord Acquire_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value);
		AtomicWord Release_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value);

		void MemoryBarrier();
		void NoBarrier_Store(volatile AtomicWord* ptr, AtomicWord value);
		void Acquire_Store(volatile AtomicWord* ptr, AtomicWord value);
		void Release_Store(volatile AtomicWord* ptr, AtomicWord value);

		AtomicWord NoBarrier_Load(volatile const AtomicWord* ptr);
		AtomicWord Acquire_Load(volatile const AtomicWord* ptr);
		AtomicWord Release_Load(volatile const AtomicWord* ptr);

		////////////////////////////////////////////////////////////////////////////////////
		inline AtomicWord NoBarrier_AtomicIncrement(volatile AtomicWord* ptr,
			AtomicWord increment)
		{
			return Barrier_AtomicIncrement(ptr, increment);
		}

		inline AtomicWord Barrier_AtomicIncrement(volatile AtomicWord* ptr,
			AtomicWord increment)
		{
			return InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(ptr),
				static_cast<LONG>(increment))+increment;
		}

		inline AtomicWord NoBarrier_AtomicExchange(volatile AtomicWord* ptr,
			AtomicWord new_value)
		{
			LONG result = InterlockedExchange(
				reinterpret_cast<volatile LONG*>(ptr),
				static_cast<LONG>(new_value));
			return static_cast<AtomicWord>(result);
		}

		inline AtomicWord NoBarrier_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value)
		{
			LONG result = InterlockedCompareExchange(
				reinterpret_cast<volatile LONG*>(ptr),
				static_cast<LONG>(new_value),
				static_cast<LONG>(old_value));
			return static_cast<AtomicWord>(result);
		}

		inline AtomicWord Acquire_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value)
		{
			return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
		}

		inline AtomicWord Release_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value)
		{
			return NoBarrier_CompareAndSwap(ptr, old_value, new_value);
		}

		inline void MemoryBarrier()
		{
			::MemoryBarrier();
		}

		inline void NoBarrier_Store(volatile AtomicWord* ptr, AtomicWord value)
		{
			*ptr = value;
		}

		inline void Acquire_Store(volatile AtomicWord* ptr, AtomicWord value)
		{
			NoBarrier_AtomicExchange(ptr, value);
		}

		inline void Release_Store(volatile AtomicWord* ptr, AtomicWord value)
		{
			*ptr = value;
		}

		inline AtomicWord NoBarrier_Load(volatile const AtomicWord* ptr)
		{
			return *ptr;
		}

		inline AtomicWord Acquire_Load(volatile const AtomicWord* ptr)
		{
			AtomicWord value = *ptr;
			return value;
		}

		inline AtomicWord Release_Load(volatile const AtomicWord* ptr)
		{
			MemoryBarrier();
			return *ptr;
		}
	} //namespace subtle

} //namespace fagex