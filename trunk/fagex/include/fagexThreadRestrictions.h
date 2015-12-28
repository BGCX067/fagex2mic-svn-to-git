
#pragma once

namespace base
{

	// 特定的线程中某些行为可能会被禁止. ThreadRestrictions辅助实现这些规则.
	// 这种规则的例子:
	//
	// *不要堵塞IO(导致线程僵死)
	// *不要访问Singleton/LazyInstance(可能导致崩溃)
	//
	// 下面是如何实现保护工作的:
	//
	// 1) 如果一个线程不允许IO调用, 这样做:
	//      base::ThreadRestrictions::SetIOAllowed(false);
	//    缺省情况, 线程是允许调用IO的.
	//
	// 2) 函数调用访问磁盘时, 需要检查当前线程是否允许:
	//      base::ThreadRestrictions::AssertIOAllowed();
	//
	// ThreadRestrictions在release编译时什么都不做; 只在debug下有效.
	//
	// 风格提示: 你应该在什么地方检查AssertIOAllowed? 最好是即将访问磁盘
	// 的时候, 在底层越靠近越好. 这个准则有助于捕获所有的调用. 比如, 如果
	// 你的函数GoDoSomeBlockingDiskCall()只调用其它函数而不是fopen(), 你
	// 应该在辅助函数中添加AssertIOAllowed检查.

	class ThreadRestrictions 
	{
	public:
		enum EnumRestrictionsAllowed
		{
			IO,
			SINGLETON,
		};

		static bool AssertRestrictionsAllowed(size_t type) 
		{
			return true;
		}
	};
}
