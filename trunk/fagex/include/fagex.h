
#pragma once
#include <boost/asio.hpp>

namespace fagex
{
	namespace ui
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// ������ʵ�ֶ�ƽ̨��ش��ڻ��ǿؼ��ķ�װ
		class NativeWidget
		{

		};

		// �����Ǵ������ʵ��
		class NativeWidgetWin //Window(HWND)
		{
		}; 

		/////////////////////////////////////////////////////////////////////////////////////
		// ��������ڴ��ڡ��ؼ��ĳ���
		class WidgetDelegate
		{

		};

		// ����һ���Ǵ�����Ļ���
		class Widget 
		{

		};

		/////////////////////////////////////////////////////////////////////////////////////
		// �������Ƕ���ͼ�����ķ�װ���޹ش���
		class View
		{

		};

		//����һ��������ͼ�Ļ���
		class WidgetDelegateView : public WidgetDelegate, public View
		{

		};

		/////////////////////////////////////////////////////////////////////////////////////
		// ��������Ϣѭ�����
		class MessageLoop
		{

		};

	}
}