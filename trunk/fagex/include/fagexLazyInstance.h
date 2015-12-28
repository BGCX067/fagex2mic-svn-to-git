
/************************************************************************/
/*                                                                      
	LazyInstance<Type, Traits>��(������������ָ�ȽϷ���Ĵ���)
	����Type�ĵ�һʵ��, �����ڵ�һ�η���ʱ������.
	һ��������Ҫ��������ľ�̬����ʱ, �����ǳ�����, ����Ҫ��֤�̰߳�ȫ��.
	Type�Ĺ��캯����ʹ���̳߳�ͻ�������Ҳֻ�ᱻ����һ��. Get()��Pointer()��
	�Ƿ�����ͬ����ȫ��ʼ����ʵ������. ���������ڳ����˳�ʱ����.

	������󱻰�ȫ�Ĵ���, LazyInstanceҲ���̰߳�ȫ��. �����POD��ʼ��, ���Բ�
	��Ҫ��̬�Ĺ��캯��. ͨ��base::LinkerInitialized����һ��ȫ�ֵ�LazyInstance
	�����ǱȽϺõ�����.

	LazyInstance����Singleton(����), ��û�е���������. 
	��ͬ�����Ϳ����ж��LazyInstance,
	ÿ��LazyInstanceά��һ��Ψһ��ʵ��. ����ΪTypeԤ����ռ�, �����ڶ��ϴ���
	����. �����������ʵ������������, ���ٶ���Ƭ. ��ҪType�������������Ա�ȷ
	����С.

	�÷�ʾ��:
	static LazyInstance<MyClass> my_instance(base::LINKER_INITIALIZED);
	void SomeMethod() {
	   my_instance.Get().SomeMethod(); // MyClass::SomeMethod()

	   MyClass* ptr = my_instance.Pointer();
	   ptr->DoDoDo(); // MyClass::DoDoDo
	}

	������Ҫ��ԭ�Ӳ���Ϊ����, �����̼߳���л����ڵȴ������̴߳���ʱ����Ҫ��CPU��
	����Ȩ�ƽ����£�������Ҫsleep(0)
	���������£�ʵ�����ͷŶ���Ҫ��˳����������ͷŻ��Ǻ���Ҫ�ġ����������
	AtExitManager
*/
/************************************************************************/
#pragma once
#include "fagexAtomicops.h"

namespace fagex
{
	//������ʱȱʡʹ�õĳ�ʼ���뷴��ʼ��
	template<typename Type>
	struct DefaultLazyInstanceTraits
	{
		static const bool kAllowedToAccessOnNonjoinableThread = false;

		static Type* New(void* instance)
		{
			// ʹ��placement new��Ԥ����Ŀռ��ϳ�ʼ��ʵ��.
			// Բ���ź���Ҫ, ǿ��POD���ͳ�ʼ��.
			return new (instance) Type();
		}
		static void Delete(void* instance)
		{
			// ��ʽ������������.
			reinterpret_cast<Type*>(instance)->~Type();
		}
	};

	//-----------------------------------------------------------------------------------
	// ��ȡ���ַ�ģ�庯��, �������԰Ѹ��ӵĴ���Ƭ�η���.cpp�ļ�.
	class LazyInstanceHelper
	{
	protected:
		enum
		{
			STATE_EMPTY    = 0,
			STATE_CREATING = 1,
			STATE_CREATED  = 2
		};
		LazyInstanceHelper() { /* state_Ϊ0 */ }

		// ����Ƿ���Ҫ����ʵ��. ����true��ʾ��Ҫ����, ����false��ʾ�����߳�
		// ���ڴ���, �ȴ�ʵ��������ɲ�����false.
		bool NeedsInstance();

		// ����ʵ��֮��, ע������˳��ǵ��õ���������, ������stateΪ
		// STATE_CREATED.
		void CompleteInstance(void* instance, void (*dtor)(void*));

		subtle::AtomicWord state_;

	private:
		DISALLOW_COPY_AND_ASSIGN(LazyInstanceHelper);
	};

	//-----------------------------------------------------------------------------------
	template<typename Type, typename Traits = DefaultLazyInstanceTraits<Type>>
	class LazyInstance : public LazyInstanceHelper
	{
	public:
		LazyInstance() : LazyInstanceHelper() {}

		Type& Get()
		{
			return *Pointer();
		}

		Type* Pointer()
		{
			if(!Traits::kAllowedToAccessOnNonjoinableThread)
			{
				ThreadRestrictions::AssertRestrictionsAllowed(ThreadRestrictions::SINGLETON);
			}

			// ��������Ѿ�������, ϣ���ܿ��ٷ���.
			if((subtle::NoBarrier_Load(&state_) != STATE_CREATED) && NeedsInstance())
			{
				// ��|buf_|���ڵĿռ��ϴ���ʵ��.
				instance_ = Traits::New(buf_);
				
				// LeakyLazyInstanceTraits��Traits::DeleteΪ��.
				void (*dtor)(void*) = Traits::Delete;
				CompleteInstance(this, (dtor == NULL) ? NULL : OnExit);
			}

			return instance_;
		}

		bool operator == (Type* p)
		{
			switch(subtle::NoBarrier_Load(&state_))
			{
			case STATE_EMPTY:
				return p == NULL;

			case STATE_CREATING:
				return static_cast<int8*>(static_cast<void*>(p)) == buf_;

			case STATE_CREATED:
				return p == instance_;

			default:
				return false;
			}
		}

	private:
		// ���AtExit�����亯��. Ӧ���ڵ��߳��е���, ���Բ���Ҫʹ��ԭ�Ӳ���.
		// �������߳�ʹ��ʵ��ʱ����OnExit�Ǵ����.
		static void OnExit(void* lazy_instance)
		{
			LazyInstance<Type, Traits>* me =
				reinterpret_cast<LazyInstance<Type, Traits>*>(lazy_instance);
			Traits::Delete(me->instance_);
			me->instance_ = NULL;
			subtle::Release_Store(&me->state_, STATE_EMPTY);
		}

		int8 buf_[sizeof(Type)]; // Typeʵ����Ԥ����ռ�.
		Type* instance_;

		DISALLOW_COPY_AND_ASSIGN(LazyInstance);
	};

}
