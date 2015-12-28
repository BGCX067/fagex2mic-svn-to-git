
#pragma once
#include <boost/asio.hpp>

namespace fagex
{
	namespace ui
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// 以下类实现对平台相关窗口或是控件的封装
		class NativeWidget
		{

		};

		// 此类是窗口类的实现
		class NativeWidgetWin //Window(HWND)
		{
		}; 

		/////////////////////////////////////////////////////////////////////////////////////
		// 以下类对于窗口、控件的抽象
		class WidgetDelegate
		{

		};

		// 此类一般是窗口类的基类
		class Widget 
		{

		};

		/////////////////////////////////////////////////////////////////////////////////////
		// 以下类是对视图创作的封装，无关窗口
		class View
		{

		};

		//此类一般是主视图的基类
		class WidgetDelegateView : public WidgetDelegate, public View
		{

		};

		/////////////////////////////////////////////////////////////////////////////////////
		// 以下是消息循环相关
		class MessageLoop
		{

		};

	}
}