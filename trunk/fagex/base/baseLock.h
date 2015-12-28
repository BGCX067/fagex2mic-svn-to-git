

#pragma once
#include <Windows.h>
#include "base/baseBasicType.h"
#include "base/basePlatformThread.h"
#include "base/baseLogging.h"

namespace base
{
	namespace internal
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		class LockDelegate
		{
		public:
			// 如果当前没有锁, 上锁并返回true, 否则立即返回错误.
			virtual bool Try() = 0;
			
			// 加锁, 函数堵塞直到加锁成功.
			virtual void Lock() = 0;
			
			// 解锁. 只能由上锁者调用: Try()成功返回或者Lock()成功后.
			virtual void Unlock() = 0;
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		class Win32Lock : public LockDelegate
		{
		public:
			Win32Lock()
			{
				// 第二个参数是自旋技术, 对于短锁的情况可以避免线程切换提高性能.
				// MSDN说: 第二个参数在多处理器情况下, 一个线程在请求已加锁的
				//         critical section时会进入循环, 当超时后才会进入sleep.
				InitializeCriticalSectionAndSpinCount(&_critical_section, 2000);
			}

			~Win32Lock()
			{
				DeleteCriticalSection(&_critical_section);
			}

			bool Try()
			{
				if(TryEnterCriticalSection(&_critical_section) != FALSE)
				{
					return true;
				}
				return false;
			}

			// 加锁, 函数堵塞直到加锁成功.
			void Lock()
			{
				EnterCriticalSection(&_critical_section);
			}

			// 解锁. 只能由上锁者调用: Try()成功返回或者Lock()成功后.
			void Unlock()
			{
				LeaveCriticalSection(&_critical_section);
			}

		private:
			CRITICAL_SECTION _critical_section;
			DISALLOW_COPY_AND_ASSIGN(Win32Lock);
		};
		typedef Win32Lock LockImpl;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		template<class LockDelegateImpl>
		class LockService
		{
		public:
#if defined(NDEBUG)
			LockService() : _impl() {}
			~LockService() {}

			void Acquire() { _impl.Lock(); }
			void Release() { _impl.Unlock(); }

			// 如果没有上锁, 上锁并返回true. 如果已经被其它线程上锁, 立即返回false.
			// 已经上锁的线程不要再调用(可能产生预期不到的断言失败).
			bool Try() { return _impl.Try(); }

			// 非debug模式下是空实现.
			void AssertAcquired() const {}
#else //!NDEBUG
			LockService(): _impl()
			{
				owned_by_thread_ = false;
				owning_thread_id_ = base::kInvalidThreadId;
			}
			~LockService() {}

			// 注意: 尽管windows的临界区支持递归加锁, 但是这里不允许, 如果线程第二次
			// 请求加锁(已经加锁)会触发DCHECK().
			void Acquire()
			{
				_impl.Lock();
				CheckUnheldAndMark();
			}

			void Release()
			{
				CheckHeldAndUnmark();
				_impl.Unlock();
			}

			bool Try()
			{
				bool rv = _impl.Try();
				if(rv)
				{
					CheckUnheldAndMark();
				}
				return rv;
			}

			void AssertAcquired() const
			{
				DCHECK(owned_by_thread_);
				DCHECK_EQ(owning_thread_id_, PlatformThread::CurrentId());
			}
#endif //!NDEBUG

		private:
			LockDelegateImpl _impl;
			DISALLOW_COPY_AND_ASSIGN(LockService<LockDelegateImpl>);

#if !defined(NDEBUG)
			// 下面2个函数为递归加锁时触发断言设计.
			void CheckHeldAndUnmark()
			{
				DCHECK(owned_by_thread_);
				DCHECK_EQ(owning_thread_id_, PlatformThread::CurrentId());
				owned_by_thread_ = false;
				owning_thread_id_ = base::kInvalidThreadId;
			}
			void CheckUnheldAndMark()
			{
				DCHECK(!owned_by_thread_);
				owned_by_thread_ = true;
				owning_thread_id_ = PlatformThread::CurrentId();
			}

			// 所有私有成员受lock_保护, 因此需要注意只能在加锁后使用.

			// owned_by_thread_用来确定owning_thread_id_是否合法.
			// 因为owning_thread_id_没有空值(null).
			bool owned_by_thread_;
			base::PlatformThreadId owning_thread_id_;
#endif //NDEBUG
		};

	}
	typedef internal::LockService<internal::Win32Lock> Lock;
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 自动加锁辅助类, 在作用域内自动加锁.
	class AutoLock
	{
	public:
		explicit AutoLock(Lock& lock) : lock_(lock)
		{
			lock_.Acquire();
		}

		~AutoLock()
		{
			lock_.AssertAcquired();
			lock_.Release();
		}

	private:
		Lock& lock_;
		DISALLOW_COPY_AND_ASSIGN(AutoLock);
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// 自动解锁辅助类, 构造函数中断言已加锁并解锁, 析构时重新加锁.
	class AutoUnlock
	{
	public:
		explicit AutoUnlock(Lock& lock) : lock_(lock)
		{
			// 断言调用者已加锁.
			lock_.AssertAcquired();
			lock_.Release();
		}

		~AutoUnlock()
		{
			lock_.Acquire();
		}

	private:
		Lock& lock_;
		DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
	};

}