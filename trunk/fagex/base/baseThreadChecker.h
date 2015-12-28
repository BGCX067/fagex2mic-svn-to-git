
#ifndef __base_thread_checker_h__
#define __base_thread_checker_h__

#pragma once
#include "base/baseLock.h"
#include "base/basePlatformThread.h"

namespace base
{

	// Real implementation of ThreadChecker, for use in debug mode, or
	// for temporary use in release mode (e.g. to CHECK on a threading issue
	// seen only in the wild).
	//
	// Note: You should almost always use the ThreadChecker class to get the
	// right version for your build configuration.
	class ThreadCheckerImpl
	{
	public:
		ThreadCheckerImpl();
		~ThreadCheckerImpl();

		bool CalledOnValidThread() const;

		// Changes the thread that is checked for in CalledOnValidThread.  This may
		// be useful when an object may be created on one thread and then used
		// exclusively on another thread.
		void DetachFromThread();

	private:
		void EnsureThreadIdAssigned() const;

		mutable base::Lock lock_;
		// This is mutable so that CalledOnValidThread can set it.
		// It's guarded by |lock_|.
		mutable PlatformThreadId valid_thread_id_;
	};

	// Do nothing implementation, for use in release mode.
	//
	// Note: You should almost always use the ThreadChecker class to get the
	// right version for your build configuration.
	class ThreadCheckerDoNothing
	{
	public:
		bool CalledOnValidThread() const
		{
			return true;
		}

		void DetachFromThread() {}
	};

	// Before using this class, please consider using NonThreadSafe as it
	// makes it much easier to determine the nature of your class.
	//
	// ThreadChecker is a helper class used to help verify that some methods of a
	// class are called from the same thread.  One can inherit from this class and
	// use CalledOnValidThread() to verify.
	//
	// Inheriting from class indicates that one must be careful when using the
	// class with multiple threads. However, it is up to the class document to
	// indicate how it can be used with threads.
	//
	// Example:
	// class MyClass : public ThreadChecker {
	//  public:
	//   void Foo() {
	//     DCHECK(CalledOnValidThread());
	//     ... (do stuff) ...
	//   }
	// }
	//
	// In Release mode, CalledOnValidThread will always return true.
#ifndef NDEBUG
	class ThreadChecker : public ThreadCheckerImpl {};
#else
	class ThreadChecker : public ThreadCheckerDoNothing {};
#endif //NDEBUG

} //namespace base

#endif  //__base_thread_checker_h__