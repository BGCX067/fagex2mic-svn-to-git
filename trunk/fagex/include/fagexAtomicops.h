
/************************************************************************/
/*                                                                      
	reference counts方面的原子操作参见atomic_refcount.h.
	sequence numbers方面的原子操作参见atomic_sequence_num.h.

	本模块暴露的例程具有一定风险. 不仅需要正确编码, 还需要对原子化(atomicity)
	和内存有序化(memory ordering)仔细推理; 可读性和可维护性都较差. 假如不使用
	会有严重影响或者别无选择的时候才考虑这些函数. 最好只使用有显式安全说明
	的函数, 最好只在x86平台上使用, 其它的架构平台上会导致崩溃. 如果不能确保
	这些, 尽量不要使用, 可以选择使用Mutex.

	直接存取原子变量是错误的. 应该使用NoBarrier版本的存取函数:
	NoBarrier_Store()
	NoBarrier_Load()
	当前的编译器层面没有做强制要求, 但是最好这样做.
*/
/************************************************************************/
#pragma once

#include <Windows.h>
#include "fagexBaseType.h"

namespace fagex
{
	namespace subtle
	{
		//intptr_t在64位下为64位长度，32位系统为32位长度
		typedef intptr_t AtomicWord; 

		//判断原子变量是否很系统要求的长度一致
		COMPILE_ASSERT(sizeof(AtomicWord) == sizeof(LPVOID*), __atomic_is_invalid);

		// 原子操作:
		//     result = *ptr;
		//     if(*ptr == old_value)
		//         *ptr = new_value;
		//     return result;
		//
		// 如果"*ptr"=="old_value"则被替换成"new_value", 返回"*ptr"以前的值.
		// 没有内存屏障(memory barriers).
		AtomicWord NoBarrier_CompareAndSwap(volatile AtomicWord* ptr,
			AtomicWord old_value, AtomicWord new_value);

		// 原子操作: 存储new_value到*ptr, 并返回以前的值.
		// 没有内存屏障(memory barriers).
		AtomicWord NoBarrier_AtomicExchange(volatile AtomicWord* ptr, AtomicWord new_value);

		// 原子操作: *ptr值增加"increment", 返回增加后的新值.
		// 没有内存屏障(memory barriers).
		AtomicWord NoBarrier_AtomicIncrement(volatile AtomicWord* ptr, AtomicWord increment);

		AtomicWord Barrier_AtomicIncrement(volatile AtomicWord* ptr, AtomicWord increment);

		// 下面几个底层函数对于开发自旋锁(spinlocks)、信号量(mutexes)和条件变量
		// (condition-variables)这种上层同步的开发者有用. 组合了CompareAndSwap(),
		// 取或存操作, 保证指令操作内存有序化. "Acquire"函数保证后面的内存访问不会
		// 提前; "Release"函数保证前面的内存访问不会延后. "Barrier"函数兼有
		// "Acquire"和"Release"语义. MemoryBarrier()具有"Barrier"语义但是没有访问
		// 内存.
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