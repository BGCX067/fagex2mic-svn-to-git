

#pragma once

// DISALLOW_COPY_AND_ASSIGN���ÿ����͸�ֵ���캯��.
// ��Ҫ�����private:���ʿ�������ʹ��.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&); \
	void operator=(const TypeName&)

// DISALLOW_IMPLICIT_CONSTRUCTORS��ֹ��ʽ�Ĺ��캯��, ����ȱʡ���캯����
// �������캯���͸�ֵ���캯��.
//
// ��Ҫ�����private:���ʿ�������ʹ���Է�ֹʵ����, ����ֻ�о�̬������
// ��ǳ�����.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName(); \
	DISALLOW_COPY_AND_ASSIGN(TypeName)

// COMPILE_ASSERT�������ڱ���ʱ���Ա��ʽ. �������������֤��̬�����С:
//     COMPILE_ASSERT(ARRAYSIZE_UNSAFE(content_type_names)==CONTENT_NUM_TYPES,
//         content_type_names_incorrect_size);
//
// ����ȷ���ṹ��С��һ����С:
//     COMPILE_ASSERT(sizeof(foo)<128, foo_too_large);
// �ڶ���������Ǳ�����, ������ʽΪfalse, �����������һ�������������Ĵ���/����.

// COMPILE_ASSERTʵ��ϸ��:
//
// - COMPILE_ASSERTͨ������һ������Ϊ-1������(�Ƿ�)��ʵ�ֵ�, ��ʱ���ʽfalse.
//
// - ����򻯵Ķ���
//       #define COMPILE_ASSERT(expr, msg) typedef char msg[(expr)?1:-1]
//   �ǷǷ���. ����gcc֧������ʱȷ�����ȵı䳤����(gcc��չ, ������C++��׼),
//   ����������μ򵥵Ĵ��붨�岻����:
//       int foo;
//       COMPILE_ASSERT(foo, msg); // not supposed to compile as foo is
//                                 // not a compile-time constant.
//
// - Ҫʹ������CompileAssert<(bool(expr))>, ����ȷ��expr�Ǳ���ʱ����.
//   (ģ������ڱ���ʱȷ��.)
//
// - CompileAssert<(bool(expr))>������Բ�������ڽ��gcc 3.4.4��4.0.1��
//   һ��bug. ���д��
//       CompileAssert<bool(expr)>
//   ���������޷�����
//       COMPILE_ASSERT(5>0, some_message);
//   ("5>0"�е�">"������Ϊ��ģ������б��β��">".)
//
// - �����С��(bool(expr)?1:-1)������((expr)?1:-1), ���Խ��MS VC 7.1
//   �а�((0.0)?1:-1)�������Ϊ1��bug.

template<bool>
struct CompileAssert {};

#ifdef COMPILE_ASSERT
#	undef COMPILE_ASSERT
#endif //COMPILE_ASSERT
#define COMPILE_ASSERT(expr, __any) \
	typedef CompileAssert<(bool(expr))> __any[bool(expr) ? 1 : -1]
