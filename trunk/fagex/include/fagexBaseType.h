

#pragma once

// DISALLOW_COPY_AND_ASSIGN禁用拷贝和赋值构造函数.
// 需要在类的private:访问控制域中使用.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&); \
	void operator=(const TypeName&)

// DISALLOW_IMPLICIT_CONSTRUCTORS禁止隐式的构造函数, 包括缺省构造函数、
// 拷贝构造函数和赋值构造函数.
//
// 需要在类的private:访问控制域中使用以防止实例化, 对于只有静态方法的
// 类非常有用.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName(); \
	DISALLOW_COPY_AND_ASSIGN(TypeName)

// COMPILE_ASSERT宏用来在编译时断言表达式. 例如可以这样保证静态数组大小:
//     COMPILE_ASSERT(ARRAYSIZE_UNSAFE(content_type_names)==CONTENT_NUM_TYPES,
//         content_type_names_incorrect_size);
//
// 或者确保结构体小于一定大小:
//     COMPILE_ASSERT(sizeof(foo)<128, foo_too_large);
// 第二个宏参数是变量名, 如果表达式为false, 编译器会产生一条包含变量名的错误/警告.

// COMPILE_ASSERT实现细节:
//
// - COMPILE_ASSERT通过定义一个长度为-1的数组(非法)来实现的, 此时表达式false.
//
// - 下面简化的定义
//       #define COMPILE_ASSERT(expr, msg) typedef char msg[(expr)?1:-1]
//   是非法的. 由于gcc支持运行时确定长度的变长数组(gcc扩展, 不属于C++标准),
//   导致下面这段简单的代码定义不报错:
//       int foo;
//       COMPILE_ASSERT(foo, msg); // not supposed to compile as foo is
//                                 // not a compile-time constant.
//
// - 要使用类型CompileAssert<(bool(expr))>, 必须确保expr是编译时常量.
//   (模板参数在编译时确定.)
//
// - CompileAssert<(bool(expr))>最外层的圆括号用于解决gcc 3.4.4和4.0.1的
//   一个bug. 如果写成
//       CompileAssert<bool(expr)>
//   编译器将无法编译
//       COMPILE_ASSERT(5>0, some_message);
//   ("5>0"中的">"被误认为是模板参数列表结尾的">".)
//
// - 数组大小是(bool(expr)?1:-1)而不是((expr)?1:-1), 可以解决MS VC 7.1
//   中把((0.0)?1:-1)错误计算为1的bug.

template<bool>
struct CompileAssert {};

#ifdef COMPILE_ASSERT
#	undef COMPILE_ASSERT
#endif //COMPILE_ASSERT
#define COMPILE_ASSERT(expr, __any) \
	typedef CompileAssert<(bool(expr))> __any[bool(expr) ? 1 : -1]
