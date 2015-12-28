

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
			// �����ǰû����, ����������true, �����������ش���.
			virtual bool Try() = 0;
			
			// ����, ��������ֱ�������ɹ�.
			virtual void Lock() = 0;
			
			// ����. ֻ���������ߵ���: Try()�ɹ����ػ���Lock()�ɹ���.
			virtual void Unlock() = 0;
		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		class Win32Lock : public LockDelegate
		{
		public:
			Win32Lock()
			{
				// �ڶ�����������������, ���ڶ�����������Ա����߳��л��������.
				// MSDN˵: �ڶ��������ڶദ���������, һ���߳��������Ѽ�����
				//         critical sectionʱ�����ѭ��, ����ʱ��Ż����sleep.
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

			// ����, ��������ֱ�������ɹ�.
			void Lock()
			{
				EnterCriticalSection(&_critical_section);
			}

			// ����. ֻ���������ߵ���: Try()�ɹ����ػ���Lock()�ɹ���.
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

			// ���û������, ����������true. ����Ѿ��������߳�����, ��������false.
			// �Ѿ��������̲߳�Ҫ�ٵ���(���ܲ���Ԥ�ڲ����Ķ���ʧ��).
			bool Try() { return _impl.Try(); }

			// ��debugģʽ���ǿ�ʵ��.
			void AssertAcquired() const {}
#else //!NDEBUG
			LockService(): _impl()
			{
				owned_by_thread_ = false;
				owning_thread_id_ = base::kInvalidThreadId;
			}
			~LockService() {}

			// ע��: ����windows���ٽ���֧�ֵݹ����, �������ﲻ����, ����̵߳ڶ���
			// �������(�Ѿ�����)�ᴥ��DCHECK().
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
			// ����2������Ϊ�ݹ����ʱ�����������.
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

			// ����˽�г�Ա��lock_����, �����Ҫע��ֻ���ڼ�����ʹ��.

			// owned_by_thread_����ȷ��owning_thread_id_�Ƿ�Ϸ�.
			// ��Ϊowning_thread_id_û�п�ֵ(null).
			bool owned_by_thread_;
			base::PlatformThreadId owning_thread_id_;
#endif //NDEBUG
		};

	}
	typedef internal::LockService<internal::Win32Lock> Lock;
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// �Զ�����������, �����������Զ�����.
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
	// �Զ�����������, ���캯���ж����Ѽ���������, ����ʱ���¼���.
	class AutoUnlock
	{
	public:
		explicit AutoUnlock(Lock& lock) : lock_(lock)
		{
			// ���Ե������Ѽ���.
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