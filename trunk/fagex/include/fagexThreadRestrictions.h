
#pragma once

namespace base
{

	// �ض����߳���ĳЩ��Ϊ���ܻᱻ��ֹ. ThreadRestrictions����ʵ����Щ����.
	// ���ֹ��������:
	//
	// *��Ҫ����IO(�����߳̽���)
	// *��Ҫ����Singleton/LazyInstance(���ܵ��±���)
	//
	// ���������ʵ�ֱ���������:
	//
	// 1) ���һ���̲߳�����IO����, ������:
	//      base::ThreadRestrictions::SetIOAllowed(false);
	//    ȱʡ���, �߳����������IO��.
	//
	// 2) �������÷��ʴ���ʱ, ��Ҫ��鵱ǰ�߳��Ƿ�����:
	//      base::ThreadRestrictions::AssertIOAllowed();
	//
	// ThreadRestrictions��release����ʱʲô������; ֻ��debug����Ч.
	//
	// �����ʾ: ��Ӧ����ʲô�ط����AssertIOAllowed? ����Ǽ������ʴ���
	// ��ʱ��, �ڵײ�Խ����Խ��. ���׼�������ڲ������еĵ���. ����, ���
	// ��ĺ���GoDoSomeBlockingDiskCall()ֻ������������������fopen(), ��
	// Ӧ���ڸ������������AssertIOAllowed���.

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
